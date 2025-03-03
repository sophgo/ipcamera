#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"
#include "app_ipcam_osd.h"
#include "cvi_kit.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_OcclusionMutex);
static pthread_mutex_t g_OcclusionStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_OCCLUSION_CFG_S g_stOcclusionCfg;

static APP_PARAM_AI_OCCLUSION_CFG_S *g_pstOcclusionCfg = &g_stOcclusionCfg;


static volatile bool g_bOcclusionRunning = CVI_FALSE;
static volatile bool g_bOcclusionPause = CVI_FALSE;
static pthread_t g_OcclusionThreadHandle;
static cvtdl_occlusion_meta_t g_stOcclusionObj;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_OCCLUSION_CFG_S *app_ipcam_Ai_Occlusion_Param_Get(void)
{
    return g_pstOcclusionCfg;
}

CVI_VOID app_ipcam_Ai_Occlusion_ProcStatus_Set(CVI_BOOL flag)
{
    g_bOcclusionRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Occlusion_ProcStatus_Get(void)
{
    return g_bOcclusionRunning;
}

CVI_VOID app_ipcam_Ai_Occlusion_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_OcclusionStatusMutex);
    g_bOcclusionPause = flag;
    pthread_mutex_unlock(&g_OcclusionStatusMutex);
}

CVI_BOOL app_ipcam_Ai_Occlusion_Pause_Get(void)
{
    return g_bOcclusionPause;
}


static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d\n", 
        g_pstOcclusionCfg->bEnable, g_pstOcclusionCfg->VpssGrp, g_pstOcclusionCfg->VpssChn);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "chn_w=%d chn_h=%d \n",
        g_pstOcclusionCfg->u32ChnWidth, g_pstOcclusionCfg->u32ChnHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "x1=%f y1=%f x2=%f y2=%f \n",
        g_pstOcclusionCfg->occlusion_meta.crop_bbox.x1, g_pstOcclusionCfg->occlusion_meta.crop_bbox.y1, 
        g_pstOcclusionCfg->occlusion_meta.crop_bbox.x2, g_pstOcclusionCfg->occlusion_meta.crop_bbox.y2);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "laplacian_th=%f occ_ratio_th=%f sensitive_th=%f \n",
        g_pstOcclusionCfg->occlusion_meta.laplacian_th, g_pstOcclusionCfg->occlusion_meta.occ_ratio_th, g_pstOcclusionCfg->occlusion_meta.sensitive_th);
}

static CVI_S32 app_ipcam_Ai_Occlusion_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Occlusion Detection init ------------------> start \n");

    app_ipcam_Ai_Param_dump();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Occlusion Detection init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_Occlusion_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Occlusion Detection start running!\n");

    prctl(PR_SET_NAME, "Thread_Occlusion_PROC");

    VPSS_GRP VpssGrp = g_pstOcclusionCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstOcclusionCfg->VpssChn;

    VIDEO_FRAME_INFO_S stFrame = {0};

    while (app_ipcam_Ai_Occlusion_ProcStatus_Get()) {
        pthread_mutex_lock(&g_OcclusionStatusMutex);
        s32Ret = app_ipcam_Ai_Occlusion_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_OcclusionStatusMutex);
            usleep(1000*1000);
            continue;
        }
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_OcclusionStatusMutex);
            usleep(100*1000);
            continue;
        }
        pthread_mutex_unlock(&g_OcclusionStatusMutex);
        
        cvtdl_occlusion_meta_t obj_meta;
        memset(&obj_meta, 0, sizeof(cvtdl_occlusion_meta_t));
        obj_meta.crop_bbox = g_stOcclusionCfg.occlusion_meta.crop_bbox;
        obj_meta.laplacian_th = g_stOcclusionCfg.occlusion_meta.laplacian_th;
        obj_meta.occ_ratio_th = g_stOcclusionCfg.occlusion_meta.occ_ratio_th;
        obj_meta.sensitive_th = g_stOcclusionCfg.occlusion_meta.sensitive_th;

        CVI_TDL_Set_Occlusion_Laplacian(&stFrame, &obj_meta);

        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Occlusion Detect score: %f \n", obj_meta.occ_score);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Occlusion Detect class: %d \n", obj_meta.occ_class);

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }

        SMT_MutexAutoLock(g_OcclusionMutex, lock);

        memset(&g_stOcclusionObj, 0, sizeof(cvtdl_occlusion_meta_t));
        memcpy(&g_stOcclusionObj, &obj_meta, sizeof obj_meta);

    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_Occlusion_ObjDrawInfo_Get(cvtdl_occlusion_meta_t *pstOcclusionObj)
{
    _NULL_POINTER_CHECK_(pstOcclusionObj, -1);

    SMT_MutexAutoLock(g_OcclusionMutex, lock);

    memcpy(pstOcclusionObj, &g_stOcclusionObj, sizeof g_stOcclusionObj);

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Occlusion_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstOcclusionCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Occlusion Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Occlusion_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Occlusion Detection has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Occlusion_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_OcclusionThreadHandle)
    {
        pthread_join(g_OcclusionThreadHandle, NULL);
        g_OcclusionThreadHandle = 0;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Occlusion Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Occlusion_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstOcclusionCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Occlusion Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bOcclusionRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Occlusion Detection has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Occlusion_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Occlusion_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Occlusion_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_OcclusionThreadHandle, NULL, Thread_Occlusion_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}
