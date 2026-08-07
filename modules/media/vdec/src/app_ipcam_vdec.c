#include <sys/prctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "cvi_vdec.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_vdec.h"

#ifdef RTSP_SUPPORT
#include "app_ipcam_rtsp.h"
#endif

#ifdef RTP_SUPPORT
#include "app_ipcam_rtp.h"
#endif

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

#define APP_RTP_VDEC_BUF_FULL_RETRY_MAX 10
#define APP_RTP_VDEC_BUF_FULL_RETRY_US 1000

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_VDEC_CTX_S g_stVdecCtx, *g_pstVdecCtx = &g_stVdecCtx;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_VDEC_CTX_S *app_ipcam_Vdec_Param_Get() {
    return g_pstVdecCtx;
}

APP_VDEC_CHN_CFG_S *app_ipcam_VdecChnCfg_Get() {
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg;

    return pstVdecChnCfg;
}

CVI_VOID *threadSendFramesToDecoder(CVI_VOID *arg) {
    CVI_S32 s32Ret = CVI_SUCCESS;
    FILE *fpStrm = NULL;
    CVI_U8 *pu8Buf = NULL;
    CVI_S32 bufSize = 0;
    CVI_S32 s32UsedBytes = 0;
    CVI_S32 s32ReadLen = 0;
    CVI_U64 u64PTS = 0;
    CVI_U32 u32Start = 0;
    CVI_CHAR strBuf[64];
    VDEC_STREAM_S stStream = {0};
    APP_PARAM_VDEC_CTX_S *param = (APP_PARAM_VDEC_CTX_S *)arg;
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &param->astVdecChnCfg;
#ifdef RTP_SUPPORT
    CVI_U32 u32RtpSendFailCount = 0;
    CVI_U32 u32RtpBufFullRetry = 0;
    CVI_U32 u32RtpBufFullRetryCount = 0;
    CVI_U8 *rtp_frame_data = NULL;
    CVI_U32 rtp_frame_len = 0;
    CVI_U64 rtp_frame_pts = 0;
#endif
#ifdef RTSP_SUPPORT
    APP_RTSP_CLIENT_HANDLE *rtsp_hdl = NULL;
    APP_RTSP_CLIENT_FRAME_S rtsp_frame;
#endif
    memset(strBuf, 0, sizeof(strBuf));
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    snprintf(strBuf, sizeof(strBuf), "thread_vdec-%d", pstVdecChnCfg->VdecChn);
    prctl(PR_SET_NAME, strBuf);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "VdecChn:%d input_type:%d.\n",
        pstVdecChnCfg->VdecChn, pstVdecChnCfg->input_type);

    /* 根据输入源类型初始化：文件读帧或 RTSP 拉流 */
    if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_FILE) {
        fpStrm = fopen(pstVdecChnCfg->decode_file_name, "rb");
        if (fpStrm == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "open file err, filename:%s.\n"
                , pstVdecChnCfg->decode_file_name);
            return (CVI_VOID *)(CVI_FAILURE);
        }

        bufSize = (pstVdecChnCfg->u32Width * pstVdecChnCfg->u32Height * 3) >> 1;
        pu8Buf = malloc(bufSize);
        if (pu8Buf == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "chn %d can't alloc %d in send stream thread!\n",
                    pstVdecChnCfg->VdecChn, bufSize);
            if (fpStrm) {
                fclose(fpStrm);
                fpStrm = NULL;
            }
            return (CVI_VOID *)(CVI_FAILURE);
        }
        memset(pu8Buf, 0, bufSize);
    /* 根据输入源类型初始化：RTSP 输入，建立客户端连接 */
    } else if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTSP) {
#ifdef RTSP_SUPPORT
        APP_RTSP_CLIENT_ATTR_S cli_attr;
        memset(&cli_attr, 0, sizeof(cli_attr));
        strncpy(cli_attr.url, pstVdecChnCfg->rtsp_url, sizeof(cli_attr.url) - 1);
        cli_attr.transport = (pstVdecChnCfg->rtsp_transport == APP_RTSP_TRANS_UDP) ?
            APP_RTSP_TRANS_UDP : APP_RTSP_TRANS_TCP;
        cli_attr.timeout_ms = 3000;
        cli_attr.max_frame_size = 2 * 1024 * 1024;

        if (app_ipcam_Rtsp_Client_Create(&rtsp_hdl, &cli_attr) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP client create failed. url:%s.\n", pstVdecChnCfg->rtsp_url);
            return (CVI_VOID *)(CVI_FAILURE);
        }

        APP_PROF_LOG_PRINT(LEVEL_INFO, "RTSP client create ok. url:%s transport:%s.\n",
            pstVdecChnCfg->rtsp_url,
            (cli_attr.transport == APP_RTSP_TRANS_UDP) ? "udp" : "tcp");
#else
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP support not enabled.\n");
        return (CVI_VOID *)(CVI_FAILURE);
#endif
	} else if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTP) {
#ifdef RTP_SUPPORT
		if (pstVdecChnCfg->astChnAttr.enType != PT_H264) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTP input only supports H264.\n");
			return (CVI_VOID *)(CVI_FAILURE);
		}
