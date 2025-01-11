#ifndef __APP_IPCAM_BLACKLIGHT_H__
#define __APP_IPCAM_BLACKLIGHT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include "cvi_ive.h"
#include "cvi_tpu_ive.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"



typedef struct APP_PARAM_BLACK_LIGHT_CTX_T {
	CVI_U32 frameWidth;
	CVI_U32 frameHeight;
	VPSS_GRP VpssGrpIR;
	VPSS_CHN VpssChnIR;
	VPSS_GRP VpssGrpRGB;
	VPSS_CHN VpssChnRGB;
	VPSS_GRP VpssGrpBlackLight;
	VPSS_CHN VpssChnBlackLight;
	VENC_CHN VencChn;

	pthread_t blackLightThread;
	volatile CVI_BOOL bStart;
	volatile CVI_BOOL bBlackLight;

	IVE_HANDLE iveHandle;
	IVE_HANDLE tpuIveHandle;
	IVE_SRC_MEM_INFO_S iveTable;
	IVE_IMAGE_S imageMap;
} APP_PARAM_BLACK_LIGHT_CTX_S;

APP_PARAM_BLACK_LIGHT_CTX_S *app_ipcam_BlackLight_Param_Get(void);
int app_ipcam_BlackLight_Init(void);
int app_ipcam_BlackLight_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif
