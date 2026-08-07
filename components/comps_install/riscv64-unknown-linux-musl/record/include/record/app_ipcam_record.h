#ifndef __APP_IPCAM_RECORD_H__
#define __APP_IPCAM_RECORD_H__

#include <cvi_comm_aio.h>
#include <linux/cvi_comm_video.h>
#include "cvi_comm_vb.h"
#include <linux/cvi_comm_venc.h>
#include "app_ipcam_comm.h"

#define APP_IPCAM_RECORD_CHN_MAX 16

typedef struct _APP_IPCAM_RECORD_AUDIO_CHN_S
{
    int s32Enable;
    int s32BindChn;//support -1: unbind must set chnattr , 0,1.... : attr from cvi audio
    int s32Type;//support 104: pcm 105: aac  only support pcm
    int s32SampleBitWidth;// support 8,16,24,32 default 16
    int s32ChnCnt; //audio sound chn number support
    int s32SamplePerFrame;//
    int s32SampleRate;
}APP_IPCAM_RECORD_AUDIO_CHN_S;

typedef struct _APP_IPCAM_RECORD_VIDEO_CHN_S
{
    int s32Enable;
    int s32BindChn;//support -1: unbind must set chnattr , 0,1.... : attr from cvi video
    int s32VencChn;//set venc chn number
    int s32Type;//support 96: h264 98: h265
    int s32Width;
    int s32Height;
    int s32BitRate;//bit rate
    int s32FrameRate;//video fps
    int s32Gop;//video gop
}APP_IPCAM_RECORD_VIDEO_CHN_S;

typedef struct _APP_IPCAM_RECORD_CHN_S
{
    int s32Chn;
    int s32RecordNum;//record file number
    APP_IPCAM_RECORD_VIDEO_CHN_S stVideoAttribute;
    APP_IPCAM_RECORD_AUDIO_CHN_S stAudioAttribute;
}APP_IPCAM_RECORD_CHN_S;

typedef struct _APP_IPCAM_RECORD_S
{
    //Manager
    int s32ChnNumber;//channle number
    int s32MediaType;//only support mov 0
    int s32SplitTimeSec;//record duration ms default 5 * 60 * 1000 (5 min)
    //Chn Handle
    APP_IPCAM_RECORD_CHN_S stChnHandle[APP_IPCAM_RECORD_CHN_MAX];
    //rec private attribute
    void * pstPrivate[APP_IPCAM_RECORD_CHN_MAX];//rec_attr (CVI_RECORDER_ATTR_T)
    void * pstRecPrivate[APP_IPCAM_RECORD_CHN_MAX];//rec_private_handle (CVI_RECORDER_HANDLE_T)
    RUN_THREAD_PARAM RecThread[APP_IPCAM_RECORD_CHN_MAX];
}APP_IPCAM_RECORD_S;

APP_IPCAM_RECORD_S * app_ipcam_Record_Param_Get();
int app_ipcam_Record_Recover_Init();
int app_ipcam_Record_Init();
int app_ipcam_Record_UnInit();
int app_ipcam_Record_VideoInput(int Chn, int enType, VENC_STREAM_S * pstStream);
int app_ipcam_Record_AudioInput(int Chn, AUDIO_FRAME_S * pstStream);


#endif
