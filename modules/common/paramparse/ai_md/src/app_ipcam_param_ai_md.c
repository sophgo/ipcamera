#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


int Load_Param_Ai_MD(const char * file)
{
    APP_PARAM_AI_MD_CFG_S *Md = app_ipcam_Ai_MD_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading MD config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_md_config");

    Md->bEnable       = ini_getl(tmp_section, "md_enable", 0, file);
    Md->VpssGrp       = ini_getl(tmp_section, "vpss_grp", 0, file);
    Md->VpssChn       = ini_getl(tmp_section, "vpss_chn", 0, file);
    Md->u32GrpWidth   = ini_getl(tmp_section, "grp_width", 0, file);
    Md->u32GrpHeight  = ini_getl(tmp_section, "grp_height", 0, file);
    Md->threshold     = ini_getf(tmp_section, "threshold", 0, file);
    Md->miniArea      = ini_getl(tmp_section, "miniArea", 256, file);
    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Md->model_path, tmp_buff, 128);

    ini_gets(tmp_section, "model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Md->model_id = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n",
        Md->bEnable, Md->VpssGrp, Md->VpssChn, Md->u32GrpWidth, Md->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%f miniArea=%d\n",
        Md->threshold, Md->miniArea);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Md->model_id, Md->model_path);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading MD config ------------------> done \n\n");

    return CVI_SUCCESS;
}