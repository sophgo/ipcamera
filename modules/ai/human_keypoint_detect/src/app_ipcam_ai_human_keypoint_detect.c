#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "app_ipcam_ai.h"
#include "app_ipcam_osd.h"

// Ai Model info
/*****************************************************************
 * Model Func : Human Keypoint Detection
 * Model ID   : CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE
 * Inference  : CVI_TDL_PoseDetection
 * Model file : yolov8n_pose_384_640.cvimodel (input size 1x3x640x384)
****************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_HumanKeypointMutex);
static pthread_mutex_t g_HumanKeypointStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S g_stHumanKeypointCfg;

static APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *g_pstHumanKeypointCfg = &g_stHumanKeypointCfg;

static CVI_U32 g_HumanKeypointProc;
static volatile bool g_bHumanKeypointRunning = CVI_FALSE;
static volatile bool g_bHumanKeypointPause = CVI_FALSE;
static pthread_t g_HumanKeypointThreadHandle;
static cvitdl_handle_t g_HumanKeypointAiHandle = NULL;
static cvitdl_service_handle_t g_HumanKeypointAiServiceHandle = NULL;
static cvtdl_object_t g_stHumanKeypointObjDraw;
static pfpInferenceFunc g_pfpHumanKeypointInference;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *app_ipcam_Ai_Human_Keypoint_Param_Get(void)
{
    return g_pstHumanKeypointCfg;
}

CVI_VOID app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_BOOL flag)
{
    g_bHumanKeypointRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Human_Keypoint_ProcStatus_Get(void)
{
    return g_bHumanKeypointRunning;
}

CVI_VOID app_ipcam_Ai_Human_Keypoint_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_HumanKeypointStatusMutex);
    g_bHumanKeypointPause = flag;
    pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
}

CVI_BOOL app_ipcam_Ai_Human_Keypoint_Pause_Get(void)
{
    return g_bHumanKeypointPause;
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(CVI_TDL_SUPPORTED_MODEL_E model_id)
{
    switch(model_id)
    {
        case CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE:
            g_pfpHumanKeypointInference = CVI_TDL_PoseDetection;
        break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d\n", 
        g_pstHumanKeypointCfg->bEnable, g_pstHumanKeypointCfg->VpssGrp, g_pstHumanKeypointCfg->VpssChn);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d threshold=%f\n",
        g_pstHumanKeypointCfg->model_size_w, g_pstHumanKeypointCfg->model_size_h, g_pstHumanKeypointCfg->bVpssPreProcSkip, g_pstHumanKeypointCfg->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, " model_id=%d model_path=%s\n",
        g_pstHumanKeypointCfg->model_id, g_pstHumanKeypointCfg->model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, " color r=%f g=%f b=%f size=%d\n",
        g_pstHumanKeypointCfg->rect_brush.color.r, g_pstHumanKeypointCfg->rect_brush.color.g,
        g_pstHumanKeypointCfg->rect_brush.color.b, g_pstHumanKeypointCfg->rect_brush.size);

}

static CVI_S32 app_ipcam_Ai_VpssChnAttr_Set(CVI_VOID)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_TDL_SUPPORTED_MODEL_E model_id = CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE;
    VPSS_GRP VpssGrp = g_pstHumanKeypointCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstHumanKeypointCfg->VpssChn;

    APP_PARAM_VPSS_CFG_T *pstVpssCfg = app_ipcam_Vpss_Param_Get();
    VPSS_CHN_ATTR_S *pastVpssChnAttr = &pstVpssCfg->astVpssGrpCfg[VpssGrp].astVpssChnAttr[VpssChn];
    CVI_U32 u32Width = pstVpssCfg->astVpssGrpCfg[VpssGrp].stVpssGrpAttr.u32MaxW;
    CVI_U32 u32Height = pstVpssCfg->astVpssGrpCfg[VpssGrp].stVpssGrpAttr.u32MaxH;
    CVI_S32 s32SrcFrameRate = pastVpssChnAttr->stFrameRate.s32SrcFrameRate;
    CVI_S32 s32DstFrameRate = pastVpssChnAttr->stFrameRate.s32DstFrameRate;
    CVI_U32 u32Depth = pastVpssChnAttr->u32Depth;

    cvtdl_vpssconfig_t vpssConfig;
    memset(&vpssConfig, 0, sizeof(vpssConfig));

    s32Ret = CVI_TDL_GetVpssChnConfig(
                            g_HumanKeypointAiHandle,
                            model_id,
                            u32Width,
                            u32Height,
                            0,
                            &vpssConfig);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_GetVpssChnConfig failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    vpssConfig.chn_attr.u32Depth = u32Depth;
    vpssConfig.chn_attr.stFrameRate.s32SrcFrameRate = s32SrcFrameRate;
    vpssConfig.chn_attr.stFrameRate.s32DstFrameRate = s32DstFrameRate;

    s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, vpssConfig.chn_coeff);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetChnScaleCoefLevel failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpssConfig.chn_attr u32Width=%d u32Height=%d SrcFR=%d DstFR=%d\n", 
                    vpssConfig.chn_attr.u32Width, vpssConfig.chn_attr.u32Height, s32SrcFrameRate, s32DstFrameRate);

    s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &vpssConfig.chn_attr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetChnAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_EnableChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_TDL_SetVpssTimeout(g_HumanKeypointAiHandle, 2000);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "set vpss timeout failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Ai_Human_Keypoint_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    CVI_TDL_SUPPORTED_MODEL_E model_id = g_pstHumanKeypointCfg->model_id;

    if (g_HumanKeypointAiHandle == NULL)
    {
        s32Ret = CVI_TDL_CreateHandle(&g_HumanKeypointAiHandle);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle has created\n");
        return s32Ret;
    }

    if (g_HumanKeypointAiServiceHandle == NULL)
    {
        s32Ret = CVI_TDL_Service_CreateHandle(&g_HumanKeypointAiServiceHandle, g_HumanKeypointAiHandle);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_Service_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", model_id);
        return s32Ret;
    }

    s32Ret = CVI_TDL_OpenModel(g_HumanKeypointAiHandle, model_id, g_pstHumanKeypointCfg->model_path);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelPath failed with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_TDL_SetModelThreshold(g_HumanKeypointAiHandle, model_id, g_pstHumanKeypointCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelThreshold failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    CVI_BOOL bSkip = g_pstHumanKeypointCfg->bVpssPreProcSkip;
    CVI_TDL_SetSkipVpssPreprocess(g_HumanKeypointAiHandle, model_id, bSkip);
    if (bSkip)
    {
        s32Ret = app_ipcam_Ai_VpssChnAttr_Set();
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "VpssChnAttr Set failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_Human_Keypoint_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection start running!\n");

    prctl(PR_SET_NAME, "Thread_Human_Keypoint_PROC");

    VPSS_GRP VpssGrp = g_pstHumanKeypointCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstHumanKeypointCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};

    while (app_ipcam_Ai_Human_Keypoint_ProcStatus_Get()) {
        pthread_mutex_lock(&g_HumanKeypointStatusMutex);
        s32Ret = app_ipcam_Ai_Human_Keypoint_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
            usleep(1000*1000);
            continue;
        }
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
            usleep(100*1000);
            continue;
        }
        pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
        cvtdl_object_t obj_meta;
        memset(&obj_meta, 0, sizeof(cvtdl_object_t));

        g_pfpHumanKeypointInference(g_HumanKeypointAiHandle, &stfdFrame, CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE, &obj_meta);

        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Human Keypoint Detect obj: %d \n", obj_meta.size);
        APP_PROF_LOG_PRINT(LEVEL_TRACE, "Human Keypoint Detect process takes %d\n", g_HumanKeypointProc);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        if (obj_meta.size == 0) {
            continue;
        }
        SMT_MutexAutoLock(g_HumanKeypointMutex, lock);
        if (g_stHumanKeypointObjDraw.info != NULL) {
            CVI_TDL_Free(&g_stHumanKeypointObjDraw);
        }
        memset(&g_stHumanKeypointObjDraw, 0, sizeof(cvtdl_object_t));
        memcpy(&g_stHumanKeypointObjDraw, &obj_meta, sizeof obj_meta);
        g_stHumanKeypointObjDraw.info = (cvtdl_object_info_t *)malloc(obj_meta.size * sizeof(cvtdl_object_info_t));
        memset(g_stHumanKeypointObjDraw.info, 0, sizeof(cvtdl_object_info_t) * g_stHumanKeypointObjDraw.size);
        for (uint32_t i = 0; i < obj_meta.size; i++) {
            CVI_TDL_CopyObjectInfo(&obj_meta.info[i], &g_stHumanKeypointObjDraw.info[i]);
        }

        CVI_TDL_Free(&obj_meta);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_Human_Keypoint_ObjDrawInfo_Get(cvtdl_object_t *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    SMT_MutexAutoLock(g_HumanKeypointMutex, lock);

    if (g_stHumanKeypointObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        memcpy(pstAiObj, &g_stHumanKeypointObjDraw, sizeof g_stHumanKeypointObjDraw);
        pstAiObj->info = (cvtdl_object_info_t *)malloc(g_stHumanKeypointObjDraw.size * sizeof(cvtdl_object_info_t));
        _NULL_POINTER_CHECK_(pstAiObj->info, -1);
        memset(pstAiObj->info, 0, sizeof(cvtdl_object_info_t) * g_stHumanKeypointObjDraw.size);
        for (CVI_U32 i = 0; i < g_stHumanKeypointObjDraw.size; i++) {
            CVI_TDL_CopyObjectInfo(&g_stHumanKeypointObjDraw.info[i], &pstAiObj->info[i]);
            pstAiObj->info[i].vehicle_properity = NULL;
        }
        CVI_TDL_Free(&g_stHumanKeypointObjDraw);
    }

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Human_Keypoint_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstHumanKeypointCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Human_Keypoint_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_HumanKeypointThreadHandle)
    {
        pthread_join(g_HumanKeypointThreadHandle, NULL);
        g_HumanKeypointThreadHandle = 0;
    }

    s32Ret = CVI_TDL_Service_DestroyHandle(g_HumanKeypointAiServiceHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_Service_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_HumanKeypointAiServiceHandle = NULL;
    }

    s32Ret = CVI_TDL_DestroyHandle(g_HumanKeypointAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_HumanKeypointAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI HumanKeypoint Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Human_Keypoint_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstHumanKeypointCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bHumanKeypointRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Human_Keypoint_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_HumanKeypoint_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_HumanKeypointThreadHandle, NULL, Thread_Human_Keypoint_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Ai_Human_Keypoint_StatusGet(void)
{
    return g_HumanKeypointAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Human_Keypoint_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_HumanKeypointAiHandle) {
        s32Ret = CVI_TDL_SetModelThreshold(g_HumanKeypointAiHandle, g_pstHumanKeypointCfg->model_id, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s CVI_TDL_SetModelThreshold failed with %#x!\n", g_pstHumanKeypointCfg->model_path, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}