#else
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTP support is not enabled.\n");
		return (CVI_VOID *)(CVI_FAILURE);
#endif
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Vdec input_type invalid.\n");
        return (CVI_VOID *)(CVI_FAILURE);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "VdecChn:%d thread_vdec_send_stream running\n", pstVdecChnCfg->VdecChn);

    while (param->thread_enable_flag) {
        // Impact: RTSP starts receiving immediately; static FILE playback remains paced at about 33fps.
        if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_FILE)
            usleep(30 * 1000);

        /* 文件输入：获取一帧码流并组帧送入 VDEC */
        if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_FILE) {
            u32Start = 0;
            fseek(fpStrm, s32UsedBytes, SEEK_SET);
            s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
            if (s32ReadLen == 0) {
                s32UsedBytes = 0;
                fseek(fpStrm, 0, SEEK_SET);
                s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
            }
            /* CV181x VDEC 支持 PT_JPEG/PT_MJPEG/PT_H264
             * CV180x VDEC 支持 PT_JPEG/PT_MJPEG */
            if (pstVdecChnCfg->astChnAttr.enMode == VIDEO_MODE_FRAME
                && pstVdecChnCfg->astChnAttr.enType == PT_H264) {
                s32Ret = h264Parse(pu8Buf, &s32ReadLen);
		if (s32Ret != CVI_SUCCESS) {
                    if (s32ReadLen >= bufSize) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR,"can not find H264 start code! "
                            "s32ReadLen %d, s32UsedBytes %d.!\n",
                            s32ReadLen, s32UsedBytes);
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "No a complete framee! ");
                    }
                }
            } else if (pstVdecChnCfg->astChnAttr.enMode == VIDEO_MODE_FRAME
                && pstVdecChnCfg->astChnAttr.enType == PT_H265) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"Don't support H265.\n");
            } else if (pstVdecChnCfg->astChnAttr.enType == PT_MJPEG
                || pstVdecChnCfg->astChnAttr.enType == PT_JPEG) {
                s32Ret = mjpegParse(pu8Buf, &s32ReadLen, &u32Start);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"can not find JPEG start code! "
                        "s32ReadLen %d, s32UsedBytes %d.!\n",
                        s32ReadLen, s32UsedBytes);
                    s32UsedBytes = 0;
                    continue;
                }
            } else {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support enMode:%d, enType:%d.\n"
                    , pstVdecChnCfg->astChnAttr.enMode
                    , pstVdecChnCfg->astChnAttr.enType);
                break;
            }

            stStream.u64PTS = u64PTS;
            stStream.pu8Addr = pu8Buf + u32Start;
            stStream.u32Len = s32ReadLen;
            stStream.bEndOfFrame = CVI_TRUE;
            stStream.bEndOfStream = CVI_FALSE;
            stStream.bDisplay = 1;
        /* RTSP 输入：从客户端收一帧视频 */
        } else if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTSP) {
#ifdef RTSP_SUPPORT
            s32Ret = app_ipcam_Rtsp_Client_RecvVideo(rtsp_hdl, &rtsp_frame, 200);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP recv video failed. s32Ret=%d\n", s32Ret);
                app_ipcam_Rtsp_Client_DropAudio(rtsp_hdl);
                usleep(1000);
                continue;
            }
            if (pstVdecChnCfg->astChnAttr.enType == PT_H265) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Don't support H265.\n");
                app_ipcam_Rtsp_Client_ReleaseVideo(rtsp_hdl);
                break;
            }
            stStream.u64PTS = rtsp_frame.pts;
            stStream.pu8Addr = rtsp_frame.data;
            stStream.u32Len = rtsp_frame.len;
            stStream.bEndOfFrame = CVI_TRUE;
            stStream.bEndOfStream = CVI_FALSE;
            stStream.bDisplay = 1;
