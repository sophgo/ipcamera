#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "cvi_math.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Stitch(const char *file)
{
    CVI_S32 ret = 0;
    CVI_S32 i = 0, j = 0;
    CVI_S32 enum_num = 0;
    char tmp_section[64] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_STITCH_CFG_S *Stitch = app_ipcam_Stitch_Param_Get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** mode_id = app_ipcam_Param_get_mode_id();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Stitch config ------------------> start \n");

    //Load stitch_cfg
    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "stitch_cfg");
    Stitch->Enable = ini_getl(tmp_section, "Enable", 0, file);
    Stitch->s32GrpCnt = ini_getl(tmp_section, "grp_cnt", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Stitch_cfg Enable: %d grp_cnt: %d\n", Stitch->Enable, Stitch->s32GrpCnt);

    /*Load stitch group config*/
    for (i = 0; i < Stitch->s32GrpCnt; i++) {
        APP_PARAM_STITCH_GRP_CFG_S *StitchGrp = &Stitch->astStitchGrpCfg[i];
        StitchGrp->grpId = (STITCH_GRP) i;
        //Load STITCH_SRC_ATTR_S
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_src_attr", i);
        StitchGrp->srcNum = ini_getl(tmp_section, "src_num", 0, file);
        if (StitchGrp->srcNum == 2){
            StitchGrp->srcAttr.way_num = STITCH_2_WAY;
        } else if (StitchGrp->srcNum == 4) {
            StitchGrp->srcAttr.way_num = STITCH_4_WAY;
        }

        StitchGrp->srcAttr.size[0].u32Width = ini_getl(tmp_section, "src0_width", 0, file);
        StitchGrp->srcAttr.size[0].u32Height = ini_getl(tmp_section, "src0_height", 0, file);
        StitchGrp->srcAttr.size[1].u32Width = ini_getl(tmp_section, "src1_width", 0, file);
        StitchGrp->srcAttr.size[1].u32Height = ini_getl(tmp_section, "src1_height", 0, file);
        StitchGrp->srcAttr.size[2].u32Width = ini_getl(tmp_section, "src2_width", 0, file);
        StitchGrp->srcAttr.size[2].u32Height = ini_getl(tmp_section, "src2_height", 0, file);
        StitchGrp->srcAttr.size[3].u32Width = ini_getl(tmp_section, "src3_width", 0, file);
        StitchGrp->srcAttr.size[3].u32Height = ini_getl(tmp_section, "src3_height", 0, file);
        StitchGrp->bSyncFrameEn = ini_getl(tmp_section, "sync_frame_en", 0, file);
        StitchGrp->u64SyncFrameThresh = ini_getl(tmp_section, "sync_frame_thresh", 0, file);

        ini_gets(tmp_section, "pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            StitchGrp->srcAttr.fmt_in = enum_num;
        }

        //Load STITCH_SRC_ATTR_S.stitch_src_ovlp_attr
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_src_attr.ovlp_attr", i);
        StitchGrp->srcAttr.ovlap_attr.ovlp_lx[0] = ini_getl(tmp_section, "ovlp0_lx", 0, file);
        StitchGrp->srcAttr.ovlap_attr.ovlp_rx[0] = ini_getl(tmp_section, "ovlp0_rx", 0, file);
        StitchGrp->srcAttr.ovlap_attr.ovlp_lx[1] = ini_getl(tmp_section, "ovlp1_lx", 0, file);
        StitchGrp->srcAttr.ovlap_attr.ovlp_rx[1] = ini_getl(tmp_section, "ovlp1_rx", 0, file);
        StitchGrp->srcAttr.ovlap_attr.ovlp_lx[2] = ini_getl(tmp_section, "ovlp2_lx", 0, file);
        StitchGrp->srcAttr.ovlap_attr.ovlp_rx[2] = ini_getl(tmp_section, "ovlp2_rx", 0, file);

        //Load STITCH_SRC_ATTR_S.stitch_src_bd_attr
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_src_attr.bd_attr", i);
        StitchGrp->srcAttr.bd_attr.bd_lx[0] = ini_getl(tmp_section, "bd0_lx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_rx[0] = ini_getl(tmp_section, "bd0_rx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_lx[1] = ini_getl(tmp_section, "bd1_lx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_rx[1] = ini_getl(tmp_section, "bd1_rx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_lx[2] = ini_getl(tmp_section, "bd2_lx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_rx[2] = ini_getl(tmp_section, "bd2_rx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_lx[3] = ini_getl(tmp_section, "bd3_lx", 0, file);
        StitchGrp->srcAttr.bd_attr.bd_rx[3] = ini_getl(tmp_section, "bd3_rx", 0, file);

        //Load APP_STITCH_SRC_CFG_S
        for (j = 0; j < StitchGrp->srcNum; j++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_src%d_cfg", i, j);

            StitchGrp->srcParam[j].src_idx = (STITCH_SRC_IDX)(i * StitchGrp->srcNum + j);
            StitchGrp->srcParam[j].bBindEn = ini_getl(tmp_section, "bind_en", 0, file);

            ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                StitchGrp->srcParam[j].astChn[0].enModId = enum_num;
            }

            StitchGrp->srcParam[j].astChn[0].s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
            StitchGrp->srcParam[j].astChn[0].s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);

            ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                StitchGrp->srcParam[j].astChn[1].enModId = enum_num;
            }

            StitchGrp->srcParam[j].astChn[1].s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
            StitchGrp->srcParam[j].astChn[1].s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "StitchGrp%d: srcParam[%d] src_idx:%d bBindEn:%d "
                "src_mod_id:%d src_dev_id:%d src_chn_id:%d "
                "dst_mod_id:%d dst_dev_id:%d dst_chn_id:%d\n",
                i, j,
                StitchGrp->srcParam[j].src_idx,
                StitchGrp->srcParam[j].bBindEn,
                StitchGrp->srcParam[j].astChn[0].enModId,
                StitchGrp->srcParam[j].astChn[0].s32DevId,
                StitchGrp->srcParam[j].astChn[0].s32ChnId,
                StitchGrp->srcParam[j].astChn[1].enModId,
                StitchGrp->srcParam[j].astChn[1].s32DevId,
                StitchGrp->srcParam[j].astChn[1].s32ChnId);
        }

        //Load STITCH_OP_ATTR_S
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_op_attr", i);
        StitchGrp->opAttr.wgt_mode = ini_getl(tmp_section, "wgt_mode", 0, file);
        int datasrc =  ini_getl(tmp_section, "data_src", 0, file);
        StitchGrp->opAttr.data_src = datasrc;

        //Load STITCH_WGT_ATTR_S
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_wgt_attr", i);
        memset(StitchGrp->u64PhyAddr_alpha, 0, sizeof(StitchGrp->u64PhyAddr_alpha));
        memset(StitchGrp->u64PhyAddr_beta, 0, sizeof(StitchGrp->u64PhyAddr_beta));
        memset(StitchGrp->VirAddr_alpha, 0, sizeof(StitchGrp->VirAddr_alpha));
        memset(StitchGrp->VirAddr_beta, 0, sizeof(StitchGrp->VirAddr_beta));

        ini_gets(tmp_section, "wgt0_alpha_file", "", StitchGrp->wgt_alpha_name[0] , sizeof(StitchGrp->wgt_alpha_name[0]), file);
        ini_gets(tmp_section, "wgt0_beta_file", "", StitchGrp->wgt_beta_name[0] , sizeof(StitchGrp->wgt_beta_name[0]), file);
        StitchGrp->wgt_value_alpha[0] = ini_getl(tmp_section, "wgt0_alpha_value", 0, file);
        StitchGrp->wgt_value_beta[0] = ini_getl(tmp_section, "wgt0_beta_value", 0, file);

        ini_gets(tmp_section, "wgt1_alpha_file", "", StitchGrp->wgt_alpha_name[1] , sizeof(StitchGrp->wgt_alpha_name[1]), file);
        ini_gets(tmp_section, "wgt1_beta_file", "", StitchGrp->wgt_beta_name[1] , sizeof(StitchGrp->wgt_beta_name[1]), file);
        StitchGrp->wgt_value_alpha[1] = ini_getl(tmp_section, "wgt1_alpha_value", 0, file);
        StitchGrp->wgt_value_beta[1] = ini_getl(tmp_section, "wgt1_beta_value", 0, file);

        ini_gets(tmp_section, "wgt2_alpha_file", "", StitchGrp->wgt_alpha_name[2] , sizeof(StitchGrp->wgt_alpha_name[2]), file);
        ini_gets(tmp_section, "wgt2_beta_file", "", StitchGrp->wgt_beta_name[2] , sizeof(StitchGrp->wgt_beta_name[2]), file);
        StitchGrp->wgt_value_alpha[2] = ini_getl(tmp_section, "wgt2_alpha_value", 0, file);
        StitchGrp->wgt_value_beta[2] = ini_getl(tmp_section, "wgt2_beta_value", 0, file);

        //Load STITCH_CHN_ATTR_S
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "stitchgrp%d_chn_attr", i);
        StitchGrp->chnAttr.size.u32Width = ini_getl(tmp_section, "width", 0, file);
        StitchGrp->chnAttr.size.u32Height = ini_getl(tmp_section, "height", 0, file);
        ini_gets(tmp_section, "chn_pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            StitchGrp->chnAttr.fmt_out = enum_num;
        }
        StitchGrp->bAttachEn = ini_getl(tmp_section, "attach_en", 0, file);
        StitchGrp->u32AttachVbPool = ini_getl(tmp_section, "attach_pool", 0, file);
        StitchGrp->bSaveFileEn = ini_getl(tmp_section, "savefile_en", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: grpId:%d srcNum:%d way_num:%d sync_frame_en:%d sync_frame_thresh:%" PRIu64 "\n",
            i, StitchGrp->grpId,
            StitchGrp->srcNum,
            StitchGrp->srcAttr.way_num,
            StitchGrp->bSyncFrameEn,
            StitchGrp->u64SyncFrameThresh
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: fmt_in:%d size[0]=%dx%d size[1]=%dx%d size[2]=%dx%d size[3]=%dx%d\n",
            i, StitchGrp->srcAttr.fmt_in,
            StitchGrp->srcAttr.size[0].u32Width, StitchGrp->srcAttr.size[0].u32Height,
            StitchGrp->srcAttr.size[1].u32Width, StitchGrp->srcAttr.size[1].u32Height,
            StitchGrp->srcAttr.size[2].u32Width, StitchGrp->srcAttr.size[2].u32Height,
            StitchGrp->srcAttr.size[3].u32Width, StitchGrp->srcAttr.size[3].u32Height
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: ovlp_lx[0-2]=%d,%d,%d ovlp_rx[0-2]=%d,%d,%d\n",
            i,
            StitchGrp->srcAttr.ovlap_attr.ovlp_lx[0],
            StitchGrp->srcAttr.ovlap_attr.ovlp_lx[1],
            StitchGrp->srcAttr.ovlap_attr.ovlp_lx[2],
            StitchGrp->srcAttr.ovlap_attr.ovlp_rx[0],
            StitchGrp->srcAttr.ovlap_attr.ovlp_rx[1],
            StitchGrp->srcAttr.ovlap_attr.ovlp_rx[2]
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: bd_lx[0-3]=%d,%d,%d,%d bd_rx[0-3]=%d,%d,%d,%d\n",
            i,
            StitchGrp->srcAttr.bd_attr.bd_lx[0],
            StitchGrp->srcAttr.bd_attr.bd_lx[1],
            StitchGrp->srcAttr.bd_attr.bd_lx[2],
            StitchGrp->srcAttr.bd_attr.bd_lx[3],
            StitchGrp->srcAttr.bd_attr.bd_rx[0],
            StitchGrp->srcAttr.bd_attr.bd_rx[1],
            StitchGrp->srcAttr.bd_attr.bd_rx[2],
            StitchGrp->srcAttr.bd_attr.bd_rx[3]
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: fmt_out:%d size=%dx%d bAttachEn:%d u32AttachVbPool:%d bSaveFileEn:%d\n",
            i,
            StitchGrp->chnAttr.fmt_out,
            StitchGrp->chnAttr.size.u32Width,
            StitchGrp->chnAttr.size.u32Height,
            StitchGrp->bAttachEn,
            StitchGrp->u32AttachVbPool,
            StitchGrp->bSaveFileEn
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: opAttr.wgt_mode:%d data_src:%d\n",
            i,
            StitchGrp->opAttr.wgt_mode,
            StitchGrp->opAttr.data_src
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "StitchGrp%d: wgtAttr alpha[0-2]=%d,%d,%d beta[0-2]=%d,%d,%d\n",
            i,
            StitchGrp->wgt_value_alpha[0],
            StitchGrp->wgt_value_alpha[1],
            StitchGrp->wgt_value_alpha[2],
            StitchGrp->wgt_value_beta[0],
            StitchGrp->wgt_value_beta[1],
            StitchGrp->wgt_value_beta[2]
        );

        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.alpha0_filename: %s\n", i, StitchGrp->wgt_alpha_name[0]);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.beta0_filename: %s\n", i, StitchGrp->wgt_beta_name[0]);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.alpha1_filename: %s\n", i, StitchGrp->wgt_alpha_name[1]);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.beta1_filename: %s\n", i, StitchGrp->wgt_beta_name[1]);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.alpha2_filename: %s\n", i, StitchGrp->wgt_alpha_name[2]);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "StitchGrp%d: wgtAttr.beta2_filename: %s\n", i, StitchGrp->wgt_beta_name[2]);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Stitch config ------------------> done \n\n");
    return CVI_SUCCESS;
}