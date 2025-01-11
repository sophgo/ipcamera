#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_gdc.h"

const char *gdc_type[PT_BUTT] = {
    [GDC_GRIDINFO] = "GDC_GRIDINFO",
    [GDC_MESH] = "GDC_MESH",
};

static int Load_Param_GDC_Vi(const char * file)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 enum_num = 0;
    CVI_CHAR tmp_section[64] = {0};
    CVI_CHAR tmp_buff[PARAM_STRING_LEN] = {0};
    CVI_CHAR str_name[PARAM_STRING_NAME_LEN] = {0};

    APP_PARAM_GDC_CFG_S *gdc = app_ipcam_GDC_Param_Get();

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "gdc_config_vi");
    gdc->viCnt = ini_getl(tmp_section, "gdc_vi_cnt", 0, file);

    for (int vi_idx = 0; vi_idx < gdc->viCnt; vi_idx++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config_vi_%d", vi_idx);
        gdc->stGDCViCfg[vi_idx].bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        if (gdc->stGDCViCfg[vi_idx].bEnable) {
            gdc->stGDCViCfg[vi_idx].ViPipe = ini_getl(tmp_section, "vi_pipe", 0, file);
            gdc->stGDCViCfg[vi_idx].ViChn  = ini_getl(tmp_section, "vi_chn", 0, file);

            ini_gets(tmp_section, "gdc_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
            s32Ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, gdc_type, GDC_MAX, &enum_num);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO
                    , "[%s][gdc_type] Fail to convert string name [%s] to enum number!\n"
                    , tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO
                    , "[%s][gdc_type] Convert string name [%s] to enum number [%d].\n"
                    , tmp_section, str_name, enum_num);
                gdc->stGDCViCfg[vi_idx].enType = enum_num;
            }

            APP_PROF_LOG_PRINT(LEVEL_INFO, "Load_Param_GDC_Vi vi_idx:%d, en:%d, pipe:%d, chn:%d, type:%d. \n"
                , vi_idx
                , gdc->stGDCViCfg[vi_idx].bEnable
                , gdc->stGDCViCfg[vi_idx].ViPipe
                , gdc->stGDCViCfg[vi_idx].ViChn
                , gdc->stGDCViCfg[vi_idx].enType);

            switch (gdc->stGDCViCfg[vi_idx].enType) {
                case GDC_GRIDINFO:
                {
                    gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.Enable = CVI_TRUE;
                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    ini_gets(tmp_section, "filename", " ", tmp_buff, PARAM_STRING_LEN, file);
                    strncpy(gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.gridFileName
                        , tmp_buff, PARAM_STRING_LEN);
                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    snprintf(tmp_buff, PARAM_STRING_LEN, "Pipe%d_Chn%d"
                        , gdc->stGDCViCfg[vi_idx].ViPipe
                        , gdc->stGDCViCfg[vi_idx].ViChn);
                    strncpy(gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.gridBindName
                        , tmp_buff, PARAM_STRING_LEN);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "gridFileName:%s, gridBindName:%s. \n"
                        , gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.gridFileName
                        , gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.gridBindName);
                }
                break;
                case GDC_MESH:
                {
                    gdc->stGDCViCfg[vi_idx].stLDCAttr.stGridInfoAttr.Enable =
                        ini_getl(tmp_section, "mesh_type", 0, file);
                    gdc->stGDCViCfg[vi_idx].stMeshAttr.enModId = gdc->enModId;
                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    ini_gets(tmp_section, "filename", " ", tmp_buff, PARAM_STRING_LEN, file);
                    strncpy(gdc->stGDCViCfg[vi_idx].stMeshAttr.binFileName
                        , tmp_buff, PARAM_STRING_LEN);
                    gdc->stGDCViCfg[vi_idx].stMeshAttr.viMeshAttr.chn = gdc->stGDCViCfg[vi_idx].ViChn;
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "binFileName:%s. \n"
                        , gdc->stGDCViCfg[vi_idx].stMeshAttr.binFileName);
                }
                break;
                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support enType:%d. "
                        , gdc->stGDCViCfg[vi_idx].enType);
                break;
            }
        }
    }

    return CVI_SUCCESS;
}

