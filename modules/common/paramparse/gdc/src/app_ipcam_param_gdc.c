#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"



int Load_Param_Gdc(const char *file)
{
    CVI_U32 cfg_idx = 0;
    CVI_S32 ret = 0;
    CVI_S32 enum_num = 0;
    char tmp_section[64] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_GDC_CFG_T *Gdc = app_ipcam_Gdc_Param_Get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** rotation = app_ipcam_Param_get_rotation();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading gdc config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "gdc_config");
    Gdc->u32CfgCnt = ini_getl(tmp_section, "config_cnt", 0, file);
    Gdc->bUserEnable = ini_getl(tmp_section, "user_enable", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config:config_cnt: %d\n", Gdc->u32CfgCnt);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config:user_enable: %d\n", Gdc->bUserEnable);
    for (cfg_idx = 0; cfg_idx < Gdc->u32CfgCnt; cfg_idx++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d", cfg_idx);
        APP_GDC_CFG_T *pstGdcCfg = &Gdc->astGdcCfg[cfg_idx];
        pstGdcCfg->bEnable = ini_getl(tmp_section, "enable", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:enable: %d\n", cfg_idx, pstGdcCfg->bEnable);
        if (pstGdcCfg->bEnable == 0) {
            continue;
        }
        pstGdcCfg->hHandle      = ini_getl(tmp_section, "hHandle", 0, file);
        pstGdcCfg->u32Operation = ini_getl(tmp_section, "operation", 0, file);
        ini_gets(tmp_section, "filename_in", " ", str_name, PARAM_STRING_NAME_LEN, file);
        strncpy(pstGdcCfg->filename_in, str_name, PARAM_STRING_NAME_LEN);
        ini_gets(tmp_section, "filename_out", " ", str_name, PARAM_STRING_NAME_LEN, file);
        strncpy(pstGdcCfg->filename_out, str_name, PARAM_STRING_NAME_LEN);
        pstGdcCfg->size_in.u32Width   = ini_getl(tmp_section, "width_in", 0, file);
        pstGdcCfg->size_in.u32Height  = ini_getl(tmp_section, "height_in", 0, file);
        pstGdcCfg->size_out.u32Width  = ini_getl(tmp_section, "width_out", 0, file);
        pstGdcCfg->size_out.u32Height = ini_getl(tmp_section, "height_out", 0, file);

        ini_gets(tmp_section, "enPixelFormat", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstGdcCfg->enPixelFormat = enum_num;
        }

        ini_gets(tmp_section, "enRotation", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rotation, ROTATION_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstGdcCfg->enRotation = enum_num;
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_identity", cfg_idx);
        ini_gets(tmp_section, "Name", " ", str_name, PARAM_STRING_NAME_LEN, file);
        strncpy(pstGdcCfg->identity.Name, str_name, PARAM_STRING_NAME_LEN);
        pstGdcCfg->identity.enModId  = ini_getl(tmp_section, "enModId", 0, file);
        pstGdcCfg->identity.u32ID    = ini_getl(tmp_section, "u32ID", 0, file);
        pstGdcCfg->identity.syncIo   = ini_getl(tmp_section, "syncIo", 0, file);
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_ldc", cfg_idx);
        pstGdcCfg->LdcAttr.bEnable = ini_getl(tmp_section, "ldc_enable", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:hHandle: %ld\n", cfg_idx, pstGdcCfg->hHandle);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:operation: %d\n", cfg_idx, pstGdcCfg->u32Operation);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:filename_in: %s\n", cfg_idx, pstGdcCfg->filename_in);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:filename_out: %s\n", cfg_idx, pstGdcCfg->filename_out);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:width_in: %d, height_in: %d, width_out: %d, height_out: %d\n", cfg_idx,
                            pstGdcCfg->size_in.u32Width, pstGdcCfg->size_in.u32Height, pstGdcCfg->size_out.u32Width, pstGdcCfg->size_out.u32Height);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:enPixelFormat: %d\n", cfg_idx, pstGdcCfg->enPixelFormat);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d:enRotation: %d\n", cfg_idx, pstGdcCfg->enRotation);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_identity:Name: %s\n", cfg_idx, pstGdcCfg->identity.Name);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_identity:enModId: %d\n", cfg_idx, pstGdcCfg->identity.enModId);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_identity:u32ID: %d\n", cfg_idx, pstGdcCfg->identity.u32ID);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_identity:syncIo: %d\n", cfg_idx, pstGdcCfg->identity.syncIo);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:ldc_enable: %d\n", cfg_idx, pstGdcCfg->LdcAttr.bEnable);
        if (pstGdcCfg->LdcAttr.bEnable) {
            pstGdcCfg->LdcAttr.stAttr.bAspect               = ini_getl(tmp_section, "ldc_aspect", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32XRatio             = ini_getl(tmp_section, "ldc_xratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32YRatio             = ini_getl(tmp_section, "ldc_yratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32XYRatio            = ini_getl(tmp_section, "ldc_xyratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32CenterXOffset      = ini_getl(tmp_section, "ldc_xoffset", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32CenterYOffset      = ini_getl(tmp_section, "ldc_yoffset", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32DistortionRatio    = ini_getl(tmp_section, "ldc_distortion_ratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.Enable = ini_getl(tmp_section, "ldc_gridinfo_enable", 0, file);
            ini_gets(tmp_section, "gridFileName", " ", str_name, PARAM_STRING_NAME_LEN, file);
            strncpy(pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridFileName, str_name, PARAM_STRING_NAME_LEN);
            ini_gets(tmp_section, "gridBindName", " ", str_name, PARAM_STRING_NAME_LEN, file);
            strncpy(pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridBindName, str_name, PARAM_STRING_NAME_LEN);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:ldc_aspect: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.bAspect);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32XRatio: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32XRatio);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32YRatio: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32YRatio);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32XYRatio: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32XYRatio);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32CenterXOffset: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32CenterXOffset);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32CenterYOffset: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32CenterYOffset);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:s32DistortionRatio: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.s32DistortionRatio);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:stGridInfoAttr.Enable: %d\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.Enable);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:stGridInfoAttr.gridFileName: %s\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridFileName);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc:stGridInfoAttr.gridBindName: %s\n", cfg_idx, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridBindName);
        }
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_fisheye", cfg_idx);
        FISHEYE_ATTR_S *pstFisheyeAttr = &pstGdcCfg->FisheyeAttr;
        pstFisheyeAttr->bEnable = ini_getl(tmp_section, "fisheye_enable", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:fisheye_enable: %d\n", cfg_idx, pstFisheyeAttr->bEnable);
        if (pstFisheyeAttr->bEnable) {
            pstFisheyeAttr->bBgColor              = ini_getl(tmp_section, "fisheye_bgcolor", 0, file);
            pstFisheyeAttr->u32BgColor            = ini_getl(tmp_section, "fisheye_u32bgcolor", 0, file);
            pstFisheyeAttr->s32HorOffset          = ini_getl(tmp_section, "fisheye_s32horoffset", 0, file);
            pstFisheyeAttr->s32VerOffset          = ini_getl(tmp_section, "fisheye_s32veroffset", 0, file);
            pstFisheyeAttr->u32TrapezoidCoef      = ini_getl(tmp_section, "fisheye_u32trapezoidcoef", 0, file);
            pstFisheyeAttr->s32FanStrength        = ini_getl(tmp_section, "fisheye_s32fanstrength", 0, file);
            pstFisheyeAttr->enMountMode           = ini_getl(tmp_section, "fisheye_enmountmode", 0, file);
            pstFisheyeAttr->enUseMode             = ini_getl(tmp_section, "fisheye_enusemode", 0, file);
            pstFisheyeAttr->u32RegionNum          = ini_getl(tmp_section, "fisheye_u32regionnum", 0, file);
            pstFisheyeAttr->stGridInfoAttr.Enable = ini_getl(tmp_section, "fisheye_gridinfo_enable", 0, file);
            ini_gets(tmp_section, "gridFileName", " ", str_name, PARAM_STRING_NAME_LEN, file);
            strncpy(pstFisheyeAttr->stGridInfoAttr.gridFileName, str_name, PARAM_STRING_NAME_LEN);
            ini_gets(tmp_section, "gridBindName", " ", str_name, PARAM_STRING_NAME_LEN, file);
            strncpy(pstFisheyeAttr->stGridInfoAttr.gridBindName, str_name, PARAM_STRING_NAME_LEN);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:bBgColor: %d\n", cfg_idx, pstFisheyeAttr->bBgColor);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:u32BgColor: %d\n", cfg_idx, pstFisheyeAttr->u32BgColor);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:s32HorOffset: %d\n", cfg_idx, pstFisheyeAttr->s32HorOffset);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:s32VerOffset: %d\n", cfg_idx, pstFisheyeAttr->s32VerOffset);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:u32TrapezoidCoef: %d\n", cfg_idx, pstFisheyeAttr->u32TrapezoidCoef);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:s32FanStrength: %d\n", cfg_idx, pstFisheyeAttr->s32FanStrength);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:enMountMode: %d\n", cfg_idx, pstFisheyeAttr->enMountMode);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:enUseMode: %d\n", cfg_idx, pstFisheyeAttr->enUseMode);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:stGridInfoAttr.Enable: %d\n", cfg_idx, pstFisheyeAttr->stGridInfoAttr.Enable);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:stGridInfoAttr.gridFileName: %s\n", cfg_idx, pstFisheyeAttr->stGridInfoAttr.gridFileName);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:stGridInfoAttr.gridBindName: %s\n", cfg_idx, pstFisheyeAttr->stGridInfoAttr.gridBindName);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye:u32RegionNum: %d\n", cfg_idx, pstFisheyeAttr->u32RegionNum);
            for (CVI_U32 region_id = 0; region_id < pstFisheyeAttr->u32RegionNum; region_id++) {
                snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_fisheye_region%d", cfg_idx, region_id);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].enViewMode          = ini_getl(tmp_section, "fisheye_enviewmode", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32InRadius         = ini_getl(tmp_section, "fisheye_u32inradius", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32OutRadius        = ini_getl(tmp_section, "fisheye_u32outradius", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Pan              = ini_getl(tmp_section, "fisheye_u32pan", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Tilt             = ini_getl(tmp_section, "fisheye_u32tilt", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32HorZoom          = ini_getl(tmp_section, "fisheye_u32horzoom", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32VerZoom          = ini_getl(tmp_section, "fisheye_u32verzoom", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32X      = ini_getl(tmp_section, "fisheye_stoutrect_s32x", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32Y      = ini_getl(tmp_section, "fisheye_stoutrect_s32y", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Width  = ini_getl(tmp_section, "fisheye_stoutrect_u32width", 0, file);
                pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Height = ini_getl(tmp_section, "fisheye_stoutrect_u32height", 0, file);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:enViewMode: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].enViewMode);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32InRadius: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32InRadius);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32OutRadius: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32OutRadius);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32Pan: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Pan);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32Tilt: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Tilt);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32HorZoom: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32HorZoom);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:u32VerZoom: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32VerZoom);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:stOutRect.s32X: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32X);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:stOutRect.s32Y: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32Y);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:stOutRect.u32Width: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Width);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:stOutRect.u32Height: %d\n", cfg_idx, region_id, pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Height);
            }
        }
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_affine", cfg_idx);
        APP_AFFINE_ATTR_T *pstAffineAttr = &pstGdcCfg->AffineAttr;
        pstAffineAttr->bEnable      = ini_getl(tmp_section, "affine_enable", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine:affine_enable: %d\n", cfg_idx, pstAffineAttr->bEnable);
        if (pstAffineAttr->bEnable) {
            pstAffineAttr->stAffineAttr.stDestSize.u32Width     = ini_getl(tmp_section, "u32Width", 0, file);
            pstAffineAttr->stAffineAttr.stDestSize.u32Height    = ini_getl(tmp_section, "u32Height", 0, file);
            pstAffineAttr->stAffineAttr.u32RegionNum = ini_getl(tmp_section, "u32RegionNum", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine:u32Width: %d\n", cfg_idx, pstAffineAttr->stAffineAttr.stDestSize.u32Width);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine:u32Height: %d\n", cfg_idx, pstAffineAttr->stAffineAttr.stDestSize.u32Height);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine:u32RegionNum: %d\n", cfg_idx, pstAffineAttr->stAffineAttr.u32RegionNum);
            for (CVI_U32 region_id = 0; region_id < pstAffineAttr->stAffineAttr.u32RegionNum; region_id++) {
                memset(tmp_section, 0, sizeof(tmp_section));
                snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_affine_region%d", cfg_idx, region_id);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][0].x = ini_getl(tmp_section, "x0", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][0].y = ini_getl(tmp_section, "y0", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][1].x = ini_getl(tmp_section, "x1", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][1].y = ini_getl(tmp_section, "y1", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][2].x = ini_getl(tmp_section, "x2", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][2].y = ini_getl(tmp_section, "y2", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][3].x = ini_getl(tmp_section, "x3", 0, file);
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][3].y = ini_getl(tmp_section, "y3", 0, file);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine_region%d:(%f, %f), (%f, %f), (%f, %f), (%f, %f)\n", cfg_idx, region_id,
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][0].x, pstAffineAttr->stAffineAttr.astRegionAttr[region_id][0].y,
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][1].x, pstAffineAttr->stAffineAttr.astRegionAttr[region_id][1].y,
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][2].x, pstAffineAttr->stAffineAttr.astRegionAttr[region_id][2].y,
                pstAffineAttr->stAffineAttr.astRegionAttr[region_id][3].x, pstAffineAttr->stAffineAttr.astRegionAttr[region_id][3].y);
            }
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading gdc config ------------------> done \n");
    return CVI_SUCCESS;
}
