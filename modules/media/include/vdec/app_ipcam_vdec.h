#ifndef __APP_IPCAM_VDEC_H__
#define __APP_IPCAM_VDEC_H__

#include "linux/cvi_type.h"
#include <linux/cvi_common.h>
#include "linux/cvi_comm_vdec.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define VDEC_CHN_MAX 4
#define VDEC_RTSP_URL_LEN 256

typedef enum APP_VDEC_INPUT_TYPE_E {
    APP_VDEC_INPUT_NONE = 0,
    APP_VDEC_INPUT_FILE,
    APP_VDEC_INPUT_RTSP,
    APP_VDEC_INPUT_BUTT
} APP_VDEC_INPUT_TYPE_E;

typedef struct APP_VDEC_CHN_CFG_T {
    CVI_BOOL    bEnable;
    VDEC_CHN    VdecChn;
    CVI_U32     u32Width;
    CVI_U32     u32Height;
    CVI_CHAR    decode_file_name[64];
    APP_VDEC_INPUT_TYPE_E input_type;
    CVI_CHAR    rtsp_url[VDEC_RTSP_URL_LEN];
    CVI_S32     rtsp_transport;
    RECT_S      astDispRect;
    VDEC_CHN_ATTR_S astChnAttr;
    VDEC_CHN_PARAM_S astChnParam;

} APP_VDEC_CHN_CFG_S;

typedef struct APP_PARAM_VDEC_CTX_T {
    CVI_BOOL    bInit;
    CVI_S32     s32VdecChnCnt;
    VB_POOL     PicVbPool;
    pthread_t   send_to_vdec_thread;
    pthread_t   send_to_vpss_thread;
    CVI_BOOL    thread_enable_flag;
    APP_VDEC_CHN_CFG_S astVdecChnCfg;

} APP_PARAM_VDEC_CTX_S;

APP_PARAM_VDEC_CTX_S *app_ipcam_Vdec_Param_Get(void);
APP_VDEC_CHN_CFG_S *app_ipcam_VdecChnCfg_Get(void);

int app_ipcam_Vdec_Start(void);
int app_ipcam_Vdec_Stop(void);
int app_ipcam_Vdec_Init(void);

#ifdef __cplusplus
}
#endif

#endif
