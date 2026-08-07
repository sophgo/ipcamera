#ifndef __APP_IPCAM_RTP_H__
#define __APP_IPCAM_RTP_H__

#include "linux/cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_RTP_PEER_IP_LEN 64
#define APP_RTP_DEFAULT_AU_SIZE (1024 * 1024)

typedef enum APP_RTP_ROLE_E {
	APP_RTP_ROLE_NONE = 0,
	APP_RTP_ROLE_TX,
	APP_RTP_ROLE_RX,
	APP_RTP_ROLE_BUTT
} APP_RTP_ROLE_E;

typedef struct APP_RTP_PARAM_S {
	CVI_BOOL bEnable;
	APP_RTP_ROLE_E enRole;
	CVI_S32 s32VencChn;
	CVI_S32 s32VdecChn;
	CVI_CHAR szPeerIp[APP_RTP_PEER_IP_LEN];
	CVI_U16 u16VideoPort;
	CVI_U16 u16ControlPort;
	CVI_U32 u32MaxAuSize;
} APP_RTP_PARAM_S;

APP_RTP_PARAM_S *app_ipcam_Rtp_Param_Get(void);
int app_ipcam_Rtp_Init(void);
int app_ipcam_Rtp_DeInit(void);
int app_ipcam_Rtp_SendFrame(CVI_S32 venc_chn, const CVI_U8 *data,
	CVI_U32 data_len, CVI_U64 pts);
int app_ipcam_Rtp_RecvFrame(CVI_S32 vdec_chn, CVI_U8 **data,
	CVI_U32 *data_len, CVI_U64 *pts, CVI_S32 timeout_ms);
int app_ipcam_Rtp_DropFrame(CVI_S32 vdec_chn);

#ifdef __cplusplus
}
#endif

#endif
