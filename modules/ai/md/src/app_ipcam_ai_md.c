#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_MDMutex);
static pthread_mutex_t g_MdStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_MD_CFG_S g_stMdCfg;
static APP_PARAM_AI_MD_CFG_S *g_pstMdCfg = &g_stMdCfg;

static CVI_U32 g_MDFps;
static CVI_U32 g_MDProc;
static float g_MDScaleX, g_MDScaleY;
static volatile bool g_bMDRunning = CVI_FALSE;
static volatile bool g_bMDPause = CVI_FALSE;
static pthread_t g_MDThreadHandle;
static TDLHandle g_MDAiHandle = NULL;
static TDLObject g_stMDObjDraw;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_MD_CFG_S *app_ipcam_Ai_MD_Param_Get(void)
{
    return g_pstMdCfg;
}

CVI_VOID app_ipcam_Ai_MD_Thresold_Set(float value)
{
    if (g_MDAiHandle) {
        TDL_SetModelThreshold(g_MDAiHandle, g_pstMdCfg->model_id, value);
    }
    g_pstMdCfg->threshold = value;
}

float app_ipcam_Ai_MD_Thresold_Get(void)
{
    return g_pstMdCfg->threshold;
}

CVI_VOID app_ipcam_Ai_MD_ProcStatus_Set(CVI_BOOL flag)
{
    g_bMDRunning = flag;
}

CVI_BOOL app_ipcam_Ai_MD_ProcStatus_Get(void)
{
    return g_bMDRunning;
}

CVI_VOID app_ipcam_Ai_MD_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_MdStatusMutex);
    g_bMDPause = flag;
    pthread_mutex_unlock(&g_MdStatusMutex);
}

CVI_BOOL app_ipcam_Ai_MD_Pause_Get(void)
{
    return g_bMDPause;
}

CVI_U32 app_ipcam_Ai_MD_ProcFps_Get(void)
{
    return g_MDFps;
}

CVI_S32 app_ipcam_Ai_MD_ProcTime_Get(void)
{
    return g_MDProc;
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_TOPFORMER_SEG_MOTION:
        break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n",
        g_pstMdCfg->bEnable, g_pstMdCfg->VpssGrp, g_pstMdCfg->VpssChn, g_pstMdCfg->u32GrpWidth, g_pstMdCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%f miniArea=%d\n",
        g_pstMdCfg->threshold, g_pstMdCfg->miniArea);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n", g_pstMdCfg->model_id, g_pstMdCfg->model_path);

}

/**
 * 浅拷贝 TDLObject 结构体（只拷贝指针，不分配新内存）
 * 源对象需由调用方归零或不再使用，避免 double-free
 * @param dst 目标对象
 * @param src 源对象
 */
static void shallow_copy_tdl_object(TDLObject* dst, TDLObject* src) {
    dst->size = src->size;
    dst->width = src->width;
    dst->height = src->height;
    dst->info = src->info;
    src->info = NULL;
    src->size = 0;
}

static CVI_S32 app_ipcam_Ai_MD_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    if (g_MDAiHandle == NULL)
    {
        g_MDAiHandle = TDL_CreateHandle(0);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstMdCfg->model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", g_pstMdCfg->model_id);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_MDAiHandle, g_pstMdCfg->model_id, g_pstMdCfg->model_path, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel failed with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_SetModelThreshold(g_MDAiHandle, g_pstMdCfg->model_id, g_pstMdCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetModelThreshold failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_MD_Proc(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD start running!\n");

    prctl(PR_SET_NAME, "Thread_MD_Proc");

    VPSS_GRP VpssGrp = g_pstMdCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstMdCfg->VpssChn;
    CVI_U32 miniArea = g_pstMdCfg->miniArea;

    VIDEO_FRAME_INFO_S stMdFrame = {0};
    TDLImage image_handle;

    CVI_U32 md_frame = 0;
    CVI_S32 iTime_start, iTime_proc, iTime_fps;
    iTime_start = GetCurTimeInMsec();

    while (app_ipcam_Ai_MD_ProcStatus_Get()) {
        pthread_mutex_lock(&g_MdStatusMutex);
        s32Ret = app_ipcam_Ai_MD_Pause_Get();

        if (s32Ret) {
            pthread_mutex_unlock(&g_MdStatusMutex);
            usleep(1000*1000);
            continue;
        }
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stMdFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_MdStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stMdFrame, false, false);

        pthread_mutex_unlock(&g_MdStatusMutex);

        iTime_proc = GetCurTimeInMsec();

        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));
        TDL_SegMotionDetection(g_MDAiHandle, g_pstMdCfg->model_id, image_handle, miniArea, &obj_meta);
        APP_PROF_LOG_PRINT(LEVEL_TRACE, "MD obj: %d \n", obj_meta.size);

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stMdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);

        g_MDProc = GetCurTimeInMsec() - iTime_proc;
        md_frame ++;
        iTime_fps = GetCurTimeInMsec();
        float iTime_gop = (float)(iTime_fps - iTime_start)/1000;
        if(iTime_gop >= 1)
        {
            g_MDFps = md_frame/iTime_gop;
            md_frame = 0;
            iTime_start = iTime_fps;
        }

        if (obj_meta.size == 0) {
            TDL_ReleaseObjectMeta(&obj_meta);
            continue;
        }
        {
            SMT_MutexAutoLock(g_MDMutex, lock);
            if (g_stMDObjDraw.info != NULL) {
                TDL_ReleaseObjectMeta(&g_stMDObjDraw);
            }
            memset(&g_stMDObjDraw, 0, sizeof(TDLObject));
            shallow_copy_tdl_object(&g_stMDObjDraw, &obj_meta);
        }
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_MD_ObjDrawInfo_Get(TDLObject *pstMdObj)
{
    _NULL_POINTER_CHECK_(pstMdObj, -1);

    SMT_MutexAutoLock(g_MDMutex, lock);

    if (g_stMDObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        shallow_copy_tdl_object(pstMdObj, &g_stMDObjDraw);
    }
    return CVI_SUCCESS;
}

int app_ipcam_Ai_MD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstMdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_MD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_MD_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_MDThreadHandle)
    {
        pthread_join(g_MDThreadHandle, NULL);
        g_MDThreadHandle = 0;
    }

    TDL_CloseModel(g_MDAiHandle, g_pstMdCfg->model_id);
    s32Ret = TDL_DestroyHandle(g_MDAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_MDAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_MD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_VPSS_GRP_CFG_T *pstVpssCfg = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[0];

    if (!g_pstMdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bMDRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_MD_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_MD_Proc_Init failed!\n");
        return s32Ret;
    }

    g_MDScaleX = g_MDScaleY = fmax(((float)pstVpssCfg->astVpssChnAttr[0].u32Width / (float)g_pstMdCfg->u32GrpWidth), \
                                     ((float)pstVpssCfg->astVpssChnAttr[0].u32Height / (float)g_pstMdCfg->u32GrpHeight));

    app_ipcam_Ai_MD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_MDThreadHandle, NULL, Thread_MD_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
CVI_S32 app_ipcam_Ai_MD_StatusGet(void)
{
    return g_MDAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Md_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_MDAiHandle) {
        s32Ret = TDL_SetModelThreshold(g_MDAiHandle, g_pstMdCfg->model_id, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n",g_pstMdCfg->model_path, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}