#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

#define VO_INTF_TYPE_MAX ((0x01L << 16) + 1)

const char *vo_intf_type[VO_INTF_TYPE_MAX] = {
    [VO_INTF_CVBS]       = "VO_INTF_CVBS",
    [VO_INTF_YPBPR]      = "VO_INTF_YPBPR",
    [VO_INTF_VGA]        = "VO_INTF_VGA",
    [VO_INTF_BT656]      = "VO_INTF_BT656",
    [VO_INTF_BT1120]     = "VO_INTF_BT1120",
    [VO_INTF_LCD]        = "VO_INTF_LCD",
    [VO_INTF_LCD_18BIT]  = "VO_INTF_LCD_18BIT",
    [VO_INTF_LCD_24BIT]  = "VO_INTF_LCD_24BIT",
    [VO_INTF_LCD_30BIT]  = "VO_INTF_LCD_30BIT",
    [VO_INTF_MIPI]       = "VO_INTF_MIPI",
    [VO_INTF_MIPI_SLAVE] = "VO_INTF_MIPI_SLAVE",
    [VO_INTF_HDMI]       = "VO_INTF_HDMI",
    [VO_INTF_I80]        = "VO_INTF_I80"
};

const char *vo_intf_sync[VO_OUTPUT_BUTT] = {
    [VO_OUTPUT_PAL]           = "VO_OUTPUT_PAL",
    [VO_OUTPUT_NTSC]          = "VO_OUTPUT_NTSC",
    [VO_OUTPUT_1080P24]       = "VO_OUTPUT_1080P24",
    [VO_OUTPUT_1080P25]       = "VO_OUTPUT_1080P25",
    [VO_OUTPUT_1080P30]       = "VO_OUTPUT_1080P30",
    [VO_OUTPUT_720P50]        = "VO_OUTPUT_720P50",
    [VO_OUTPUT_720P60]        = "VO_OUTPUT_720P60",
    [VO_OUTPUT_1080P50]       = "VO_OUTPUT_1080P50",
    [VO_OUTPUT_1080P60]       = "VO_OUTPUT_1080P60",
    [VO_OUTPUT_576P50]        = "VO_OUTPUT_576P50",
    [VO_OUTPUT_480P60]        = "VO_OUTPUT_480P60",
    [VO_OUTPUT_800x600_60]    = "VO_OUTPUT_800x600_60",
    [VO_OUTPUT_1024x768_60]   = "VO_OUTPUT_1024x768_60",
    [VO_OUTPUT_1280x1024_60]  = "VO_OUTPUT_1280x1024_60",
    [VO_OUTPUT_1366x768_60]   = "VO_OUTPUT_1366x768_60",
    [VO_OUTPUT_1440x900_60]   = "VO_OUTPUT_1440x900_60",
    [VO_OUTPUT_1280x800_60]   = "VO_OUTPUT_1280x800_60",
    [VO_OUTPUT_1600x1200_60]  = "VO_OUTPUT_1600x1200_60",
    [VO_OUTPUT_1680x1050_60]  = "VO_OUTPUT_1680x1050_60",
    [VO_OUTPUT_1920x1200_60]  = "VO_OUTPUT_1920x1200_60",
    [VO_OUTPUT_640x480_60]    = "VO_OUTPUT_640x480_60",
    [VO_OUTPUT_720x1280_60]   = "VO_OUTPUT_720x1280_60",
    [VO_OUTPUT_1080x1920_60]  = "VO_OUTPUT_1080x1920_60",
    [VO_OUTPUT_USER]          = "VO_OUTPUT_USER"
};

const char *rotation[ROTATION_MAX] = {
    [ROTATION_0]       = "ROTATION_0",
    [ROTATION_90]      ="ROTATION_90",
    [ROTATION_180]     = "ROTATION_180",
    [ROTATION_270]     = "ROTATION_270",
    [ROTATION_XY_FLIP] = "ROTATION_XY_FLIP"
};

const char *vo_mode[VO_MODE_BUTT] = {
    [VO_MODE_1MUX]  = "VO_MODE_1MUX",
    [VO_MODE_2MUX]  = "VO_MODE_2MUX",
    [VO_MODE_4MUX]  = "VO_MODE_4MUX",
    [VO_MODE_8MUX]  = "VO_MODE_8MUX",
    [VO_MODE_9MUX]  = "VO_MODE_9MUX",
    [VO_MODE_16MUX] = "VO_MODE_16MUX",
    [VO_MODE_25MUX] = "VO_MODE_25MUX",
    [VO_MODE_36MUX] = "VO_MODE_36MUX",
    [VO_MODE_49MUX] = "VO_MODE_49MUX",
    [VO_MODE_64MUX] = "VO_MODE_64MUX",
    [VO_MODE_2X4]   = "VO_MODE_2X4"
};

