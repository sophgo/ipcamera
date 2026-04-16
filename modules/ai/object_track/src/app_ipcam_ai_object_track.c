#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "app_ipcam_ai.h"
#include "tdl_sdk.h"
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define MAX_DET_NUM 100
#define TRACK_REQUEST_PATH "/tmp/track"
#define SELECTION_BOX_SIZE 200

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OBJECT_TRACK_MODE g_mode = DETECTION;
static uint64_t g_frame_id = 0;
static uint32_t g_lost_start_time = 0;
static bool g_lost_timer_started = false;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S g_stObjTrackCfg;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S *g_pstObjTrackCfg = &g_stObjTrackCfg;
static volatile bool g_bObjectTrackRunning = CVI_FALSE;
static volatile bool g_bObjectTrackPause = CVI_FALSE;
static pthread_t g_ObjectTrackHandle;
static TDLHandle g_ObjectTrackTDLHandle;
static TDLObject g_stObjDraw = {0};
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ModeMutex = PTHREAD_MUTEX_INITIALIZER;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static uint32_t get_time_in_ms(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0) {
        return 0;
    }

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

CVI_VOID app_ipcam_Ai_Object_Track_DefaultBox_Get(int32_t box[4])
{
    CVI_U32 grp_width = g_pstObjTrackCfg->u32GrpWidth;
    CVI_U32 grp_height = g_pstObjTrackCfg->u32GrpHeight;
    CVI_U32 box_size = SELECTION_BOX_SIZE;
    CVI_U32 center_x = 0;
    CVI_U32 center_y = 0;

    if (box == NULL) {
        return;
    }

    if (grp_width == 0 || grp_height == 0) {
        grp_width = 640;
        grp_height = 384;
    }

    if (box_size > grp_width) {
        box_size = grp_width;
    }
    if (box_size > grp_height) {
        box_size = grp_height;
    }

    center_x = grp_width / 2;
    center_y = grp_height / 2;
    box[0] = (int32_t)(center_x - box_size / 2);
    box[1] = (int32_t)(center_y - box_size / 2);
    box[2] = (int32_t)(box[0] + box_size);
    box[3] = (int32_t)(box[1] + box_size);
}

APP_PARAM_AI_OBJECT_TRACK_CFG_S *app_ipcam_Ai_Object_Track_Param_Get(void)
{
    return g_pstObjTrackCfg;
}

APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Mode_Get(void)
{
    APP_PARAM_OBJECT_TRACK_MODE mode = DETECTION;

    pthread_mutex_lock(&g_ModeMutex);
    mode = g_mode;
    pthread_mutex_unlock(&g_ModeMutex);

    return mode;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Mode_Set(APP_PARAM_OBJECT_TRACK_MODE mode)
{
    pthread_mutex_lock(&g_ModeMutex);
    g_mode = mode;
    pthread_mutex_unlock(&g_ModeMutex);
}

CVI_VOID app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_BOOL flag)
{
    g_bObjectTrackRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Object_Track_ProcStatus_Get(void)
{
    return g_bObjectTrackRunning;
}

CVI_VOID app_ipcam_Ai_Object_Track_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_StatusMutex);
    g_bObjectTrackPause = flag;
    pthread_mutex_unlock(&g_StatusMutex);
}

CVI_BOOL app_ipcam_Ai_Object_Track_Pause_Get(void)
{
    CVI_BOOL pause = CVI_FALSE;

    pthread_mutex_lock(&g_StatusMutex);
    pause = g_bObjectTrackPause;
    pthread_mutex_unlock(&g_StatusMutex);

    return pause;
}

CVI_VOID app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    if (pstAiObj == NULL) {
        return;
    }

    if (pstAiObj->info == NULL) {
        pstAiObj->info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
    }
    if (pstAiObj->info == NULL) {
        pstAiObj->size = 0;
        return;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);

        if (g_stObjDraw.size == 0 || g_stObjDraw.info == NULL) {
            pstAiObj->size = 0;
            return;
        }

        pstAiObj->size = g_stObjDraw.size <= MAX_DET_NUM ? g_stObjDraw.size : MAX_DET_NUM;
        memcpy(pstAiObj->info, g_stObjDraw.info, pstAiObj->size * sizeof(TDLObjectInfo));
    }
}

