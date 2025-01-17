#ifndef __APP_IPCAM_GDC_H__
#define __APP_IPCAM_GDC_H__
#include "cvi_gdc.h"
#include "cvi_math.h"
#include <cvi_type.h>
#include "cvi_common.h"
#include "cvi_comm_video.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************
 *                        H E A D E R   F I L E S
 **************************************************************************/


 /**************************************************************************
 *                              M A C R O S
 **************************************************************************/

 /**************************************************************************
 *                          C O N S T A N T S
 **************************************************************************/

/**************************************************************************
 *                          D A T A   T Y P E S
 **************************************************************************/

#define GDC_MAX_CFG_NUM 8

typedef enum _GDC__OP {
	GDC_FISHEYE = 0,
	GDC_ROT,
	GDC_LDC,
	GDC_AFFINE,
} GDC_OP;

typedef struct APP_LDC_ATTR_S {
    CVI_BOOL bEnable;
    LDC_ATTR_S stAttr;
} APP_LDC_ATTR_T;

typedef struct APP_AFFINE_ATTR_S {
    CVI_BOOL bEnable;
    AFFINE_ATTR_S stAffineAttr;
} APP_AFFINE_ATTR_T;

typedef struct APP_GDC_CFG_S {
    CVI_BOOL bEnable;
    GDC_HANDLE hHandle;
    SIZE_S size_in;
    SIZE_S size_out;
    PIXEL_FORMAT_E enPixelFormat;
    CVI_U32 u32Operation;
    GDC_IDENTITY_ATTR_S identity;
    GDC_TASK_ATTR_S stTask;
    ROTATION_E enRotation;
    APP_LDC_ATTR_T LdcAttr;
    FISHEYE_ATTR_S FisheyeAttr;
    APP_AFFINE_ATTR_T AffineAttr;
    CVI_CHAR filename_in[64];
    CVI_CHAR filename_out[64];
} APP_GDC_CFG_T;

typedef struct APP_PARAM_GDC_CFG_S {
    CVI_U32 u32CfgCnt;
    CVI_BOOL bUserEnable;
    APP_GDC_CFG_T astGdcCfg[GDC_MAX_CFG_NUM];
} APP_PARAM_GDC_CFG_T;

 /**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S
 **************************************************************************/

APP_PARAM_GDC_CFG_T *app_ipcam_Gdc_Param_Get(void);

int app_ipcam_Ldc_Init(void);
int app_ipcam_Gdc_Init(void);
int app_ipcam_Gdc_DeInit(void);
int app_ipcam_Gdc_SendFrame(APP_GDC_CFG_T *pstGdcCfg, VIDEO_FRAME_INFO_S *pstVideoFrame);
int app_ipcam_Gdc_GetFrame(APP_GDC_CFG_T *pstGdcCfg, VIDEO_FRAME_INFO_S *pstVideoFrame);
int app_ipcam_Gdc_SendFile(APP_GDC_CFG_T *pstGdcCfg);


#ifdef __cplusplus
}
#endif

#endif /* __APP_IPCAM_GDC_H__ */