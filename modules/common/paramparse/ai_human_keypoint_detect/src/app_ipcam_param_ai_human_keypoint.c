#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_HumanKeypoint(const char * file)
{
    APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *Ai = app_ipcam_Ai_Human_Keypoint_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[40] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Human Keypoint Detection config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_keypoint_config");

    Ai->bEnable             = ini_getl(tmp_section, "keypoint_enable", 0, file);
    Ai->VpssGrp             = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn             = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->u32GrpWidth         = ini_getl(tmp_section, "grp_width", 0, file);
    Ai->u32GrpHeight        = ini_getl(tmp_section, "grp_height", 0, file);
    Ai->threshold           = ini_getf(tmp_section, "threshold", 0, file);

    ini_gets(tmp_section, "model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Convert string name [%s] to enum number [%d]!\n", tmp_section, str_name, enum_num);
        Ai->model_id = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d \n",
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d threshold=%f\n",
        Ai->u32GrpWidth, Ai->u32GrpHeight,  Ai->bVpssPreProcSkip, Ai->threshold);

    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id, Ai->model_path);

    float color_r = ini_getf(tmp_section, "color_r", 255.0f, file);
    float color_g = ini_getf(tmp_section, "color_g", 0.0f, file);
    float color_b = ini_getf(tmp_section, "color_b", 0.0f, file);
    CVI_S32 point_size = ini_getl(tmp_section, "point_size", 0, file);
    CVI_S32 line_width = ini_getl(tmp_section, "line_width", 0, file);

    if (color_r < 0.0f) color_r = 0.0f;
    if (color_r > 255.0f) color_r = 255.0f;
    if (color_g < 0.0f) color_g = 0.0f;
    if (color_g > 255.0f) color_g = 255.0f;
    if (color_b < 0.0f) color_b = 0.0f;
    if (color_b > 255.0f) color_b = 255.0f;

    Ai->color_r = (CVI_U8)(color_r + 0.5f);
    Ai->color_g = (CVI_U8)(color_g + 0.5f);
    Ai->color_b = (CVI_U8)(color_b + 0.5f);
    Ai->point_size = (CVI_U32)point_size;
    Ai->line_width = (CVI_U32)line_width;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "color_r=%d color_g=%d color_b=%d point_size=%u line_width=%u\n",
        Ai->color_r, Ai->color_g, Ai->color_b, Ai->point_size, Ai->line_width);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Human Keypoint Detection config ------------------> done \n\n");

    return CVI_SUCCESS;
}
