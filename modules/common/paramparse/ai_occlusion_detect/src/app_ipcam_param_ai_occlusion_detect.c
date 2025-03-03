#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


int Load_Param_Ai_Occlusion(const char * file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Occlusion Detection config ------------------> start \n");

    APP_PARAM_AI_OCCLUSION_CFG_S *OcclusionObj = app_ipcam_Ai_Occlusion_Param_Get();

    OcclusionObj->bEnable                      = ini_getl("occlusion_config", "occlusion_config_enable", 0, file);
    OcclusionObj->VpssGrp                      = ini_getl("occlusion_config", "vpss_grp", 0, file);
    OcclusionObj->VpssChn                      = ini_getl("occlusion_config", "vpss_chn", 0, file);
    OcclusionObj->u32ChnWidth                  = ini_getl("occlusion_config", "chn_width", 0, file);
    OcclusionObj->u32ChnHeight                 = ini_getl("occlusion_config", "chn_height", 0, file);
    OcclusionObj->occlusion_meta.crop_bbox.x1  = ini_getf("occlusion_config", "x1", 0.0, file);
    OcclusionObj->occlusion_meta.crop_bbox.y1  = ini_getf("occlusion_config", "y1", 0.0, file);
    OcclusionObj->occlusion_meta.crop_bbox.x2  = ini_getf("occlusion_config", "x2", 1.0, file);
    OcclusionObj->occlusion_meta.crop_bbox.y2  = ini_getf("occlusion_config", "y2", 1.0, file);
    OcclusionObj->occlusion_meta.laplacian_th  = ini_getf("occlusion_config", "laplacian_th", 10.0, file);
    OcclusionObj->occlusion_meta.occ_ratio_th  = ini_getf("occlusion_config", "occ_ratio_th", 0.5, file);
    OcclusionObj->occlusion_meta.sensitive_th  = ini_getf("occlusion_config", "sensitive_th", 1.0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d ChnWidth=%d ChnHeight=%d \n",
        OcclusionObj->bEnable, OcclusionObj->VpssGrp, OcclusionObj->VpssChn, OcclusionObj->u32ChnWidth, OcclusionObj->u32ChnHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "x1=%f y1=%f x2=%f y2=%f \n",
        OcclusionObj->occlusion_meta.crop_bbox.x1, OcclusionObj->occlusion_meta.crop_bbox.y1, 
        OcclusionObj->occlusion_meta.crop_bbox.x2, OcclusionObj->occlusion_meta.crop_bbox.y2);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "laplacian_th=%f occ_ratio_th=%f sensitive_th=%f \n",
        OcclusionObj->occlusion_meta.laplacian_th, OcclusionObj->occlusion_meta.occ_ratio_th, OcclusionObj->occlusion_meta.sensitive_th);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Occlusion Detection config ------------------> done \n\n");

    return CVI_SUCCESS;
}
