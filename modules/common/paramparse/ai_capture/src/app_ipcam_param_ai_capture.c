#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_CAPTURE(const char * file)
{
    APP_PARAM_AI_CAPTURE_CFG_S *Ai = app_ipcam_Ai_Capture_Param_Get();
    char tmp_section[32] = {0};
    char tmp_buff[128] = {0};
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Capture config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_capture_config");

    Ai->bEnable           = ini_getl(tmp_section, "capture_enable", 0, file);
    Ai->VpssGrp           = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn           = ini_getl(tmp_section, "vpss_chn", 0, file);

    ini_gets(tmp_section, "config_file", " ", tmp_buff, 128, file);
    strncpy(Ai->config_file, tmp_buff, 128);
    ini_gets(tmp_section, "gallery_dir", " ", tmp_buff, 128, file);
    strncpy(Ai->gallery_dir, tmp_buff, 128);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Capture config ------------------> done \n\n");

    return CVI_SUCCESS;
}
