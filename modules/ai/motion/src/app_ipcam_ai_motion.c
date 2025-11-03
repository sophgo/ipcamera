#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"
#include <cvi_comm_isp.h>

static APP_PARAM_AI_Motion_CFG_S g_stMotionCfg;
static APP_PARAM_AI_Motion_CFG_S *g_pstMotionCfg = &g_stMotionCfg;

APP_PARAM_AI_Motion_CFG_S *app_ipcam_Ai_Motion_Param_Get(void)
{
    return g_pstMotionCfg;
}

int app_ipcam_Ai_Motion_Start(void)
{
    if (g_pstMotionCfg->bEnable == false) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "ai motion not enable\n");
        return CVI_SUCCESS;
    }
    
    VI_PIPE ViPipe = 0;
    CVI_S32 dev = 0;
    CVI_S32 iso = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;
    TEAISP_BNR_MODEL_INFO_S stModelInfo;

    ViPipe = g_pstMotionCfg->ViPipe;
    dev = g_pstMotionCfg->dev_num;//according to ISP init settings
    iso = g_pstMotionCfg->iso;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "ai motion enable, ViPipe=%d, dev=%d, iso=%d\n", ViPipe, dev, iso);

    s32Ret = CVI_TEAISP_Init(ViPipe, dev);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TEAISP_Init failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    //load_bnr_model
    memset(&stModelInfo, 0, sizeof(TEAISP_BNR_MODEL_INFO_S));
    snprintf(stModelInfo.path, TEAISP_MODEL_PATH_LEN, "%s", g_pstMotionCfg->model_path);

    stModelInfo.enterISO = iso;
    stModelInfo.tolerance = iso * 10 / 100;

    s32Ret = CVI_TEAISP_BNR_SetModel(ViPipe, &stModelInfo);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TEAISP_BNR_SetModel failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Motion_Stop(void)
{
    return CVI_SUCCESS;
}