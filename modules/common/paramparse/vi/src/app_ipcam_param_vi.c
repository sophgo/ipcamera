#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "sensor_list.h"
#include "app_ipcam_paramparse.h"

const char *wdr_mode[WDR_MODE_MAX] = {
    [WDR_MODE_NONE] = "WDR_MODE_NONE",
    [WDR_MODE_BUILT_IN] = "WDR_MODE_BUILT_IN",
    [WDR_MODE_QUDRA] = "WDR_MODE_QUDRA",
    [WDR_MODE_2To1_LINE] = "WDR_MODE_2To1_LINE",
    [WDR_MODE_2To1_FRAME] = "WDR_MODE_2To1_FRAME",
    [WDR_MODE_2To1_FRAME_FULL_RATE] = "WDR_MODE_2To1_FRAME_FULL_RATE",
    [WDR_MODE_3To1_LINE] = "WDR_MODE_3To1_LINE",
    [WDR_MODE_3To1_FRAME] = "WDR_MODE_3To1_FRAME",
    [WDR_MODE_3To1_FRAME_FULL_RATE] = "WDR_MODE_3To1_FRAME_FULL_RATE",
    [WDR_MODE_4To1_LINE] = "WDR_MODE_4To1_LINE",
    [WDR_MODE_4To1_FRAME] = "WDR_MODE_4To1_FRAME",
    [WDR_MODE_4To1_FRAME_FULL_RATE] = "WDR_MODE_4To1_FRAME_FULL_RATE"
};

const char *dynamic_range[DYNAMIC_RANGE_MAX] = {
    [DYNAMIC_RANGE_SDR8] = "DYNAMIC_RANGE_SDR8",
    [DYNAMIC_RANGE_SDR10] = "DYNAMIC_RANGE_SDR10",
    [DYNAMIC_RANGE_HDR10] = "DYNAMIC_RANGE_HDR10",
    [DYNAMIC_RANGE_HLG] = "DYNAMIC_RANGE_HLG",
    [DYNAMIC_RANGE_SLF] = "DYNAMIC_RANGE_SLF",
    [DYNAMIC_RANGE_XDR] = "DYNAMIC_RANGE_XDR"
};


