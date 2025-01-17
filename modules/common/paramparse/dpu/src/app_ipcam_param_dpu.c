#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "cvi_math.h"
#include "app_ipcam_paramparse.h"

const char *DpuMode[DPU_MODE_BUTT] = {
    [DPU_MODE_DEFAULT] = "DPU_MODE_DEFAULT",
    [DPU_MODE_SGBM_MUX0] = "DPU_MODE_SGBM_MUX0",
    [DPU_MODE_SGBM_MUX1] = "DPU_MODE_SGBM_MUX1",
    [DPU_MODE_SGBM_MUX2] = "DPU_MODE_SGBM_MUX2",
    [DPU_MODE_SGBM_FGS_ONLINE_MUX0] = "DPU_MODE_SGBM_FGS_ONLINE_MUX0",
    [DPU_MODE_SGBM_FGS_ONLINE_MUX1] = "DPU_MODE_SGBM_FGS_ONLINE_MUX1",
    [DPU_MODE_SGBM_FGS_ONLINE_MUX2] = "DPU_MODE_SGBM_FGS_ONLINE_MUX2",
    [DPU_MODE_FGS_MUX0] = "DPU_MODE_SGBM_FGS_ONLINE_MUX0",
    [DPU_MODE_FGS_MUX1] = "DPU_MODE_SGBM_FGS_ONLINE_MUX0",
};

const char *MaskMode[DPU_MASK_MODE_BUTT] = {
    [DPU_MASK_MODE_DEFAULT] = "DPU_MASK_MODE_DEFAULT",
    [DPU_MASK_MODE_1x1] = "DPU_MASK_MODE_1x1",
    [DPU_MASK_MODE_3x3] = "DPU_MASK_MODE_3x3",
    [DPU_MASK_MODE_5x5] = "DPU_MASK_MODE_5x5",
    [DPU_MASK_MODE_7x7] = "DPU_MASK_MODE_7x7",
};

const char *DispRange[DPU_DISP_RANGE_BUTT] = {
    [DPU_DISP_RANGE_DEFAULT] = "DPU_DISP_RANGE_DEFAULT",
    [DPU_DISP_RANGE_16] = "DPU_DISP_RANGE_16",
    [DPU_DISP_RANGE_32] = "DPU_DISP_RANGE_32",
    [DPU_DISP_RANGE_48] = "DPU_DISP_RANGE_48",
    [DPU_DISP_RANGE_64] = "DPU_DISP_RANGE_64",
    [DPU_DISP_RANGE_80] = "DPU_DISP_RANGE_80",
    [DPU_DISP_RANGE_96] = "DPU_DISP_RANGE_96",
    [DPU_DISP_RANGE_112] = "DPU_DISP_RANGE_112",
    [DPU_DISP_RANGE_128] = "DPU_DISP_RANGE_128",
};

const char *DccDir[DPU_DCC_DIR_BUTT] = {
    [DPU_DCC_DIR_DEFAULT] = "DPU_DCC_DIR_DEFAULT",
    [DPU_DCC_DIR_A12] = "DPU_DCC_DIR_A12",
    [DPU_DCC_DIR_A13] = "DPU_DCC_DIR_A13",
    [DPU_DCC_DIR_A14] = "DPU_DCC_DIR_A14",
};

const char *DepthUnit[DPU_DEPTH_UNIT_BUTT] = {
    [DPU_DEPTH_UNIT_DEFAULT] = "DPU_DEPTH_UNIT_DEFAULT",
    [DPU_DEPTH_UNIT_MM] = "DPU_DEPTH_UNIT_MM",
    [DPU_DEPTH_UNIT_CM] = "DPU_DEPTH_UNIT_CM",
    [DPU_DEPTH_UNIT_DM] = "DPU_DEPTH_UNIT_DM",
    [DPU_DEPTH_UNIT_M] = "DPU_DEPTH_UNIT_M",
};

