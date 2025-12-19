#include <stdio.h>
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "app_ipcam_ai.h"
#include "tdl_sdk.h"
#include <pthread.h>
#include <stdio.h>
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FEATURE_SIZE 256
#define MAX_DET_NUM 100

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
static TDLObject g_obj_meta = {0};
static uint64_t g_frame_id = 0;
static uint32_t g_lost_start_time;          // Start time when the object is lost
static bool g_lost_timer_started = false;   // Whether the lost timer has started
static APP_PARAM_AI_OBJECT_TRACK_CFG_S g_stObjTrackCfg;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S *g_pstObjTrackCfg = &g_stObjTrackCfg;
static volatile bool g_bObjectTrackRunning = CVI_FALSE;
static volatile bool g_bObjectTrackPause = CVI_FALSE;
static pthread_t g_ObjectTrackHandle;
static pthread_t g_ObjectDetHandle;
static TDLHandle g_ObjectTrackTDLHandle;
static TDLObject g_stObjDraw = {0};
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ModeMutex = PTHREAD_MUTEX_INITIALIZER;
const int LOST_TIMEOUT_SECONDS = 5;  // Timeout for object lost

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static uint32_t get_time_in_ms() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    return 0;
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

APP_PARAM_AI_OBJECT_TRACK_CFG_S *app_ipcam_Ai_Object_Track_Param_Get(void)
{
    return g_pstObjTrackCfg;
}

APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Mode_Get(void) {
    pthread_mutex_lock(&g_ModeMutex);
    APP_PARAM_OBJECT_TRACK_MODE mode = g_mode;
    pthread_mutex_unlock(&g_ModeMutex);
    return mode;
}

CVI_VOID app_ipcam_Ai_Object_Track_Mode_Set(APP_PARAM_OBJECT_TRACK_MODE mode) {
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
    pthread_mutex_lock(&g_StatusMutex);
    CVI_BOOL Pause = g_bObjectTrackPause;
    pthread_mutex_unlock(&g_StatusMutex);
    return Pause;
}

CVI_VOID app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    if (pstAiObj == NULL) return;
    if (pstAiObj->info == NULL) {
        pstAiObj->info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
    }
    if (pstAiObj == NULL ||
        g_stObjDraw.size == 0 ||
        g_stObjDraw.info == NULL ||
        pstAiObj->info == NULL) {
        pstAiObj->size = 0;
        return;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        pstAiObj->size = g_stObjDraw.size <= MAX_DET_NUM ? g_stObjDraw.size : MAX_DET_NUM;
        memcpy(pstAiObj->info, g_stObjDraw.info, pstAiObj->size * sizeof(TDLObjectInfo));
    }

}

static CVI_VOID *Thread_Object_Det_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    VPSS_GRP VpssGrp = g_pstObjTrackCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstObjTrackCfg->VpssChn;

    TDLObject obj_meta = {0};
    VIDEO_FRAME_INFO_S stObjectDetFrame = {0};

    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        if (app_ipcam_Ai_Object_Track_Pause_Get() ||
            app_ipcam_Ai_Object_Track_Mode_Get() != DETECTION) {
            usleep(1000*1000);
            continue;
        }

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stObjectDetFrame, 3000);
        if (s32Ret != 0){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to CVI_VPSS_GetChnFrame with %x\n", s32Ret);
            continue;
        }

        g_frame_id++;

        TDLImage image = TDL_WrapFrame(&stObjectDetFrame, true);
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame \n");
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectDetFrame);
            continue;
        }

        s32Ret = TDL_Detection(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det, image, &obj_meta);
        if (s32Ret != 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x!\n", s32Ret);
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectDetFrame);
            TDL_DestroyImage(image);
            continue;
        }
        TDL_CopyObjectMeta(&obj_meta, &g_obj_meta);

        {
            SMT_MutexAutoLock(g_Mutex, lock);
            g_stObjDraw.size = 0;
            if (obj_meta.size > 0 &&
                obj_meta.info != NULL &&
                g_stObjDraw.info != NULL) {
                g_stObjDraw.size = obj_meta.size <= MAX_DET_NUM ? obj_meta.size : MAX_DET_NUM;
                // printf("enter obj_meta.size = %d\n", obj_meta.size);
                memcpy(g_stObjDraw.info, obj_meta.info, g_stObjDraw.size * sizeof(TDLObjectInfo));
            }
        }

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectDetFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_ReleaseObjectMeta(&obj_meta);
        TDL_DestroyImage(image);
    }


    pthread_exit(NULL);

    return NULL;
}

