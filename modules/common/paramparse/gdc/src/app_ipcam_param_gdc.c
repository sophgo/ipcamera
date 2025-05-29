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
    const char ** mode_id = app_ipcam_Param_get_mode_id();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading gdc config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "gdc_config");
    Gdc->u32CfgCnt = ini_getl(tmp_section, "config_cnt", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config:config_cnt: %d\n", Gdc->u32CfgCnt);
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
        pstGdcCfg->bSaveFileEn = ini_getl(tmp_section, "savefile_en", 0, file);
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
        ini_gets(tmp_section, "enModId", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enModId] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enModId] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstGdcCfg->identity.enModId = enum_num;
        }
        pstGdcCfg->identity.u32ID    = ini_getl(tmp_section, "u32ID", 0, file);
        pstGdcCfg->identity.syncIo   = ini_getl(tmp_section, "syncIo", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d: hHandle: %ld, operation: %d, bSaveFileEn: %d, filename_in: %s, filename_out: %s\n",
            cfg_idx, pstGdcCfg->hHandle, pstGdcCfg->u32Operation, pstGdcCfg->bSaveFileEn,
            pstGdcCfg->filename_in, pstGdcCfg->filename_out);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d: width_in: %d, height_in: %d, width_out: %d, height_out: %d, enPixelFormat: %d, enRotation: %d\n",
            cfg_idx, pstGdcCfg->size_in.u32Width, pstGdcCfg->size_in.u32Height,
            pstGdcCfg->size_out.u32Width, pstGdcCfg->size_out.u32Height,
            pstGdcCfg->enPixelFormat, pstGdcCfg->enRotation);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d: identity Name: %s, enModId: %d, u32ID: %d, syncIo: %d\n",
            cfg_idx, pstGdcCfg->identity.Name, pstGdcCfg->identity.enModId,
            pstGdcCfg->identity.u32ID, pstGdcCfg->identity.syncIo);

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_ldc", cfg_idx);
        pstGdcCfg->LdcAttr.bEnable = ini_getl(tmp_section, "ldc_enable", 0, file);
        if (pstGdcCfg->LdcAttr.bEnable) {
            pstGdcCfg->LdcAttr.stAttr.bAspect               = ini_getl(tmp_section, "ldc_aspect", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32XRatio             = ini_getl(tmp_section, "ldc_xratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32YRatio             = ini_getl(tmp_section, "ldc_yratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32XYRatio            = ini_getl(tmp_section, "ldc_xyratio", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32CenterXOffset      = ini_getl(tmp_section, "ldc_xoffset", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32CenterYOffset      = ini_getl(tmp_section, "ldc_yoffset", 0, file);
            pstGdcCfg->LdcAttr.stAttr.s32DistortionRatio    = ini_getl(tmp_section, "ldc_distortion_ratio", 0, file);
            pstGdcCfg->LdcAttr.bUpdateMesh                  = ini_getl(tmp_section, "ldc_update_mesh_enable", 0, file);
            pstGdcCfg->LdcAttr.s32MeshNum                   = ini_getl(tmp_section, "ldc_mesh_number", 0, file);
            pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.Enable = ini_getl(tmp_section, "ldc_gridinfo_enable", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc: ldc_enable:%d, ldc_aspect:%d, s32XRatio:%d, s32YRatio:%d,"
                                           "s32XYRatio:%d, s32CenterXOffset:%d, s32CenterYOffset:%d, s32DistortionRatio:%d,"
                                           "bUpdateMesh: %d, s32MeshNum:%d\n",
                cfg_idx, pstGdcCfg->LdcAttr.bEnable, pstGdcCfg->LdcAttr.stAttr.bAspect,
                pstGdcCfg->LdcAttr.stAttr.s32XRatio, pstGdcCfg->LdcAttr.stAttr.s32YRatio,
                pstGdcCfg->LdcAttr.stAttr.s32XYRatio, pstGdcCfg->LdcAttr.stAttr.s32CenterXOffset,
                pstGdcCfg->LdcAttr.stAttr.s32CenterYOffset, pstGdcCfg->LdcAttr.stAttr.s32DistortionRatio,
                pstGdcCfg->LdcAttr.bUpdateMesh, pstGdcCfg->LdcAttr.s32MeshNum);

            if (pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.Enable) {
                ini_gets(tmp_section, "gridFileName", " ", str_name, PARAM_STRING_NAME_LEN, file);
                strncpy(pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridFileName, str_name, PARAM_STRING_NAME_LEN);
                ini_gets(tmp_section, "gridBindName", " ", str_name, PARAM_STRING_NAME_LEN, file);
                strncpy(pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridBindName, str_name, PARAM_STRING_NAME_LEN);
                pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_in.u32Width = pstGdcCfg->size_in.u32Width;
                pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_in.u32Height = pstGdcCfg->size_in.u32Height;
                pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_out.u32Width = pstGdcCfg->size_out.u32Width;
                pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_out.u32Height = pstGdcCfg->size_out.u32Height;
                pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.bEISEnable = ini_getl(tmp_section, "gridinfo_eis_enable", 0, file);

                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc: stGridInfoAttr.Enable: %d,"
                                               "grid_in.u32Width: %d, grid_in.u32Height: %d,"
                                               "grid_out.u32Width: %d, grid_out.u32Height: %d,"
                                               "bEISEnable: %d\n",
                    cfg_idx, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.Enable,
                    pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_in.u32Width,
                    pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_in.u32Height,
                    pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_out.u32Width,
                    pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.grid_out.u32Height,
                    pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.bEISEnable);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_ldc: stGridInfoAttr.gridFileName: %s, gridBindName: %s\n",
                    cfg_idx, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridFileName, pstGdcCfg->LdcAttr.stAttr.stGridInfoAttr.gridBindName);
            }
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_fisheye", cfg_idx);
        FISHEYE_ATTR_S *pstFisheyeAttr = &pstGdcCfg->FisheyeAttr;
        pstFisheyeAttr->bEnable = ini_getl(tmp_section, "fisheye_enable", 0, file);
        if (pstFisheyeAttr->bEnable) {
            pstFisheyeAttr->stGridInfoAttr.Enable = ini_getl(tmp_section, "fisheye_gridinfo_enable", 0, file);
            if (pstFisheyeAttr->stGridInfoAttr.Enable) {
                ini_gets(tmp_section, "gridFileName", " ", str_name, PARAM_STRING_NAME_LEN, file);
                strncpy(pstFisheyeAttr->stGridInfoAttr.gridFileName, str_name, PARAM_STRING_NAME_LEN);
                ini_gets(tmp_section, "gridBindName", " ", str_name, PARAM_STRING_NAME_LEN, file);
                strncpy(pstFisheyeAttr->stGridInfoAttr.gridBindName, str_name, PARAM_STRING_NAME_LEN);
                pstFisheyeAttr->stGridInfoAttr.grid_in.u32Width = pstGdcCfg->size_in.u32Width;
                pstFisheyeAttr->stGridInfoAttr.grid_in.u32Height = pstGdcCfg->size_in.u32Height;
                pstFisheyeAttr->stGridInfoAttr.grid_out.u32Width = pstGdcCfg->size_out.u32Width;
                pstFisheyeAttr->stGridInfoAttr.grid_out.u32Height = pstGdcCfg->size_out.u32Height;
                pstFisheyeAttr->stGridInfoAttr.bEISEnable = ini_getl(tmp_section, "gridinfo_eis_enable", 0, file);

                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye: stGridInfoAttr.Enable: %d,"
                                               "grid_in.u32Width: %d, grid_in.u32Height: %d,"
                                               "grid_out.u32Width: %d, grid_out.u32Height: %d,"
                                               "bEISEnable: %d\n",
                    cfg_idx, pstFisheyeAttr->stGridInfoAttr.Enable, pstFisheyeAttr->stGridInfoAttr.grid_in.u32Width,
                    pstFisheyeAttr->stGridInfoAttr.grid_in.u32Height, pstFisheyeAttr->stGridInfoAttr.grid_out.u32Width,
                    pstFisheyeAttr->stGridInfoAttr.grid_out.u32Height, pstFisheyeAttr->stGridInfoAttr.bEISEnable);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye: stGridInfoAttr.gridFileName: %s, gridBindName: %s\n",
                    cfg_idx, pstFisheyeAttr->stGridInfoAttr.gridFileName, pstFisheyeAttr->stGridInfoAttr.gridBindName);

            } else {
                pstFisheyeAttr->bBgColor              = ini_getl(tmp_section, "fisheye_bgcolor", 0, file);
                pstFisheyeAttr->u32BgColor            = ini_getl(tmp_section, "fisheye_u32bgcolor", 0, file);
                pstFisheyeAttr->s32HorOffset          = ini_getl(tmp_section, "fisheye_s32horoffset", 0, file);
                pstFisheyeAttr->s32VerOffset          = ini_getl(tmp_section, "fisheye_s32veroffset", 0, file);
                pstFisheyeAttr->u32TrapezoidCoef      = ini_getl(tmp_section, "fisheye_u32trapezoidcoef", 0, file);
                pstFisheyeAttr->s32FanStrength        = ini_getl(tmp_section, "fisheye_s32fanstrength", 0, file);
                pstFisheyeAttr->enMountMode           = ini_getl(tmp_section, "fisheye_enmountmode", 0, file);
                pstFisheyeAttr->enUseMode             = ini_getl(tmp_section, "fisheye_enusemode", 0, file);
                pstFisheyeAttr->u32RegionNum          = ini_getl(tmp_section, "fisheye_u32regionnum", 0, file);

                APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye: fisheye_enable: %d, bBgColor: %d,"
                                               "u32BgColor: %d, s32HorOffset: %d, s32VerOffset: %d,"
                                               "u32TrapezoidCoef: %d, s32FanStrength: %d, enMountMode: %d,"
                                               "enUseMode: %d, u32RegionNum: %d\n",
                    cfg_idx, pstFisheyeAttr->bEnable, pstFisheyeAttr->bBgColor,
                    pstFisheyeAttr->u32BgColor, pstFisheyeAttr->s32HorOffset,
                    pstFisheyeAttr->s32VerOffset, pstFisheyeAttr->u32TrapezoidCoef,
                    pstFisheyeAttr->s32FanStrength, pstFisheyeAttr->enMountMode,
                    pstFisheyeAttr->enUseMode, pstFisheyeAttr->u32RegionNum);

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

                    APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_fisheye_region%d:enViewMode: %d, u32InRadius: %d,"
                                                   "u32OutRadius: %d, u32Pan: %d, u32Tilt: %d, u32HorZoom: %d,"
                                                   "u32VerZoom: %d, stOutRect.s32X: %d, stOutRect.s32Y: %d,"
                                                   "stOutRect.u32Width: %d, stOutRect.u32Height: %d\n",
                        cfg_idx, region_id,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].enViewMode,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32InRadius,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32OutRadius,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Pan,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32Tilt,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32HorZoom,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].u32VerZoom,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32X,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.s32Y,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Width,
                        pstFisheyeAttr->astFishEyeRegionAttr[region_id].stOutRect.u32Height);
                }
            }
        }

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config%d_affine", cfg_idx);
        APP_AFFINE_ATTR_T *pstAffineAttr = &pstGdcCfg->AffineAttr;
        pstAffineAttr->bEnable      = ini_getl(tmp_section, "affine_enable", 0, file);
        if (pstAffineAttr->bEnable) {
            pstAffineAttr->stAffineAttr.stDestSize.u32Width     = ini_getl(tmp_section, "u32Width", 0, file);
            pstAffineAttr->stAffineAttr.stDestSize.u32Height    = ini_getl(tmp_section, "u32Height", 0, file);
            pstAffineAttr->stAffineAttr.u32RegionNum = ini_getl(tmp_section, "u32RegionNum", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "gdc_config%d_affine: affine_enable: %d, u32Width: %d, u32Height: %d, u32RegionNum: %d\n",
                cfg_idx, pstAffineAttr->bEnable,
                pstAffineAttr->stAffineAttr.stDestSize.u32Width,
                pstAffineAttr->stAffineAttr.stDestSize.u32Height,
                pstAffineAttr->stAffineAttr.u32RegionNum);
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
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading gdc config ------------------> done \n\n");
    return CVI_SUCCESS;
}