static int Load_Param_GDC_Vpss(const char * file)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 enum_num = 0;
    CVI_CHAR tmp_section[64] = {0};
    CVI_CHAR tmp_buff[PARAM_STRING_LEN] = {0};
    CVI_CHAR str_name[PARAM_STRING_NAME_LEN] = {0};

    APP_PARAM_GDC_CFG_S *gdc = app_ipcam_GDC_Param_Get();

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "gdc_config_vpss");
    gdc->vpssCnt = ini_getl(tmp_section, "gdc_vpss_cnt", 0, file);

    for (int vpss_idx = 0; vpss_idx < gdc->vpssCnt; vpss_idx++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "gdc_config_vpss_%d", vpss_idx);
        gdc->stGDCVpssCfg[vpss_idx].bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        if (gdc->stGDCVpssCfg[vpss_idx].bEnable) {
            gdc->stGDCVpssCfg[vpss_idx].VpssGrp = ini_getl(tmp_section, "vpss_grp", 0, file);
            gdc->stGDCVpssCfg[vpss_idx].VpssChn = ini_getl(tmp_section, "vpss_chn", 0, file);

            ini_gets(tmp_section, "gdc_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
            s32Ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, gdc_type, GDC_MAX, &enum_num);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO
                    , "[%s][gdc_type] Fail to convert string name [%s] to enum number!\n"
                    , tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO
                    , "[%s][gdc_type] Convert string name [%s] to enum number [%d].\n"
                    , tmp_section, str_name, enum_num);
                gdc->stGDCVpssCfg[vpss_idx].enType = enum_num;
            }

            APP_PROF_LOG_PRINT(LEVEL_INFO, "Load_Param_GDC_Vi vpss_idx:%d, en:%d, VpssGrp:%d, VpssChn:%d, type:%d. \n"
                , vpss_idx
                , gdc->stGDCVpssCfg[vpss_idx].bEnable
                , gdc->stGDCVpssCfg[vpss_idx].VpssGrp
                , gdc->stGDCVpssCfg[vpss_idx].VpssChn
                , gdc->stGDCVpssCfg[vpss_idx].enType);

            switch (gdc->stGDCVpssCfg[vpss_idx].enType) {
                case GDC_GRIDINFO:
                {
                    gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.Enable = CVI_TRUE;
                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    ini_gets(tmp_section, "filename", " ", tmp_buff, PARAM_STRING_LEN, file);
                    strncpy(gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.gridFileName
                        , tmp_buff, PARAM_STRING_LEN);

                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    snprintf(tmp_buff, PARAM_STRING_LEN, "Grp%d_Chn%d"
                        , gdc->stGDCVpssCfg[vpss_idx].VpssGrp
                        , gdc->stGDCVpssCfg[vpss_idx].VpssChn);
                    strncpy(gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.gridBindName
                        , tmp_buff, PARAM_STRING_LEN);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "gridFileName:%s, gridBindName:%s. \n"
                        , gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.gridFileName
                        , gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.gridBindName);
                }
                break;
                case GDC_MESH:
                {
                    gdc->stGDCVpssCfg[vpss_idx].stLDCAttr.stGridInfoAttr.Enable =
                        ini_getl(tmp_section, "mesh_type", 0, file);
                    gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.enModId = gdc->enModId;
                    memset(tmp_buff, 0, PARAM_STRING_LEN);
                    ini_gets(tmp_section, "filename", " ", tmp_buff, PARAM_STRING_LEN, file);
                    strncpy(gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.binFileName
                        , tmp_buff, PARAM_STRING_LEN);
                    gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.vpssMeshAttr.grp =
                        ini_getl(tmp_section, "vpss_grp", 0, file);
                    gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.vpssMeshAttr.chn =
                        ini_getl(tmp_section, "vpss_chn", 0, file);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Grp%d_Chn%d, binFileName:%s. \n"
                        , gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.vpssMeshAttr.grp
                        , gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.vpssMeshAttr.chn
                        , gdc->stGDCVpssCfg[vpss_idx].stMeshAttr.binFileName);
                }
                break;
                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support enType:%d. "
                        , gdc->stGDCVpssCfg[vpss_idx].enType);
                break;
            }
        }
    }

    return CVI_SUCCESS;
}

int Load_Param_GDC(const char * file)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 enum_num = 0;
    CVI_CHAR tmp_section[64] = {0};
    CVI_CHAR str_name[PARAM_STRING_NAME_LEN] = {0};

    const char ** mode_id = app_ipcam_Param_get_mode_id();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading GDC config ------------------> start \n");

    APP_PARAM_GDC_CFG_S *gdc = app_ipcam_GDC_Param_Get();

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "gdc_config");

    ini_gets(tmp_section, "gdc_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    s32Ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO
            , "[%s][gdc_mod_id] Fail to convert string name [%s] to enum number!\n"
            , tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO
            , "[%s][gdc_mod_id] Convert string name [%s] to enum number [%d].\n"
            , tmp_section, str_name, enum_num);
        gdc->enModId = enum_num;
    }

    switch(gdc->enModId) {
        case CVI_ID_VI:
            s32Ret = Load_Param_GDC_Vi(file);
        break;
        case CVI_ID_VPSS:
            s32Ret = Load_Param_GDC_Vpss(file);
        break;
        default:
            s32Ret = CVI_FAILURE;
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support modId:%d. \n"
                , gdc->enModId);
        break;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading GDC config ------------------> done \n\n");

    return s32Ret;
}
