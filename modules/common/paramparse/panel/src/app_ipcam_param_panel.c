#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Panel(const char * file)
{
    int ret = 0;
    int enum_num = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};

    PANEL_DESC_S *pst_panel_cfg = app_ipcam_panel_param_get();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading panel config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    sprintf(tmp_section, "panel_config");
    pst_panel_cfg->panel_num = ini_getl(tmp_section, "panel_num", 0, file);
    ini_gets(tmp_section, "panel_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, panel_type_name, PANEL_MAX, &enum_num);
    if (ret == CVI_SUCCESS) {
        pst_panel_cfg->panel_type = (PANEL_TYPE_E)enum_num;
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Parsed panel_type: %s (%d)\n", str_name, pst_panel_cfg->panel_type);
    } else {
        pst_panel_cfg->panel_type = DSI_PANEL_HX8394_EVB;
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Failed to parse panel_type, using default DSI_PANEL_HX8394_EVB\n");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading panel config ------------------> done \n");

    return CVI_SUCCESS;
}