#else
            break;
#endif
		} else if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTP) {
#ifdef RTP_SUPPORT
			s32Ret = app_ipcam_Rtp_RecvFrame(pstVdecChnCfg->VdecChn,
				&rtp_frame_data, &rtp_frame_len, &rtp_frame_pts, 20);
			if (s32Ret != CVI_SUCCESS)
				continue;
			stStream.u64PTS = rtp_frame_pts;
			stStream.pu8Addr = rtp_frame_data;
			stStream.u32Len = rtp_frame_len;
			stStream.bEndOfFrame = CVI_TRUE;
			stStream.bEndOfStream = CVI_FALSE;
			stStream.bDisplay = 1;
			u32RtpBufFullRetry = 0;
#else
			break;
#endif
        } else {
            break;
        }

        // What changed: Add a VENC-style one-shot VDEC frame trace controlled by /tmp/vdec_debug.
        if (access("/tmp/vdec_debug", F_OK) == 0) {
            APP_PROF_LOG_PRINT(LEVEL_WARN,
                "VdecChn(%d) input=%d codec=%d au=%u pts=%llu eof=%d eos=%d timeout=%d\n",
                pstVdecChnCfg->VdecChn, pstVdecChnCfg->input_type,
                pstVdecChnCfg->astChnAttr.enType, stStream.u32Len, stStream.u64PTS,
                stStream.bEndOfFrame, stStream.bEndOfStream,
                pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTP ? 0 : -1);
            remove("/tmp/vdec_debug");
        }

SendAgain:
        s32Ret = CVI_VDEC_SendStream(pstVdecChnCfg->VdecChn, &stStream,
            pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTP ? 0 : -1);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%d dec chn CVI_VDEC_SendStream err ret=%d\n"
                , pstVdecChnCfg->VdecChn, s32Ret);
            if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTSP) {
#ifdef RTSP_SUPPORT
                app_ipcam_Rtsp_Client_ReleaseVideo(rtsp_hdl);
#endif
                if (!param->thread_enable_flag) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream stop.\n");
                    break;
                }
                usleep(1000);
                continue;
            }
            if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTP) {
#ifdef RTP_SUPPORT
                if (s32Ret == CVI_ERR_VDEC_BUF_FULL &&
                    u32RtpBufFullRetry < APP_RTP_VDEC_BUF_FULL_RETRY_MAX) {
                    u32RtpBufFullRetry++;
                    u32RtpBufFullRetryCount++;
                    if ((u32RtpBufFullRetryCount & 0x1f) == 1) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                            "RTP VDEC buffer full au=%u retry=%u total=%u\n",
                            stStream.u32Len, u32RtpBufFullRetry, u32RtpBufFullRetryCount);
                    }
                    usleep(APP_RTP_VDEC_BUF_FULL_RETRY_US);
                    goto SendAgain;
                }
                u32RtpSendFailCount++;
                if ((u32RtpSendFailCount & 0x1f) == 1) {
                    APP_PROF_LOG_PRINT(LEVEL_WARN,
                        "RTP VDEC SendStream failed ret=%#x fail=%u\n", s32Ret,
                        u32RtpSendFailCount);
                }
                app_ipcam_Rtp_DropFrame(pstVdecChnCfg->VdecChn);
                continue;
