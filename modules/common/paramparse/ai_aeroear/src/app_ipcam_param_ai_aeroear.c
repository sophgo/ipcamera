/*
 * app_ipcam_param_ai_aeroear.c — AeroEar parameter parser
 *
 * Reads [ai_aeroear_config] section from ini file:
 *   enable / model_path / model_id / snr_threshold / act_threshold
 * Results are exposed via app_ipcam_Ai_Aeroear_Param_Get().
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_Aeroear(const char *file)
{
    APP_PARAM_AI_AEROEAR_CFG_S *Ai = app_ipcam_Ai_Aeroear_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char **ai_supported_model = app_ipcam_Param_get_ai_supported_model();
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "loading AI Aeroear config ------------------> start\n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_aeroear_config");

    Ai->bEnable = ini_getl(tmp_section, "aeroear_enable", 0, file);
    Ai->model_id = ini_getl(tmp_section, "model_id", 0, file);
    Ai->snr_threshold = ini_getf(tmp_section, "snr_threshold", 0.0f, file);
    Ai->act_threshold = ini_getf(tmp_section, "act_threshold", 0.3f, file);
    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path, tmp_buff, 128);

    /* Try to convert model_id string to enum value */
    ini_gets(tmp_section, "model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name,
        ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "[%s][model_id] Fail to convert string name [%s] to enum number!\n",
            tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "[%s][model_id] Convert string name [%s] to enum number [%d]!\n",
            tmp_section, str_name, enum_num);
        Ai->model_id = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "model_id=%d model_path=%s snr_thresh=%.1f act_thresh=%.2f\n",
        Ai->model_id, Ai->model_path,
        Ai->snr_threshold, Ai->act_threshold);

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "loading AI Aeroear config ------------------> done\n\n");

    return CVI_SUCCESS;
}