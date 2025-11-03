#ifndef __APP_IPCAM_LGVL_FRMBUF_H__
#define __APP_IPCAM_LGVL_FRMBUF_H__

#include "linux/cvi_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

CVI_S32 app_ipcam_FrmBuf_LVGL_Init(void);
CVI_S32 app_ipcam_FrmBuf_LVGL_DeInit(void);
CVI_S32 app_ipcam_FrmBuf_LVGL_Start(void);
CVI_S32 app_ipcam_FrmBuf_LVGL_Stop(void);

#ifdef __cplusplus
}
#endif

#endif
