#ifndef __APP_IPCAM_DPU_H__
#define __APP_IPCAM_DPU_H__

#include <semaphore.h>
#include "cvi_comm_dpu.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_venc.h"
#include "cvi_comm_ive.h"
#include "cvi_comm_video.h"
#include "cvi_comm_vb.h"
#include "cvi_vpss.h"
#include "cvi_venc.h"
#include "cvi_ive.h"
#include "cvi_vb.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum APP_DPU_DATA_SEND_TO_T {
    APP_DPU_DATA_SEND_TO_NONE,
    APP_DPU_DATA_SEND_TO_VENC,
    APP_DPU_DATA_SEND_TO_FILE,
} APP_DPU_DATA_SEND_TO_E;

typedef struct APP_DPU_GRP_CFG_S {
    DPU_GRP DpuGrp;
    CVI_BOOL bEnable;
    CVI_U32 SendTo;
    CVI_BOOL bColorMap;
    CVI_U32 VencId;
    CVI_U32 VpssGrpL;
    CVI_U32 VpssChnL;
    CVI_U32 VpssGrpR;
    CVI_U32 VpssChnR;
    CVI_BOOL bGdcGrid;
    CVI_U32 GdcCfgL;
    CVI_U32 GdcCfgR;
    DPU_GRP_ATTR_S stDpuGrpAttr;
    DPU_CHN_ATTR_S stDpuChnAttr;
} APP_DPU_GRP_CFG_T;
typedef struct APP_PARAM_DPU_CFG_S {
    CVI_U32 u32GrpCnt;
    APP_DPU_GRP_CFG_T stDpuGrpCfg[DPU_MAX_GRP_NUM];
} APP_PARAM_DPU_CFG_S;

APP_PARAM_DPU_CFG_S *app_ipcam_Dpu_Param_Get(void);
CVI_S32 app_ipcam_Dpu_Init(void);
CVI_S32 app_ipcam_Dpu_UnInit(void);
CVI_S32 app_ipcam_Dpu_Start(void);
CVI_S32 app_ipcam_Dpu_Stop(void);

#ifdef __cplusplus
}
#endif

#endif