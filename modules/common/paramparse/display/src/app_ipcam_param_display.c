#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

#define VO_INTF_TYPE_MAX ((0x01L << 16) + 1)
const char *panel_name[PANEL_MAX] = {
    [PANEL_DSI_3AML069LP01G] = "PANEL_DSI_3AML069LP01G",
    [PANEL_DSI_GM8775C] = "PANEL_DSI_GM8775C",
    [PANEL_DSI_HX8394_EVB] = "PANEL_DSI_HX8394_EVB",
    [PANEL_DSI_HX8399_1080P] = "PANEL_DSI_HX8399_1080P",
    [PANEL_DSI_ICN9707] = "PANEL_DSI_ICN9707",
    [PANEL_DSI_ILI9881C] = "PANEL_DSI_ILI9881C",
    [PANEL_DSI_ILI9881D] = "PANEL_DSI_ILI9881D",
    [PANEL_DSI_JD9366AB] = "PANEL_DSI_JD9366AB",
    [PANEL_DSI_JD9852] = "PANEL_DSI_JD9852",
    [PANEL_DSI_LT9611_1024x768_60] = "PANEL_DSI_LT9611_1024x768_60",
    [PANEL_DSI_LT9611_1280x720_60] = "PANEL_DSI_LT9611_1280x720_60",
    [PANEL_DSI_LT9611_1280x1024_60] = "PANEL_DSI_LT9611_1280x1024_60",
    [PANEL_DSI_LT9611_1600x1200_60] = "PANEL_DSI_LT9611_1600x1200_60",
    [PANEL_DSI_LT9611_1920x1080_30] = "PANEL_DSI_LT9611_1920x1080_30",
    [PANEL_DSI_LT9611_1920x1080_60] = "PANEL_DSI_LT9611_1920x1080_60",
    [PANEL_DSI_NT35521] = "PANEL_DSI_NT35521",
    [PANEL_DSI_OTA7290B] = "PANEL_DSI_OTA7290B",
    [PANEL_DSI_OTA7290B_1920] = "PANEL_DSI_OTA7290B_1920",
    [PANEL_DSI_ST7701] = "PANEL_DSI_ST7701",
    [PANEL_DSI_ST7785M] = "PANEL_DSI_ST7785M",
    [PANEL_DSI_ST7796S] = "PANEL_DSI_ST7796S",
    [PANEL_BT656_TP2803] = "PANEL_BT656_TP2803"
};

const char *vo_intf_type[VO_INTF_TYPE_MAX] = {
    [VO_INTF_BT656] = "VO_INTF_BT656",
    [VO_INTF_BT1120] = "VO_INTF_BT1120",
    [VO_INTF_MIPI] = "VO_INTF_MIPI",
    [VO_INTF_HDMI] = "VO_INTF_HDMI",
    [VO_INTF_I80] = "VO_INTF_I80"
};

const char *vo_intf_sync[VO_OUTPUT_BUTT] = {
    [VO_OUTPUT_PAL] = "VO_OUTPUT_PAL",
    [VO_OUTPUT_NTSC] = "VO_OUTPUT_NTSC",
    [VO_OUTPUT_1080P24] = "VO_OUTPUT_1080P24",
    [VO_OUTPUT_1080P25] = "VO_OUTPUT_1080P25",
    [VO_OUTPUT_1080P30] = "VO_OUTPUT_1080P30",
    [VO_OUTPUT_720P50] = "VO_OUTPUT_720P50",
    [VO_OUTPUT_720P60] = "VO_OUTPUT_720P60",
    [VO_OUTPUT_1080P50] = "VO_OUTPUT_1080P50",
    [VO_OUTPUT_1080P60] = "VO_OUTPUT_1080P60",
    [VO_OUTPUT_576P50] = "VO_OUTPUT_576P50",
    [VO_OUTPUT_480P60] = "VO_OUTPUT_480P60",
    [VO_OUTPUT_800x600_60] = "VO_OUTPUT_800x600_60",
    [VO_OUTPUT_1024x768_60] = "VO_OUTPUT_1024x768_60",
    [VO_OUTPUT_1280x1024_60] = "VO_OUTPUT_1280x1024_60",
    [VO_OUTPUT_1366x768_60] = "VO_OUTPUT_1366x768_60",
    [VO_OUTPUT_1440x900_60] = "VO_OUTPUT_1440x900_60",
    [VO_OUTPUT_1280x800_60] = "VO_OUTPUT_1280x800_60",
    [VO_OUTPUT_1600x1200_60] = "VO_OUTPUT_1600x1200_60",
    [VO_OUTPUT_1680x1050_60] = "VO_OUTPUT_1680x1050_60",
    [VO_OUTPUT_1920x1200_60] = "VO_OUTPUT_1920x1200_60",
    [VO_OUTPUT_640x480_60] = "VO_OUTPUT_640x480_60",
    [VO_OUTPUT_720x1280_60] = "VO_OUTPUT_720x1280_60",
    [VO_OUTPUT_1080x1920_60] = "VO_OUTPUT_1080x1920_60",
    [VO_OUTPUT_USER] = "VO_OUTPUT_USER"
};

