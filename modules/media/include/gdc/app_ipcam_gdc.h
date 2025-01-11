#ifndef __APP_IPCAM_GDC_H__
#define __APP_IPCAM_GDC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_gdc.h"

typedef enum {
    GDC_GRIDINFO = 0,
    GDC_MESH,
    GDC_MAX,
} GDC_TYPE_E;

typedef struct APP_PARAM_GDC_VI_CFG_T {
    CVI_BOOL         bEnable;
    VI_PIPE          ViPipe;
    VI_CHN           ViChn;
    GDC_TYPE_E       enType;
    LDC_ATTR_S       stLDCAttr;
    MESH_DUMP_ATTR_S stMeshAttr;
} APP_PARAM_GDC_VI_CFG_S;

typedef struct APP_PARAM_GDC_VPSS_CFG_T {
    CVI_BOOL         bEnable;
    VPSS_GRP         VpssGrp;
    VPSS_CHN         VpssChn;
    GDC_TYPE_E       enType;
    LDC_ATTR_S       stLDCAttr;
    MESH_DUMP_ATTR_S stMeshAttr;
} APP_PARAM_GDC_VPSS_CFG_S;

typedef struct APP_PARAM_GDC_CFG_T {
    MOD_ID_E enModId;
    CVI_S32 viCnt;
    APP_PARAM_GDC_VI_CFG_S stGDCViCfg[VI_MAX_PIPE_NUM];
    CVI_S32 vpssCnt;
    APP_PARAM_GDC_VPSS_CFG_S stGDCVpssCfg[VPSS_MAX_GRP_NUM * VPSS_MAX_CHN_NUM];
} APP_PARAM_GDC_CFG_S;

APP_PARAM_GDC_CFG_S *app_ipcam_GDC_Param_Get(void);
int app_ipcam_GDC_DumpMesh(void);
int app_ipcam_GDC_Init(void);
int app_ipcam_GDC_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif
