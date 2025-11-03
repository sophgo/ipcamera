/*
 * Copyright (C) Sophgo(Cvitek) Co., Ltd. 2023. All rights reserved.
 *
 * File Name: app_ipcam_vo.h
 * Description:
 */

#ifndef __APP_IPCAM_VO_H__
#define __APP_IPCAM_VO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "linux/cvi_type.h"
#include <linux/cvi_common.h>
#include "linux/cvi_comm_vo.h"

#define APP_IPCAM_VO_MAX_NUM (VO_MAX_DEV_NUM)
typedef enum _VO_MODE_E {
    VO_MODE_1MUX,
    VO_MODE_2MUX,
    VO_MODE_4MUX,
    VO_MODE_8MUX,
    VO_MODE_9MUX,
    VO_MODE_16MUX,
    VO_MODE_25MUX,
    VO_MODE_36MUX,
    VO_MODE_49MUX,
    VO_MODE_64MUX,
    VO_MODE_2X4,
    VO_MODE_BUTT
} VO_MODE_E;
typedef struct _app_param_vo_cfg_s {
    VO_DEV s32VoDev;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_MODE_E enVoMode;
    ROTATION_E enRotation;
    CVI_U32 u32DisBufLen;
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDstChn;
    CVI_BOOL bBindMode;
} APP_PARAM_VO_CFG_T;

typedef struct _app_multi_vo_param_s {
    APP_PARAM_VO_CFG_T vo_cfg[APP_IPCAM_VO_MAX_NUM];
    CVI_U32 vo_num;
} APP_MULTI_VO_PARAM_S;

APP_MULTI_VO_PARAM_S *app_ipcam_vo_param_get(void);

CVI_S32 app_ipcam_vo_start(void);
CVI_S32 app_ipcam_vo_stop(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
