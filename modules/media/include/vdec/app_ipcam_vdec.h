#ifndef __APP_IPCAM_VDEC_H__
#define __APP_IPCAM_VDEC_H__

#include "cvi_type.h"
#include "cvi_math.h"
#include "cvi_common.h"
#include "cvi_comm_vdec.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define VDEC_CHN_MAX 4

typedef struct APP_VDEC_CHN_CFG_T {
    CVI_BOOL    bEnable;
    CVI_BOOL    bStart;
    VDEC_CHN    VdecChn;
    CVI_U32     u32Width;
    CVI_U32     u32Height;
    CVI_CHAR    save_file_name[32];
    CVI_U32     u32Duration;
    CVI_CHAR    decode_file_name[32];
    RECT_S      astDispRect;
    VDEC_CHN_ATTR_S astChnAttr;
    VDEC_CHN_PARAM_S astChnParam;
    VB_SOURCE_E enVdecVBSource;
    VB_POOL     AttachPool;
} APP_VDEC_CHN_CFG_S;

typedef struct APP_PARAM_VDEC_CTX_T {
    CVI_S32 s32VdecChnCnt;
    APP_VDEC_CHN_CFG_S astVdecChnCfg[VDEC_MAX_CHN_NUM];
} APP_PARAM_VDEC_CTX_S;

APP_PARAM_VDEC_CTX_S *app_ipcam_Vdec_Param_Get(void);
APP_VDEC_CHN_CFG_S *app_ipcam_VdecChnCfg_Get(VDEC_CHN VdecChn);

int app_ipcam_Vdec_Start(void);
int app_ipcam_Vdec_Stop(void);
int app_ipcam_Vdec_Init(void);

#ifdef __cplusplus
}
#endif

#endif