const char *vo_mode[VO_MODE_BUTT] = {
    [VO_MODE_1MUX] = "VO_MODE_1MUX",
    [VO_MODE_2MUX] = "VO_MODE_2MUX",
    [VO_MODE_4MUX] = "VO_MODE_4MUX",
    [VO_MODE_8MUX] = "VO_MODE_8MUX",
    [VO_MODE_9MUX] = "VO_MODE_9MUX",
    [VO_MODE_16MUX] = "VO_MODE_16MUX",
    [VO_MODE_25MUX] = "VO_MODE_25MUX",
    [VO_MODE_36MUX] = "VO_MODE_36MUX",
    [VO_MODE_49MUX] = "VO_MODE_49MUX",
    [VO_MODE_64MUX] = "VO_MODE_64MUX",
    [VO_MODE_2X4] = "VO_MODE_2X4"
};

const char *vo_chn_mirror_type[VO_CHN_MIRROR_BUTT] = {
    [VO_CHN_MIRROR_NONE] = "VO_CHN_MIRROR_NONE",
    [VO_CHN_MIRROR_HOR] = "VO_CHN_MIRROR_HOR",
    [VO_CHN_MIRROR_VER] = "VO_CHN_MIRROR_VER",
    [VO_CHN_MIRROR_BOTH] = "VO_CHN_MIRROR_BOTH"
};

const char *vo_chn_zoom_type[VO_CHN_ZOOM_IN_BUTT] = {
    [VO_CHN_ZOOM_IN_RECT] = "VO_CHN_ZOOM_IN_RECT",
    [VO_CHN_ZOOM_IN_RATIO] = "VO_CHN_ZOOM_IN_RATIO"
};

