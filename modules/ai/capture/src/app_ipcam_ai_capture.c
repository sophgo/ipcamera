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

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_CAPTURE_CFG_S g_stCapCfg;
static APP_PARAM_AI_CAPTURE_CFG_S *g_pstCapCfg = &g_stCapCfg;
static volatile bool g_bCaptureRunning = CVI_FALSE;
static volatile bool g_bCapturePause = CVI_FALSE;
static uint64_t *channel_frame_id = NULL;
static char **channel_names = NULL;
static uint8_t channel_size = 0;
static TDLFeatureInfo gallery_feature = {0};
static pthread_t g_CaptureThreadHandle;
static TDLHandle g_CaptureTDLHandle;
static TDLObject g_stObjDraw = {0};
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static const char *emotionStr[] = {"Anger",   "Disgust", "Fear",    "Happy",
                                   "Neutral", "Sad",     "Surprise"};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
APP_PARAM_AI_CAPTURE_CFG_S *app_ipcam_Ai_Capture_Param_Get(void)
{
    return g_pstCapCfg;
}

CVI_VOID app_ipcam_Ai_Capture_ProcStatus_Set(CVI_BOOL flag)
{
    g_bCaptureRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Capture_ProcStatus_Get(void)
{
    return g_bCaptureRunning;
}

CVI_VOID app_ipcam_Ai_Capture_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_StatusMutex);
    g_bCapturePause = flag;
    pthread_mutex_unlock(&g_StatusMutex);
}

CVI_BOOL app_ipcam_Ai_Capture_Pause_Get(void)
{
    pthread_mutex_lock(&g_StatusMutex);
    CVI_BOOL Pause = g_bCapturePause;
    pthread_mutex_unlock(&g_StatusMutex);
    return Pause;
}

CVI_VOID app_ipcam_Ai_Cap_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    if (pstAiObj == NULL) return;
    if (pstAiObj == NULL ||
        g_stObjDraw.size == 0 ||
        g_stObjDraw.info == NULL ||
        pstAiObj->info == NULL) {
        pstAiObj->size = 0;
        return;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        pstAiObj->size = g_stObjDraw.size <= 100 ? g_stObjDraw.size : 100;
        memcpy(pstAiObj->info, g_stObjDraw.info, pstAiObj->size * sizeof(TDLObjectInfo));
    }

}