#endif
            }
            if (!param->thread_enable_flag) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream stop.\n");
                break;
            }
            usleep(1000);
            goto SendAgain;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "send one frame success. PTS:%llu. \n", u64PTS);
            if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_FILE) {
                if (pstVdecChnCfg->astChnAttr.enType == PT_JPEG) {
                    // Impact: Static JPEG playback continuously decodes valid full images.
                    s32UsedBytes = 0;
                } else {
                    s32UsedBytes = s32UsedBytes + s32ReadLen + u32Start;
                }
            }
            u64PTS += 1;
        }

#ifdef RTSP_SUPPORT
        if (pstVdecChnCfg->input_type == APP_VDEC_INPUT_RTSP) {
            app_ipcam_Rtsp_Client_ReleaseVideo(rtsp_hdl);
            app_ipcam_Rtsp_Client_DropAudio(rtsp_hdl);
        }
#endif
    }

    if ((pstVdecChnCfg->astChnAttr.enType == PT_H264)
    || (pstVdecChnCfg->astChnAttr.enType == PT_H265)) {
        /* 发送码流结束标志 */
        memset(&stStream, 0, sizeof(VDEC_STREAM_S));
        stStream.bEndOfStream = CVI_TRUE;
        s32Ret = CVI_VDEC_SendStream(pstVdecChnCfg->VdecChn, &stStream, -1);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VDEC_SendStream failed. s32Ret:%d.\n"
                , s32Ret);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream %d exit\n", pstVdecChnCfg->VdecChn);

    if(fpStrm) {
        fclose(fpStrm);
        fpStrm = NULL;
    }

    if (pu8Buf) {
        free(pu8Buf);
        pu8Buf = NULL;
    }

#ifdef RTSP_SUPPORT
    if (rtsp_hdl) {
        app_ipcam_Rtsp_Client_Destroy(rtsp_hdl);
        rtsp_hdl = NULL;
    }