int Load_Param_Display(const char * file)
{
    int ret = 0;
    int enum_num = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_MULTI_DISPLAY_CFG_T * pstDisplayCfg = app_ipcam_Display_Param_Get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** mode_id = app_ipcam_Param_get_mode_id();
    const char ** aspect_ratio = app_ipcam_Param_get_aspect_ratio();
    const char ** rotation = app_ipcam_Param_get_rotation();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading display config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    pstDisplayCfg->vo_num = ini_getl("display_config", "vo_cnt", 0, file);

    for(unsigned i = 0; i < pstDisplayCfg->vo_num; i++){
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "display_config_%d", i);

        ini_gets(tmp_section, "panel_name", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, panel_name, PANEL_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][panel_name] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][panel_name] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].enPanelName = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.s32VoDev = ini_getl(tmp_section, "vo_dev", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.u32BgColor = ini_getl(tmp_section, "bg_color", 0, file);

        ini_gets(tmp_section, "intf_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_type, VO_INTF_TYPE_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Fail to convert string name [%s] to number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Convert string name [%s] to number [%d].\n", tmp_section
                , str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfType = enum_num;
        }

        ini_gets(tmp_section, "intf_sync", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_sync, VO_OUTPUT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfSync = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32X = ini_getl(tmp_section, "dis_x", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32Y = ini_getl(tmp_section, "dis_y", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Width = ini_getl(tmp_section, "dis_width", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Height = ini_getl(tmp_section, "dis_height", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Width = ini_getl(tmp_section, "img_width", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Height = ini_getl(tmp_section, "img_height", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.u32DispFrmRt = ini_getl(tmp_section, "dis_framerate", 0, file);

        ini_gets(tmp_section, "pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.enPixFormat = enum_num;
        }

        ini_gets(tmp_section, "mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_mode, VO_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Convert string name [%s] to enum number [%d].\n", tmp_section
                , str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.enVoMode = enum_num;
        }

        ini_gets(tmp_section, "rotation", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rotation, ROTATION_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.enRotation = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.u32DisBufLen = ini_getl(tmp_section, "dis_buf_len", 0, file);

        pstDisplayCfg->vo_cfg[i].stVoCfg.bBindMode = ini_getl(tmp_section, "bind_mode", 0, file);
        if (pstDisplayCfg->vo_cfg[i].stVoCfg.bBindMode) {
            ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n"
                    , tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n"
                    , tmp_section, str_name, enum_num);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.enModId = enum_num;
            }

            pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);
        }
        ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.enModId = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);

        ini_gets(tmp_section, "aspectratio", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, aspect_ratio, ASPECT_RATIO_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][aspectratio] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][aspectratio] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.enMode = enum_num;
        }

        if (pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.enMode == ASPECT_RATIO_MANUAL) { /*ASPECT_RATIO_MANUAL*/
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.bEnableBgColor = ini_getl(tmp_section, "en_color", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.u32BgColor = ini_getl(tmp_section, "color", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.s32X = ini_getl(tmp_section, "s32x", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.s32Y = ini_getl(tmp_section, "s32y", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.u32Width = ini_getl(tmp_section, "rec_width", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.u32Height = ini_getl(tmp_section, "rec_heigh", 0, file);
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.enable = ini_getl(tmp_section, "border_en", 0, file);
        if (pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.enable != 0) {
            pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32TopWidth = ini_getl(tmp_section, "top_width", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32BottomWidth = ini_getl(tmp_section, "bottom_width", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32LeftWidth = ini_getl(tmp_section, "left_width", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32RightWidth = ini_getl(tmp_section, "right_width", 0, file);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32Color = ini_getl(tmp_section, "border_color", 0, file);
        }

        ini_gets(tmp_section, "mirror_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_chn_mirror_type, VO_CHN_MIRROR_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mirror_type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mirror_type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.enMirrorType = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.bZoom = ini_getl(tmp_section, "zoom_en", 0, file);
        if (pstDisplayCfg->vo_cfg[i].stVoCfg.bZoom != 0) {
            ini_gets(tmp_section, "zoom_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_chn_zoom_type, VO_CHN_MIRROR_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][zoom_type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][zoom_type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.enZoomType = enum_num;
            }

            if (pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.enZoomType == VO_CHN_ZOOM_IN_RATIO){
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stZoomRatio.u32Xratio = ini_getl(tmp_section, "x_ration", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stZoomRatio.u32Yratio = ini_getl(tmp_section, "y_ration", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stZoomRatio.u32WidthRatio = ini_getl(tmp_section, "width_ration", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stZoomRatio.u32HeightRatio = ini_getl(tmp_section, "height_ration", 0, file);
            } else if (pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.enZoomType == VO_CHN_ZOOM_IN_RECT){
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stRect.s32X = ini_getl(tmp_section, "zoom_x", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stRect.s32Y = ini_getl(tmp_section, "zoom_y", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stRect.u32Width = ini_getl(tmp_section, "zoom_width", 0, file);
                pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.stRect.u32Height = ini_getl(tmp_section, "zoom_height", 0, file);
            }
        }

        APP_PROF_LOG_PRINT(LEVEL_INFO, "enPanelName:%d, s32VoDev:%d, u32BgColor:%d, enIntfType:%d, enIntfSync:%d, "
            "x:%d, y:%d, w:%d, h:%d, imgw:%d, imgh:%d, "
            "u32DispFrmRt:%d, enPixFormat:%d, enVoMode:%d, enRotation:%d, u32DisBufLen:%d, "
            "bBindMode:%d, src: modid=%d, devid=%d, chnid=%d, dst: modid=%d, devid=%d, chnid=%d.\n"
            , pstDisplayCfg->vo_cfg[i].enPanelName
            , pstDisplayCfg->vo_cfg[i].stVoCfg.s32VoDev
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.u32BgColor
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfType
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfSync
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32X
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32Y
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Width
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Height
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Width
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Height
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.u32DispFrmRt
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.enPixFormat
            , pstDisplayCfg->vo_cfg[i].stVoCfg.enVoMode
            , pstDisplayCfg->vo_cfg[i].stVoCfg.enRotation
            , pstDisplayCfg->vo_cfg[i].stVoCfg.u32DisBufLen
            , pstDisplayCfg->vo_cfg[i].stVoCfg.bBindMode
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.enModId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32DevId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32ChnId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.enModId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32DevId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32ChnId);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "AspectRatioMode:%d, bEnableBgColor:%d, u32BgColor:%d, "
            "s32X:%d, s32Y:%d, u32Width:%d, u32Height:%d \n"
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.enMode
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.bEnableBgColor
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.u32BgColor
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.s32X
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.s32Y
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.u32Width
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stChnParam.stAspectRatio.stVideoRect.u32Height);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "BorderEnable:%d, u32Color:%d, "
            "u32TopWidth:%d, u32BottomWidth:%d, u32LeftWidth:%d, u32RightWidth:%d \n"
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.enable
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32Color
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32TopWidth
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32BottomWidth
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32LeftWidth
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stBorderAttr.stBorder.u32RightWidth);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "ZoomEnable:%d enZoomType:%d enMirrorType:%d \n"
            , pstDisplayCfg->vo_cfg[i].stVoCfg.bZoom
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stZoomAttr.enZoomType
            , pstDisplayCfg->vo_cfg[i].stVoCfg.enMirrorType);

    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading display config ------------------> done \n\n");

    return CVI_SUCCESS;
}
