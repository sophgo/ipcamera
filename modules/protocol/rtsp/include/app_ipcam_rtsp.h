#ifndef __APP_IPCAM_RTSP_H__
#define __APP_IPCAM_RTSP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include "linux/cvi_type.h"
#include "linux/cvi_common.h"
#include "app_ipcam_comm.h"
#include "rtsp.h"
#include "osal.h"

#define APP_RTSP_URL_LEN 256

typedef struct CVI_RTSP_SER_ATTR {
    CVI_S32 id;
    CVI_CHAR rtsp_name[MAX_RTSP_NAME_LEN];
    CVI_S32 max_conn;
    CVI_S32 timeout;
    CVI_S32 port;
    CVI_S32 auth_en;
    CVI_CHAR username[MAX_RTSP_NAME_LEN];
    CVI_CHAR password[MAX_RTSP_NAME_LEN];
    RTSP_VIDEO_FORMAT_E video_codec;
    RTSP_AUDIO_FORMAT_E audio_codec;
    CVI_S32 audio_en;
    CVI_FLOAT framerate;
    CVI_S32 bitrate_kbps;
    CVI_S32 audio_sample_rate;
    CVI_S32 audio_channels;
    CVI_S32 audio_pernum;
} CVI_RTSP_SER_ATTR_S;

typedef struct RTSP_SERVICE_CONTEXT {
    CVI_S32 ref;
    CVI_S32 mute;
    CVI_CHAR *mute_data;
    CVI_RTSP_SER_ATTR_S attr;
    OSAL_MUTEX_HANDLE_S mutex;
    OSAL_TASK_HANDLE_S media_task;
    CVI_S32 video_fd;
    CVI_S32 audio_fd;
    CVI_S32 video_exit;
    CVI_S32 audio_exit;
    CVI_VOID *rtsp_ser;
    RUN_THREAD_PARAM RtspThread;
    CVI_S32 i_frame_flag;
} RTSP_SERVICE_CONTEXT_S;

typedef struct APP_PARAM_RTSP_S {
    CVI_S32 session_cnt;
    CVI_S32 port;
    VENC_CHN VencChn[RTSP_INSTANCE_NUM];
    CVI_S32 bitrate_kbps[RTSP_INSTANCE_NUM];
    RTSP_SERVICE_CONTEXT_S *rtsp_ctx[RTSP_INSTANCE_NUM];
} APP_PARAM_RTSP_T;

typedef enum APP_RTSP_TRANSPORT_E {
    APP_RTSP_TRANS_UDP = 0,
    APP_RTSP_TRANS_TCP,
    APP_RTSP_TRANS_BUTT
} APP_RTSP_TRANSPORT_E;

typedef struct APP_RTSP_CLIENT_ATTR_S {
    CVI_CHAR url[APP_RTSP_URL_LEN];
    CVI_S32 transport;
    CVI_S32 timeout_ms;
    CVI_S32 max_frame_size;
} APP_RTSP_CLIENT_ATTR_S;

typedef struct APP_RTSP_CLIENT_FRAME_S {
    CVI_U8 *data;
    CVI_U32 len;
    CVI_U64 pts;
} APP_RTSP_CLIENT_FRAME_S;

typedef struct APP_RTSP_CLIENT_CTX_S {
    CVI_VOID *rtsp_cli;
    RTSP_FRAME_S frame;
    CVI_BOOL frame_valid;
} APP_RTSP_CLIENT_CTX_S;

typedef APP_RTSP_CLIENT_CTX_S APP_RTSP_CLIENT_HANDLE;

APP_PARAM_RTSP_T *app_ipcam_Rtsp_Param_Get(CVI_VOID);
CVI_S32 app_ipcam_Rtsp_Server_Create(CVI_VOID);
CVI_S32 app_ipcam_rtsp_Server_Destroy(CVI_VOID);

CVI_S32 app_ipcam_Rtsp_Client_Create(APP_RTSP_CLIENT_HANDLE **handle
    , const APP_RTSP_CLIENT_ATTR_S *attr);
CVI_S32 app_ipcam_Rtsp_Client_Destroy(APP_RTSP_CLIENT_HANDLE *handle);
CVI_S32 app_ipcam_Rtsp_Client_RecvVideo(APP_RTSP_CLIENT_HANDLE *handle
    , APP_RTSP_CLIENT_FRAME_S *frame, CVI_S32 timeout_ms);
CVI_S32 app_ipcam_Rtsp_Client_ReleaseVideo(APP_RTSP_CLIENT_HANDLE *handle);
CVI_S32 app_ipcam_Rtsp_Client_DropAudio(APP_RTSP_CLIENT_HANDLE *handle);

#ifdef __cplusplus
}
#endif

#endif