#endif

    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Start(void) {
    CVI_S32 s32Ret = CVI_SUCCESS;
    struct sched_param param = {0};
    pthread_attr_t attr = {0};
    APP_PARAM_VDEC_CTX_S *pstVdecCtx = NULL;
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = NULL;

    memset(&param, 0, sizeof(struct sched_param));
    memset(&attr, 0, sizeof(pthread_attr_t));

    pstVdecCtx = app_ipcam_Vdec_Param_Get();
    pstVdecChnCfg = app_ipcam_VdecChnCfg_Get();

    if (!pstVdecChnCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec not enable, skip start.\n");
        return CVI_SUCCESS;
    }

    pstVdecCtx->thread_enable_flag = CVI_TRUE;

    param.sched_priority = 80;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    s32Ret = pthread_create(&pstVdecCtx->send_to_vdec_thread, &attr, threadSendFramesToDecoder
            , (void *)pstVdecCtx);
    if (s32Ret != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"Create thread_vdec_send_stream failed. errCode:%d.\n", s32Ret);
        return CVI_FAILURE;
    }
    usleep(10*1000);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Start done.\n");
    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PARAM_VDEC_CTX_S *pstVdecCtx  = app_ipcam_Vdec_Param_Get();
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = app_ipcam_VdecChnCfg_Get();

    if (!pstVdecChnCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec not enable, skip stop.\n");
        return CVI_SUCCESS;
    }

    pstVdecCtx->thread_enable_flag = CVI_FALSE;

    s32Ret = pthread_join(pstVdecCtx->send_to_vdec_thread, NULL);
    if (s32Ret != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"pthread_join vdec_thread failed. errCode:%d.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VDEC_StopRecvStream(pstVdecChnCfg->VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_StopRecvStream chn[%d] failed for %#x!\n"
        , pstVdecChnCfg->VdecChn, s32Ret);
    }
    s32Ret = CVI_VDEC_DestroyChn(pstVdecChnCfg->VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_DestroyChn chn[%d] failed for %#x!\n"
        , pstVdecChnCfg->VdecChn, s32Ret);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Stop done.\n");

    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VDEC_CHN VdecChn = 0;
    VDEC_MOD_PARAM_S stModParam = {0};
    VDEC_CHN_PARAM_S stChnParam = {0};
    APP_PARAM_VDEC_CTX_S *pstVdecCtx = NULL;
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = NULL;
    VDEC_CHN_ATTR_S stAttr = {0};
    VDEC_CHN_POOL_S stPool = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec init ------------------> start \n");

    pstVdecCtx    = app_ipcam_Vdec_Param_Get();
    pstVdecChnCfg = app_ipcam_VdecChnCfg_Get();

    if (!pstVdecChnCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec not enable, skip init.\n");
        return CVI_SUCCESS;
    }

    memset(&stModParam, 0, sizeof(VDEC_MOD_PARAM_S));
    memset(&stChnParam, 0, sizeof(VDEC_CHN_PARAM_S));
    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stPool, 0, sizeof(VDEC_CHN_POOL_S));

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Init VdecChn:%d.\n", VdecChn);

    CVI_VDEC_GetModParam(&stModParam);
    stModParam.enVdecVBSource = VB_SOURCE_USER;
    CVI_VDEC_SetModParam(&stModParam);

    VdecChn                 = pstVdecChnCfg->VdecChn;
    stAttr.enType           = pstVdecChnCfg->astChnAttr.enType;
    stAttr.enMode           = pstVdecChnCfg->astChnAttr.enMode;
    stAttr.u32PicWidth      = pstVdecChnCfg->astChnAttr.u32PicWidth;
    stAttr.u32PicHeight     = pstVdecChnCfg->astChnAttr.u32PicHeight;
    stAttr.u32FrameBufCnt   = pstVdecChnCfg->astChnAttr.u32FrameBufCnt;
    stAttr.u32StreamBufSize = pstVdecChnCfg->astChnAttr.u32PicWidth * pstVdecChnCfg->astChnAttr.u32PicHeight;

    /* 创建解码通道 */
    s32Ret = CVI_VDEC_CreateChn(VdecChn, &stAttr);
    if (s32Ret != CVI_SUCCESS){
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_CreateChn chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }
    stPool.hPicVbPool = pstVdecCtx->PicVbPool;
    stPool.hTmvVbPool = VB_INVALID_POOLID;
    /* 视频通道绑定 VB 池 */
    s32Ret = CVI_VDEC_AttachVbPool(VdecChn, &stPool);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_AttachVbPool chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    /* 获取视频通道参数 */
    s32Ret = CVI_VDEC_GetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_GetChnParam chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    stChnParam.enPixelFormat      = pstVdecChnCfg->astChnParam.enPixelFormat;
    stChnParam.u32DisplayFrameNum = pstVdecChnCfg->astChnParam.u32DisplayFrameNum;
    /* 设置视频通道参数 */
    s32Ret = CVI_VDEC_SetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_SetChnParam chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    /* 启动解码通道接收码流 */
    s32Ret = CVI_VDEC_StartRecvStream(VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_StartRecvStream chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec init ------------------> done \n\n");

    return CVI_SUCCESS;
}