static CVI_VOID *Thread_Object_Track_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    VPSS_GRP VpssGrp = g_pstObjTrackCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstObjTrackCfg->VpssChn;

    TDLTracker track_meta = {0};
    VIDEO_FRAME_INFO_S stObjectTrackFrame = {0};
    bool track_init = CVI_FALSE;
    int32_t box[4] = {0};
    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        if (app_ipcam_Ai_Object_Track_Pause_Get() || access("/tmp/track", F_OK) != 0) {
            usleep(1000*1000);
            continue;
        }
        app_ipcam_Ai_Object_Track_Mode_Set(TRACKING);

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stObjectTrackFrame, 3000);
        if (s32Ret != 0){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to CVI_VPSS_GetChnFrame with %x\n", s32Ret);
            continue;
        }

        g_frame_id++;

        TDLImage image = TDL_WrapFrame(&stObjectTrackFrame, true);
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame \n");
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectTrackFrame);
            continue;
        }

        if (!track_init) {
            char buf[5] = {0};
            FILE *pFile= fopen("/tmp/track", "r");
            fread(buf, 1, sizeof(buf), pFile);
            fclose(pFile);
            box[0] = atoi(buf);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "track id : %d \n", box[0]);
            s32Ret = TDL_SetSingleObjectTracking(g_ObjectTrackTDLHandle,
                                                 image,
                                                 &g_obj_meta,
                                                 box,
                                                 1,
                                                 g_pstObjTrackCfg->search_type);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTracking failed with %#x!\n", s32Ret);
                remove("/tmp/track");
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
            } else {
                track_init = CVI_TRUE;
            }
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectTrackFrame);
            TDL_DestroyImage(image);
        } else {
            s32Ret = TDL_SingleObjectTracking(g_ObjectTrackTDLHandle, image, &track_meta, g_frame_id);
            if (s32Ret != 0) {
               APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SingleObjectTracking failed with %#x!\n", s32Ret);
               CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectTrackFrame);
               TDL_DestroyImage(image);
               app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
               track_init = CVI_FALSE;
               remove("/tmp/track");
               continue;
            }

            if (track_meta.info) {
                printf("enter track_meta.x1 == %f\n", track_meta.info[0].bbox.x1);
                SMT_MutexAutoLock(g_Mutex, lock);
                g_stObjDraw.size = 1;
                g_stObjDraw.info[0].box.x1 = track_meta.info[0].bbox.x1;
                g_stObjDraw.info[0].box.x2 = track_meta.info[0].bbox.x2;
                g_stObjDraw.info[0].box.y1 = track_meta.info[0].bbox.y1;
                g_stObjDraw.info[0].box.y2 = track_meta.info[0].bbox.y2;
            } else {
                // Start or continue lost timing
                if (!g_lost_timer_started) {
                    g_lost_start_time = get_time_in_ms();
                    g_lost_timer_started = true;
                } else {
                    uint32_t current_time = get_time_in_ms();
                    uint32_t elapsed_time = current_time - g_lost_start_time;
                    printf("enter elapsed_time = %d\n", elapsed_time);
                    if (elapsed_time >= (uint32_t)(LOST_TIMEOUT_SECONDS * 1000)) {
                        APP_PROF_LOG_PRINT(LEVEL_INFO,
                                           "The target has been lost for more than [%d] seconds, "
                                           "switching to detection state\n",
                                           LOST_TIMEOUT_SECONDS);
                        app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                        g_lost_timer_started = false;
                        track_init = CVI_FALSE;
                        remove("/tmp/track");
                    }
                }
            }
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stObjectTrackFrame);
            TDL_DestroyImage(image);
            TDL_ReleaseTrackMeta(&track_meta);
        }
    }

    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_ObjectTrackTDLHandle == NULL)
    {
        g_ObjectTrackTDLHandle = TDL_CreateHandle(0);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDLHandle has created\n");
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det, g_pstObjTrackCfg->model_path_det, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel DET failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot, g_pstObjTrackCfg->model_path_sot, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel SOT failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    // s32Ret = TDL_SetSotModelThreshold(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->threshold_occluded, g_pstObjTrackCfg->threshold_reappear);
    // if (s32Ret != CVI_SUCCESS)
    // {
    //     APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSotModelThreshold failed with %#x!\n", s32Ret);
    //     return s32Ret;
    // }

    if (g_stObjDraw.info == NULL) {
        g_stObjDraw.info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> done \n");

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Object_Track_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
    pthread_join(g_ObjectTrackHandle, NULL);
    g_ObjectTrackHandle = 0;
    pthread_join(g_ObjectDetHandle, NULL);
    g_ObjectDetHandle = 0;

    if (g_stObjDraw.info != NULL) {
        free(g_stObjDraw.info);
        g_stObjDraw.info = NULL;
    }

    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det);
    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot);

    s32Ret = TDL_DestroyHandle(g_ObjectTrackTDLHandle);

    if(s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "enter TDL_DestroyHandle fail \n");
        return s32Ret;
    }
    g_ObjectTrackTDLHandle = NULL;

    return s32Ret;
}

int app_ipcam_Ai_Object_Track_Start(void){
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bObjectTrackRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Object_Track_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Object_Track_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_ObjectDetHandle, NULL, Thread_Object_Det_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack_pthread_create failed!\n");
        return s32Ret;
    }

    s32Ret = pthread_create(&g_ObjectTrackHandle, NULL, Thread_Object_Track_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack_pthread_create failed!\n");
        return s32Ret;
    }

    return s32Ret;
}