int Load_Param_Vi(const char *file)
{
    int i = 0;
    int enum_num = 0;
    int ret = 0;
    long int work_sns_cnt = 0;
    char tmp[16] = {0};
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_VI_CTX_S *pViIniCfg = app_ipcam_Vi_Param_Get();
    const char ** vi_vpss_mode = app_ipcam_Param_get_vi_vpss_mode();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** compress_mode = app_ipcam_Param_get_compress_mode();
    const char ** video_format = app_ipcam_Param_get_video_format();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vi config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "vi_config");
    work_sns_cnt = ini_getl(tmp_section, "sensor_cnt", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "work_sns_cnt = %ld\n", work_sns_cnt);
    if(work_sns_cnt <= (long int)(sizeof(pViIniCfg->astSensorCfg) / sizeof(pViIniCfg->astSensorCfg[0]))) {
        pViIniCfg->u32WorkSnsCnt = work_sns_cnt;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "work_sns_cnt error (%ld)\n", work_sns_cnt);
        return CVI_FAILURE;
    }

    // APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor cfg path = %s\n", DEF_INI_PATH);
    for (i = 0; i< (int)pViIniCfg->u32WorkSnsCnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "sensor_config%d", i);

        pViIniCfg->astSensorCfg[i].s32SnsId = i;
        pViIniCfg->astSensorCfg[i].s32ModeId = ini_getl(tmp_section, "en_mode", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d en_mode=%d\n", i, pViIniCfg->astSensorCfg[i].s32ModeId);
        /* framerate here is right ?? */
        pViIniCfg->astSensorCfg[i].s32Framerate = ini_getl(tmp_section, "framerate", 0, file);
        /* used sensor name instead of enum ?? */
        ini_gets(tmp_section, "sns_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, snsr_type_name, SAMPLE_SNS_TYPE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][sns_type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][sns_type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astSensorCfg[i].enSnsType = enum_num;
        }

        pViIniCfg->astSensorCfg[i].MipiDev = ini_getl(tmp_section, "mipi_dev", 0, file);
        pViIniCfg->astSensorCfg[i].s32BusId = ini_getl(tmp_section, "bus_id", 0, file);
        pViIniCfg->astSensorCfg[i].s32I2cAddr = ini_getl(tmp_section, "sns_i2c_addr", -1, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d enSnsType=%d MipiDev=%d s32BusId=%d s32I2cAddr=0x%x\n", i, pViIniCfg->astSensorCfg[i].enSnsType,
            pViIniCfg->astSensorCfg[i].MipiDev, pViIniCfg->astSensorCfg[i].s32BusId, pViIniCfg->astSensorCfg[i].s32I2cAddr);
        for (int j = 0; j < MIPI_RX_MAX_LANE_NUM; j++) {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "laneid%d", j);
            pViIniCfg->astSensorCfg[i].as16LaneId[j] = ini_getl(tmp_section, tmp, 0, file);

            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "swap%d", j);
            pViIniCfg->astSensorCfg[i].as8PNSwap[j] = ini_getl(tmp_section, tmp, 0, file);
        }
        pViIniCfg->astSensorCfg[i].bMclkEn = ini_getl(tmp_section, "mclk_en", 1, file);
        pViIniCfg->astSensorCfg[i].u8Mclk = ini_getl(tmp_section, "mclk", 0, file);
        pViIniCfg->astSensorCfg[i].u8Orien = ini_getl(tmp_section, "orien", 0, file);
        pViIniCfg->astSensorCfg[i].bHwSync = ini_getl(tmp_section, "hw_sync", 0, file);
        pViIniCfg->astSensorCfg[i].u8UseDualSns = ini_getl(tmp_section, "use_dual_Sns", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d bMclkEn=%d u8Mclk=%d u8Orien=%d bHwSync=%d\n", i, pViIniCfg->astSensorCfg[i].bMclkEn,
            pViIniCfg->astSensorCfg[i].u8Mclk, pViIniCfg->astSensorCfg[i].u8Orien, pViIniCfg->astSensorCfg[i].bHwSync);

        pViIniCfg->astSensorCfg[i].bHsettlen = ini_getl(tmp_section, "hs_settle_en", 0, file);
        pViIniCfg->astSensorCfg[i].u8Hsettle = ini_getl(tmp_section, "hs_settle", 0, file);
        pViIniCfg->astSensorCfg[i].s32RstPin = ini_getl(tmp_section, "rst_pin", 0, file);
        pViIniCfg->astSensorCfg[i].s32RstActive = ini_getl(tmp_section, "rst_active", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d hs_settle_en=%d hs_settle=%d RstPin=%d RstActive=%d\n", i, pViIniCfg->astSensorCfg[i].bHsettlen,
            pViIniCfg->astSensorCfg[i].u8Hsettle, pViIniCfg->astSensorCfg[i].s32RstPin, pViIniCfg->astSensorCfg[i].s32RstActive);
    }

    for (i = 0; i< (int)pViIniCfg->u32WorkSnsCnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_dev%d", i);
        pViIniCfg->astDevInfo[i].ViDev = ini_getl(tmp_section, "videv", 0, file);

        ini_gets(tmp_section, "wdrmode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, wdr_mode, WDR_MODE_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][wdrmode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][wdrmode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astDevInfo[i].enWDRMode = enum_num;
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_pipe%d", i);
        for (int j = 0; j< WDR_MAX_PIPE_NUM; j++) {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "apipe%d", j);
            pViIniCfg->astPipeInfo[i].aPipe[j] = ini_getl(tmp_section, tmp, 0, file);
        }

        ini_gets(tmp_section, "pipe_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vi_vpss_mode, VI_VPSS_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pipe_mode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pipe_mode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astPipeInfo[i].enMastPipeMode = enum_num;
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_chn%d", i);
        pViIniCfg->astChnInfo[i].s32ChnId = i;
        pViIniCfg->astChnInfo[i].u32Width = ini_getl(tmp_section, "width", 0, file);
        pViIniCfg->astChnInfo[i].u32Height = ini_getl(tmp_section, "height", 0, file);
        pViIniCfg->astChnInfo[i].f32Fps = ini_getl(tmp_section, "fps", 0, file);

        ini_gets(tmp_section, "pixFormat", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixFormat] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixFormat] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astChnInfo[i].enPixFormat = enum_num;
        }

        ini_gets(tmp_section, "dynamic_range", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, dynamic_range, DYNAMIC_RANGE_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dynamic_range] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dynamic_range] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astChnInfo[i].enDynamicRange = enum_num;
        }

        ini_gets(tmp_section, "video_format", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, video_format, VIDEO_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][video_format] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][video_format] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astChnInfo[i].enVideoFormat = enum_num;
        }

        ini_gets(tmp_section, "compress_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, compress_mode, COMPRESS_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][compress_mode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][compress_mode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pViIniCfg->astChnInfo[i].enCompressMode = enum_num;
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_isp%d", i);
        pViIniCfg->astIspCfg[i].bAfFliter = ini_getl(tmp_section, "af_filter", 0, file);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vi config ------------------> done \n\n");

    return CVI_SUCCESS;
}
