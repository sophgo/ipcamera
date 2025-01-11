#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_blacklight.h"

int Load_Param_BlackLight(const char * file)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    char tmp_section[32] = {0};
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading blacklight config ------------------> start \n");

    APP_PARAM_BLACK_LIGHT_CTX_S *pstBlackLightCtx = app_ipcam_BlackLight_Param_Get();

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "black_light_config");

    pstBlackLightCtx->VencChn    = ini_getl(tmp_section, "venc_chn", 0, file);
    pstBlackLightCtx->VpssGrpRGB = ini_getl(tmp_section, "rgb_vpss_grp", 0, file);
    pstBlackLightCtx->VpssChnRGB = ini_getl(tmp_section, "rgb_vpss_chn", 0, file);
    pstBlackLightCtx->VpssGrpIR  = ini_getl(tmp_section, "ir_vpss_grp", 0, file);
    pstBlackLightCtx->VpssChnIR  = ini_getl(tmp_section, "ir_vpss_chn", 0, file);
    pstBlackLightCtx->VpssGrpBlackLight = ini_getl(tmp_section, "blacklight_vpss_grp", 0, file);
    pstBlackLightCtx->VpssChnBlackLight = ini_getl(tmp_section, "blacklight_vpss_chn", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "vencChn:%d, RGB[%d %d], IR[%d %d], blacklight[%d %d]. \n"
        , pstBlackLightCtx->VencChn
        , pstBlackLightCtx->VpssGrpRGB
        , pstBlackLightCtx->VpssChnRGB
        , pstBlackLightCtx->VpssGrpIR
        , pstBlackLightCtx->VpssChnIR
        , pstBlackLightCtx->VpssGrpBlackLight
        , pstBlackLightCtx->VpssChnBlackLight);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading blacklight config ------------------> done \n\n");

    return s32Ret;
}