static CVI_VOID *Thread_Object_Track_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_GRP VpssGrp = g_pstObjTrackCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstObjTrackCfg->VpssChn;
    TDLObject cur_det_meta = {0};
    TDLTracker track_meta = {0};
    bool track_init = false;

    (void)pArgs;
    prctl(PR_SET_NAME, "AI_OBJ_TRACK", 0, 0, 0);

    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        VIDEO_FRAME_INFO_S stFrame = {0};
        TDLImage image = NULL;
        bool frame_acquired = false;
        bool has_track_request = (access(TRACK_REQUEST_PATH, F_OK) == 0);

        if (app_ipcam_Ai_Object_Track_Pause_Get()) {
            usleep(1000 * 1000);
            continue;
        }

        if (!has_track_request) {
            app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
            track_init = false;
            g_lost_timer_started = false;
        } else if (app_ipcam_Ai_Object_Track_Mode_Get() == DETECTION) {
            app_ipcam_Ai_Object_Track_Mode_Set(TRACKING);
        }

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to CVI_VPSS_GetChnFrame with %x\n", s32Ret);
            continue;
        }
        frame_acquired = true;
        g_frame_id++;

        image = TDL_WrapFrame(&stFrame, false, false);
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame\n");
            goto loop_cleanup;
        }

        if (app_ipcam_Ai_Object_Track_Mode_Get() == DETECTION) {
            s32Ret = TDL_Detection(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det, image, &cur_det_meta);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x!\n", s32Ret);
                goto loop_cleanup;
            }

            {
                SMT_MutexAutoLock(g_Mutex, lock);

                g_stObjDraw.size = 0;
                if (cur_det_meta.size > 0 && cur_det_meta.info != NULL && g_stObjDraw.info != NULL) {
                    g_stObjDraw.size = cur_det_meta.size <= MAX_DET_NUM ? cur_det_meta.size : MAX_DET_NUM;
                    memcpy(g_stObjDraw.info, cur_det_meta.info, g_stObjDraw.size * sizeof(TDLObjectInfo));
                }
            }
            goto loop_cleanup;
        }

        if (!track_init) {
            char buf[8] = {0};
            int32_t box[4] = {0};
            TDLObject empty_det_meta = {0};
            TDLTargetSearchTypeE search_type = g_pstObjTrackCfg->search_type;
            FILE *pFile = fopen(TRACK_REQUEST_PATH, "r");
            size_t read_size = 0;

            if (pFile == NULL) {
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                goto loop_cleanup;
            }

            read_size = fread(buf, 1, sizeof(buf) - 1, pFile);
            fclose(pFile);
            if (read_size == 0) {
                remove(TRACK_REQUEST_PATH);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                goto loop_cleanup;
            }

            if (search_type < TDL_REJECT || search_type > TDL_COLOR) {
                search_type = TDL_COLOR;
            }

            app_ipcam_Ai_Object_Track_DefaultBox_Get(box);
            APP_PROF_LOG_PRINT(LEVEL_WARN,
                "track request[%s] init bbox: x1=%d y1=%d x2=%d y2=%d search_type=%d\n",
                buf, box[0], box[1], box[2], box[3], search_type);
            s32Ret = TDL_SetSingleObjectTracking(g_ObjectTrackTDLHandle, image, &empty_det_meta, box, 4, search_type);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTracking failed with %#x!\n", s32Ret);
                remove(TRACK_REQUEST_PATH);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                track_init = false;
            } else {
                track_init = true;
                g_lost_timer_started = false;
            }
        } else {
            s32Ret = TDL_SingleObjectTracking(g_ObjectTrackTDLHandle, image, &track_meta, g_frame_id);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SingleObjectTracking failed with %#x!\n", s32Ret);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                track_init = false;
                remove(TRACK_REQUEST_PATH);
                goto loop_cleanup;
            }

            if (track_meta.info != NULL && g_stObjDraw.info != NULL) {
                SMT_MutexAutoLock(g_Mutex, lock);

                g_stObjDraw.size = 1;
                g_stObjDraw.info[0].box.x1 = track_meta.info[0].bbox.x1;
                g_stObjDraw.info[0].box.x2 = track_meta.info[0].bbox.x2;
                g_stObjDraw.info[0].box.y1 = track_meta.info[0].bbox.y1;
                g_stObjDraw.info[0].box.y2 = track_meta.info[0].bbox.y2;
                g_lost_timer_started = false;
            } else if (!g_lost_timer_started) {
                g_lost_start_time = get_time_in_ms();
                g_lost_timer_started = true;
            } else {
                uint32_t current_time = get_time_in_ms();
                uint32_t elapsed_time = current_time - g_lost_start_time;

                if (elapsed_time >= g_pstObjTrackCfg->lost_timeout_seconds * 1000) {
                    APP_PROF_LOG_PRINT(LEVEL_WARN,
                        "The target has been lost for more than [%d] seconds, switching to detection state\n",
                        g_pstObjTrackCfg->lost_timeout_seconds);
                    app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                    g_lost_timer_started = false;
                    track_init = false;
                    remove(TRACK_REQUEST_PATH);
                }
            }
        }