int Load_Param_Vo(const char *file)
{
    int ret = 0;
    int enum_num = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_MULTI_VO_PARAM_S *pst_vo_cfg = app_ipcam_vo_param_get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** mode_id = app_ipcam_Param_get_mode_id();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vo config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    sprintf(tmp_section, "vo_config");
    pst_vo_cfg->vo_num = ini_getl(tmp_section, "vo_cnt", 0, file);
    printf("vo_num=%d\n", pst_vo_cfg->vo_num);

    for(unsigned i = 0; i < pst_vo_cfg->vo_num; i++){
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "vo_config_%d", i);

        pst_vo_cfg->vo_cfg[i].s32VoDev = ini_getl(tmp_section, "vo_dev", 0, file);
        pst_vo_cfg->vo_cfg[i].stVoPubAttr.u32BgColor = ini_getl(tmp_section, "bg_color", 0, file);

        ini_gets(tmp_section, "intf_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_type, VO_INTF_TYPE_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Fail to convert string name [%s] to number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Convert string name [%s] to number [%d].\n", tmp_section
                , str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].stVoPubAttr.enIntfType = enum_num;
        }

        ini_gets(tmp_section, "intf_sync", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_sync, VO_OUTPUT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].stVoPubAttr.enIntfSync = enum_num;
        }

        pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.s32X = ini_getl(tmp_section, "dis_x", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.s32Y = ini_getl(tmp_section, "dis_y", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.u32Width = ini_getl(tmp_section, "dis_width", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.u32Height = ini_getl(tmp_section, "dis_height", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.stImageSize.u32Width = ini_getl(tmp_section, "img_width", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.stImageSize.u32Height = ini_getl(tmp_section, "img_height", 0, file);
        pst_vo_cfg->vo_cfg[i].stLayerAttr.u32DispFrmRt = ini_getl(tmp_section, "dis_framerate", 0, file);

        ini_gets(tmp_section, "pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].stLayerAttr.enPixFormat = enum_num;
        }

        ini_gets(tmp_section, "mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_mode, VO_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Convert string name [%s] to enum number [%d].\n", tmp_section
                , str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].enVoMode = enum_num;
        }

        ini_gets(tmp_section, "rotation", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rotation, ROTATION_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].enRotation = enum_num;
        }

        pst_vo_cfg->vo_cfg[i].u32DisBufLen = ini_getl(tmp_section, "dis_buf_len", 0, file);

        ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].stSrcChn.enModId = enum_num;
        }

        pst_vo_cfg->vo_cfg[i].stSrcChn.s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
        pst_vo_cfg->vo_cfg[i].stSrcChn.s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);

        ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pst_vo_cfg->vo_cfg[i].stDstChn.enModId = enum_num;
        }

        pst_vo_cfg->vo_cfg[i].stDstChn.s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
        pst_vo_cfg->vo_cfg[i].stDstChn.s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "s32VoDev:%d, u32BgColor:%d, enIntfType:%d, enIntfSync:%d, \n"
            "x:%d, y:%d, w:%d, h:%d, imgw:%d, imgh:%d, u32DispFrmRt:%d, enPixFormat:%d, enVoMode:%d, enRotation:%d,\n"
            "u32DisBufLen:%d, src: modid=%d, devid=%d, chnid=%d, dst: modid=%d, devid=%d, chnid=%d.\n"
            , pst_vo_cfg->vo_cfg[i].s32VoDev
            , pst_vo_cfg->vo_cfg[i].stVoPubAttr.u32BgColor
            , pst_vo_cfg->vo_cfg[i].stVoPubAttr.enIntfType
            , pst_vo_cfg->vo_cfg[i].stVoPubAttr.enIntfSync
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.s32X
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.s32Y
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.u32Width
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stDispRect.u32Height
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stImageSize.u32Width
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.stImageSize.u32Height
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.u32DispFrmRt
            , pst_vo_cfg->vo_cfg[i].stLayerAttr.enPixFormat
            , pst_vo_cfg->vo_cfg[i].enVoMode
            , pst_vo_cfg->vo_cfg[i].enRotation
            , pst_vo_cfg->vo_cfg[i].u32DisBufLen
            , pst_vo_cfg->vo_cfg[i].stSrcChn.enModId
            , pst_vo_cfg->vo_cfg[i].stSrcChn.s32DevId
            , pst_vo_cfg->vo_cfg[i].stSrcChn.s32ChnId
            , pst_vo_cfg->vo_cfg[i].stDstChn.enModId
            , pst_vo_cfg->vo_cfg[i].stDstChn.s32DevId
            , pst_vo_cfg->vo_cfg[i].stDstChn.s32ChnId);
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vo config ------------------> done \n\n");

    return CVI_SUCCESS;
}