int Load_Param_Dpu(const char *file)
{
    CVI_U32 grp_idx = 0;
    CVI_S32 enum_num = 0;
    CVI_S32 ret = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_DPU_CFG_S *Dpu = app_ipcam_Dpu_Param_Get();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Dpu config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "dpu_config");

    Dpu->u32GrpCnt = ini_getl(tmp_section, "dpu_grp", 0, file);

    for (grp_idx = 0; grp_idx < Dpu->u32GrpCnt; grp_idx++)
    {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "dpugrp%d", grp_idx);

        Dpu->stDpuGrpCfg[grp_idx].bEnable = ini_getl(tmp_section, "dpu_bEnable", 0, file);
        if (!Dpu->stDpuGrpCfg[grp_idx].bEnable)
            continue;

        Dpu->stDpuGrpCfg[grp_idx].DpuGrp = grp_idx;
        Dpu->stDpuGrpCfg[grp_idx].SendTo = ini_getl(tmp_section, "dpu_send_to", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].VencId = ini_getl(tmp_section, "dpu_venc_id", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].bColorMap = ini_getl(tmp_section, "dpu_bColorMap", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].VpssGrpL = ini_getl(tmp_section, "dpu_vpss_grp_l", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].VpssChnL = ini_getl(tmp_section, "dpu_vpss_chn_l", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].VpssGrpR = ini_getl(tmp_section, "dpu_vpss_grp_r", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].VpssChnR = ini_getl(tmp_section, "dpu_vpss_chn_r", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stLeftImageSize.u32Width = ini_getl(tmp_section, "dpu_width", 448, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stLeftImageSize.u32Height  = ini_getl(tmp_section, "dpu_height", 368, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stRightImageSize.u32Width = ini_getl(tmp_section, "dpu_width", 448, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stRightImageSize.u32Height  = ini_getl(tmp_section, "dpu_height", 368, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u16DispStartPos  = ini_getl(tmp_section, "dpu_DispStartPos", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32Rshift1  = ini_getl(tmp_section, "dpu_Rshift1", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32Rshift2  = ini_getl(tmp_section, "dpu_Rshift2", 2, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32CaP1  = ini_getl(tmp_section, "dpu_CaP1", 1800, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32CaP2  = ini_getl(tmp_section, "dpu_CaP2", 14400, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32UniqRatio  = ini_getl(tmp_section, "dpu_UniqRatio", 25, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32DispShift  = ini_getl(tmp_section, "dpu_DispShift", 4, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32CensusShift  = ini_getl(tmp_section, "dpu_CensusShift", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32FxBaseline  = ini_getl(tmp_section, "dpu_FxBaseline", 864000, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32FgsMaxCount  = ini_getl(tmp_section, "dpu_FgsMaxCount", 19, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.u32FgsMaxT  = ini_getl(tmp_section, "dpu_FgsMaxT", 3, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.bIsBtcostOut  = ini_getl(tmp_section, "dpu_bIsBtcostOut", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.bNeedSrcFrame  = ini_getl(tmp_section, "dpu_bNeedSrcFrame", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stFrameRate.s32SrcFrameRate  = ini_getl(tmp_section, "dpu_SrcFrameRate", -1, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.stFrameRate.s32DstFrameRate  = ini_getl(tmp_section, "dpu_DstFrameRate", -1, file);
        Dpu->stDpuGrpCfg[grp_idx].bGdcGrid  = ini_getl(tmp_section, "dpu_gdc_en", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].GdcCfgL  = ini_getl(tmp_section, "dpu_gdc_config_l", 0, file);
        Dpu->stDpuGrpCfg[grp_idx].GdcCfgR  = ini_getl(tmp_section, "dpu_gdc_config_r", 1, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuChnAttr.stImgSize.u32Width = ini_getl(tmp_section, "dpu_chn_width", 448, file);
        Dpu->stDpuGrpCfg[grp_idx].stDpuChnAttr.stImgSize.u32Height = ini_getl(tmp_section, "dpu_chn_height", 368, file);

        ini_gets(tmp_section, "dpu_enDpuMode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, DpuMode, DPU_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDpuMode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDpuMode = DPU_MODE_SGBM_MUX0;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDpuMode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDpuMode = enum_num;
        }

        ini_gets(tmp_section, "dpu_enMaskMode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, MaskMode, DPU_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enMaskMode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enMaskMode = DPU_MASK_MODE_7x7;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enMaskMode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enMaskMode = enum_num;
        }

        ini_gets(tmp_section, "dpu_enDispRange", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, DispRange, DPU_DISP_RANGE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDispRange] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDispRange = DPU_DISP_RANGE_64;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDispRange] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDispRange = enum_num;
        }

        ini_gets(tmp_section, "dpu_enDccDir", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, DccDir, DPU_DCC_DIR_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDccDir] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDccDir = DPU_DCC_DIR_A14;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_enDccDir] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDccDir = enum_num;
        }

        ini_gets(tmp_section, "dpu_DpuDepthUnit", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, DepthUnit, DPU_DEPTH_UNIT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_DpuDepthUnit] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dpu_DpuDepthUnit] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Dpu->stDpuGrpCfg[grp_idx].stDpuGrpAttr.enDpuDepthUnit = enum_num;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Dpu config ------------------> done \n\n");
    return CVI_SUCCESS;
}