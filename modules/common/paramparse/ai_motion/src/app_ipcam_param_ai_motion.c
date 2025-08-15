#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


int Load_Param_Ai_Motion(const char * file)
{
    APP_PARAM_AI_Motion_CFG_S *Motion = app_ipcam_Ai_Motion_Param_Get();
    char tmp_section[20] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Motion config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_motion_config");

    Motion->bEnable           = ini_getl(tmp_section, "motion_enable", 0, file);
    Motion->ViPipe            = ini_getl(tmp_section, "ViPipe", 0, file);
    Motion->dev_num           = ini_getl(tmp_section, "dev_num", 0, file);
    Motion->iso               = ini_getl(tmp_section, "iso", 0, file);
    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Motion->model_path, tmp_buff, 128);


    APP_PROF_LOG_PRINT(LEVEL_INFO, "motion_enable=%d ViPipe=%d dev_num=%d iso=%d\n", \
        Motion->bEnable, Motion->ViPipe, Motion->dev_num, Motion->iso);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_path=%s \n", Motion->model_path);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Motion config ------------------> done \n\n");

    return CVI_SUCCESS;
}