loop_cleanup:
        if (track_meta.info != NULL) {
            TDL_ReleaseTrackMeta(&track_meta);
            memset(&track_meta, 0, sizeof(track_meta));
        }
        if (cur_det_meta.info != NULL) {
            TDL_ReleaseObjectMeta(&cur_det_meta);
            memset(&cur_det_meta, 0, sizeof(cur_det_meta));
        }
        if (image != NULL) {
            TDL_DestroyImage(image);
        }
        if (frame_acquired) {
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n",
                    VpssGrp, VpssChn, s32Ret);
            }
        }
    }
    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Proc_Init(CVI_VOID)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> start\n");

    if (g_ObjectTrackTDLHandle == NULL) {
        g_ObjectTrackTDLHandle = TDL_CreateHandle(0);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDLHandle has created\n");
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det,
        g_pstObjTrackCfg->model_path_det, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel DET failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot,
        g_pstObjTrackCfg->model_path_sot, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel SOT failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    if (g_stObjDraw.info == NULL) {
        g_stObjDraw.info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
        if (g_stObjDraw.info == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc g_stObjDraw.info failed\n");
            return CVI_FAILURE;
        }
    }

    g_stObjDraw.size = 0;
    g_frame_id = 0;
    g_lost_timer_started = false;
    app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> done\n");

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Object_Track_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
    if (g_ObjectTrackHandle) {
        pthread_join(g_ObjectTrackHandle, NULL);
        g_ObjectTrackHandle = 0;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);

        if (g_stObjDraw.info != NULL) {
            free(g_stObjDraw.info);
            g_stObjDraw.info = NULL;
        }
        g_stObjDraw.size = 0;
    }

    remove(TRACK_REQUEST_PATH);
    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det);
    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot);
    s32Ret = TDL_DestroyHandle(g_ObjectTrackTDLHandle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "TDL_DestroyHandle fail\n");
        return s32Ret;
    }
    g_ObjectTrackTDLHandle = NULL;

    return s32Ret;
}

int app_ipcam_Ai_Object_Track_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bObjectTrackRunning) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Object_Track_Proc_Init();
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Object_Track_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_TRUE);
    s32Ret = pthread_create(&g_ObjectTrackHandle, NULL, Thread_Object_Track_Proc, NULL);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack pthread_create failed!\n");
        app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
        app_ipcam_Ai_Object_Track_Stop();
        return s32Ret;
    }

    return s32Ret;
}