static CVI_VOID *Thread_Capture_PROC(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    VPSS_GRP VpssGrp = g_pstCapCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstCapCfg->VpssChn;

    channel_frame_id = (uint64_t *)malloc(channel_size * sizeof(uint64_t));
    if (channel_frame_id) {
        memset(channel_frame_id, 0, channel_size * sizeof(uint64_t));
    }
    TDLCaptureInfo capture_info = {0};
    VIDEO_FRAME_INFO_S stCaptureFrame = {0};
    while (app_ipcam_Ai_Capture_ProcStatus_Get()) {
        if (app_ipcam_Ai_Capture_Pause_Get()) {
            usleep(1000*1000);
            continue;
        }

        for (size_t i = 0; i < channel_size; i++) {
            channel_frame_id[i] += 1;

            s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stCaptureFrame, 3000);
            if (s32Ret != 0){
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to CVI_VPSS_GetChnFrame with %x\n", s32Ret);
                continue;
            }

            TDLImage image = TDL_WrapFrame(&stCaptureFrame, true, false);
            if (image == NULL) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame for channel %s\n", channel_names[i]);
                CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stCaptureFrame);
                continue;
            }

            s32Ret = TDL_APP_SetFrame(g_CaptureTDLHandle, channel_names[i], image, channel_frame_id[i], 3);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_APP_Capture failed for channel %s with error code %#x\n", channel_names[i], s32Ret);
                continue;
            }

            memset(&capture_info, 0, sizeof(TDLCaptureInfo));

            s32Ret = TDL_APP_Capture(g_CaptureTDLHandle, channel_names[i], &capture_info);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_APP_Capture failed for channel %s with error code %#x\n", channel_names[i], s32Ret);
                continue;
            }

            {
                SMT_MutexAutoLock(g_Mutex, lock);
                g_stObjDraw.size = 0;
                if (capture_info.person_meta.size > 0 &&
                    capture_info.person_meta.info != NULL &&
                    g_stObjDraw.info != NULL) {
                    g_stObjDraw.size = capture_info.person_meta.size <= 100 ? capture_info.person_meta.size : 100;
                    memcpy(g_stObjDraw.info, capture_info.person_meta.info, g_stObjDraw.size * sizeof(TDLObjectInfo));
                }
            }

            for (uint32_t j = 0; j < capture_info.snapshot_size; j++) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "snapshot[%d]: male:%d,glass:%d,age:%d,emotion:%s\n", j,
                                    capture_info.snapshot_info[j].male,
                                    capture_info.snapshot_info[j].glass,
                                    capture_info.snapshot_info[j].age,
                                    emotionStr[capture_info.snapshot_info[j].emotion]);
                float max_similarity = 0;
                float similarity = 0;
                uint8_t top_index = 0;
                for (uint32_t k = 0; k < gallery_feature.size; k++) {
                    TDL_CaculateSimilarity(gallery_feature.feature[k],
                                        capture_info.features[j], &similarity);
                    if (similarity > max_similarity) {
                        max_similarity = similarity;
                        top_index = k;
                    }
                }

                if (max_similarity > 0.6) {
                 APP_PROF_LOG_PRINT(LEVEL_INFO, "match feature %d.bin, track id: %ld, similarity: %.2f\n",
                                    top_index, capture_info.snapshot_info[i].track_id,
                                    max_similarity);
                }
            }

            TDL_ReleaseCaptureInfo(&capture_info);

            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stCaptureFrame);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            }
            TDL_DestroyImage(image);
        }
    }

    if (channel_frame_id) {
        free(channel_frame_id);
    }

    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Capture_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Capture init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_CaptureTDLHandle == NULL)
    {
        g_CaptureTDLHandle = TDL_CreateHandle(0);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDLHandle has created\n");
        return s32Ret;
    }

     APP_PROF_LOG_PRINT(LEVEL_INFO, "g_pstCapCfg->gallery_dir = %s, g_pstCapCfg->config_file = %s\n",
                         g_pstCapCfg->gallery_dir, g_pstCapCfg->config_file);

    TDL_GetGalleryFeature(g_pstCapCfg->gallery_dir, &gallery_feature, FEATURE_SIZE);

    s32Ret = TDL_APP_Init(g_CaptureTDLHandle, "face_pet_capture", g_pstCapCfg->config_file, &channel_names, &channel_size, false);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetPerfEvalInterval failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    if (g_stObjDraw.info == NULL) {
        g_stObjDraw.info = malloc(100 * sizeof(TDLObjectInfo));
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Capture init ------------------> done \n");

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Capture_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstCapCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Capture not enable\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Capture_ProcStatus_Set(CVI_FALSE);
    pthread_join(g_CaptureThreadHandle, NULL);
    g_CaptureThreadHandle = 0;

    for (int i = 0; i < channel_size; i++) {
        free(channel_names[i]);
    }
    free(channel_names);
    free(channel_frame_id);

    if (g_stObjDraw.info != NULL) {
        free(g_stObjDraw.info);
        g_stObjDraw.info = NULL;
    }

    for (uint32_t i = 0; i < gallery_feature.size; i++) {
        TDL_ReleaseFeatureMeta(&gallery_feature.feature[i]);
    }

    s32Ret = TDL_DestroyHandle(g_CaptureTDLHandle);

    if(s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "enter TDL_DestroyHandle fail \n");
        return s32Ret;
    }
    g_CaptureTDLHandle = NULL;

    return s32Ret;
}

int app_ipcam_Ai_Capture_Start(void){
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstCapCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Capture not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bCaptureRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Capture has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Capture_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Capture_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Capture_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_CaptureThreadHandle, NULL, Thread_Capture_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI Capture_pthread_create failed!\n");
        return s32Ret;
    }

    return s32Ret;
}
