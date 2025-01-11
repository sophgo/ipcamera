#include <sys/prctl.h>
#include <stdlib.h>
#include "cvi_sys.h"
#include "cvi_vo.h"
#include "cvi_vdec.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_vdec.h"


#define VO_SOURCE_FLAG "/tmp/vdec"

/* Send frame to decoder time statistics */
// #define SHOW_STATISTICS_1
/* Send frame to video out time statistics */
// #define SHOW_STATISTICS_2

/* save yuv to /mnt/sd */
// #define SAVE_YUV

APP_PARAM_VDEC_CTX_S g_stVdecCtx, *g_pstVdecCtx = &g_stVdecCtx;

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
	CVI_U32 cur_cnt = 0;
	CVI_CHAR strBuf[64];
    VDEC_STREAM_S stStream = {0};
    VIDEO_FRAME_INFO_S stVdecFrame = {0};
    APP_PARAM_VDEC_CTX_S *param = (APP_PARAM_VDEC_CTX_S *)arg;
	APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &param->astVdecChnCfg;
#ifdef SHOW_STATISTICS_1
	struct timeval pre_tv, tv1, tv2;
	int pre_cnt = 0;
	int accum_ms = 0;

	pre_tv.tv_usec = 0;
	pre_tv.tv_sec = 0;
#endif
    memset(strBuf, 0, sizeof(strBuf));
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));
    memset(&stVdecFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

	snprintf(strBuf, sizeof(strBuf), "thread_vdec-%d", pstVdecChnCfg->VdecChn);
	prctl(PR_SET_NAME, strBuf);

    fpStrm = fopen(pstVdecChnCfg->decode_file_name, "rb");
	if (fpStrm == NULL) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"open file err, filename:%s.\n"
			, pstVdecChnCfg->decode_file_name);
		return (CVI_VOID *)(CVI_FAILURE);
	}

    bufSize = (pstVdecChnCfg->u32Width * pstVdecChnCfg->u32Height * 3) >> 1;
    pu8Buf = malloc(bufSize);
    if (pu8Buf == NULL) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"chn %d can't alloc %d in send stream thread!\n",
				pstVdecChnCfg->VdecChn, bufSize);
        if (fpStrm) {
            fclose(fpStrm);
            fpStrm = NULL;
        }
		return (CVI_VOID *)(CVI_FAILURE);
	}
    memset(pu8Buf, 0, bufSize);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "VdecChn:%d thread_vdec_send_stream running\n"
		, pstVdecChnCfg->VdecChn);

	u64PTS = 0;
	while (param->thread_enable_flag) {
        usleep(30*1000);

        if (access(VO_SOURCE_FLAG, F_OK) == 0) {
            fseek(fpStrm, s32UsedBytes, SEEK_SET);
            s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
            if (s32ReadLen == 0) {
                s32UsedBytes = 0;
                fseek(fpStrm, 0, SEEK_SET);
                s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
            }
            /* CV181x VDEC support PT_JPEG/PT_MJPEG/PT_H264
            CV180X VDEC support PT_JPEG/PT_MJPEG */
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

SendAgain:
#ifdef SHOW_STATISTICS_1
            gettimeofday(&tv1, NULL);
#endif
            s32Ret = CVI_VDEC_SendStream(pstVdecChnCfg->VdecChn, &stStream, -1);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG,"%d dec chn CVI_VDEC_SendStream err ret=%d\n"
                    , pstVdecChnCfg->VdecChn, s32Ret);
                if (!param->thread_enable_flag) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream stop.\n");
                    break;
                }
                usleep(1000);
                goto SendAgain;
            } else {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "send one frame success. PTS:%llu. \n", u64PTS);
                s32UsedBytes = s32UsedBytes + s32ReadLen + u32Start;
                cur_cnt++;
                u64PTS += 1;
            }
#ifdef SHOW_STATISTICS_1
            gettimeofday(&tv2, NULL);
            int curr_ms =
                (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec/1000) - (tv1.tv_usec/1000);

            accum_ms += curr_ms;
            if (pre_tv.tv_usec == 0 && pre_tv.tv_sec == 0) {
                pre_tv = tv2;
            } else {
                unsigned long diffus = 0;

                if (pre_tv.tv_sec == tv2.tv_sec) {
                    if (tv2.tv_usec > pre_tv.tv_usec) {
                        diffus = tv2.tv_usec - pre_tv.tv_usec;
                    }
                } else if (tv2.tv_sec > pre_tv.tv_sec) {
                    diffus = (tv2.tv_sec - pre_tv.tv_sec) * 1000000;
                    diffus = diffus + tv2.tv_usec - pre_tv.tv_usec;
                }

                if (diffus == 0) {
                    pre_tv = tv2;
                    pre_cnt = cur_cnt;
                } else if (diffus > 1000000) {
                    int add_cnt = cur_cnt - pre_cnt;
                    double avg_fps = (add_cnt * 1000000.0 / (double) diffus);

                    printf("[%d] CVI_VDEC_SendStream Avg. %d ms FPS %.2lf\n"
                            , pstVdecChnCfg->VdecChn, accum_ms / add_cnt, avg_fps);
                    pre_tv = tv2;
                    pre_cnt = cur_cnt;
                    accum_ms = 0;
                }
            }
#endif
RETRY_GET_FRAME:
            s32Ret = CVI_VDEC_GetFrame(pstVdecChnCfg->VdecChn, &stVdecFrame, 1000);
            if (s32Ret != CVI_SUCCESS) {
                if (!param->thread_enable_flag) {
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "thread_send_vo stop. \n");
                    break;
                }
                if (s32Ret == CVI_ERR_VDEC_BUSY) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "get frame timeout ..retry\n");
                    goto RETRY_GET_FRAME;
                }
            }

            /* Because the SPS and PPS were sent in at the beginning, 
            valid frames cannot be obtained from the decoder, 
            so it is necessary to continue */
            if ((stVdecFrame.stVFrame.u32Width == 0)
            || (stVdecFrame.stVFrame.u32Height == 0)
            || (stVdecFrame.stVFrame.pu8VirAddr[0] == 0)
            || (stVdecFrame.stVFrame.u64PhyAddr[0] == 0)) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "get wrong frame ..retry\n");
                continue;
            }

            s32Ret = CVI_VO_SendFrame(0, 0, &stVdecFrame, 1000);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_SendFrame faile. VdecChn:%d, errCode:%d.\n"
                    , pstVdecChnCfg->VdecChn, s32Ret);
            }

            s32Ret = CVI_VDEC_ReleaseFrame(pstVdecChnCfg->VdecChn, &stVdecFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_ReleaseFrame faile. VdecChn:%d, errCode:%d.\n"
                    , pstVdecChnCfg->VdecChn, s32Ret);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "CVI_VDEC_ReleaseFrame succeed. \n");
            }
        } else {
            // If the VO_SOURCE_FLAG doesn't exist, the VO will play camera images.
            // View the relevant code in the VO module.
        }
    }

    if ((pstVdecChnCfg->astChnAttr.enType == PT_H264) 
    || (pstVdecChnCfg->astChnAttr.enType == PT_H265)) {
        /* send the flag of stream end */
        memset(&stStream, 0, sizeof(VDEC_STREAM_S));
        stStream.bEndOfStream = CVI_TRUE;
        s32Ret = CVI_VDEC_SendStream(pstVdecChnCfg->VdecChn, &stStream, -1);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VDEC_SendStream failed. s32Ret:%d.\n"
                , s32Ret);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream %d exit\n"
		, pstVdecChnCfg->VdecChn);
    if(fpStrm) {
        fclose(fpStrm);
        fpStrm = NULL;
    }

    if (pu8Buf) {
        free(pu8Buf);
        pu8Buf = NULL;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Start(void) {
	CVI_S32 s32Ret = CVI_SUCCESS;
    struct sched_param param = {0};
	pthread_attr_t attr = {0};
    APP_PARAM_VDEC_CTX_S *pstVdecCtx = NULL;

    memset(&param, 0, sizeof(struct sched_param));
    memset(&attr, 0, sizeof(pthread_attr_t));

	pstVdecCtx = app_ipcam_Vdec_Param_Get();

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

    /* Create decoder */
    s32Ret = CVI_VDEC_CreateChn(VdecChn, &stAttr);
    if (s32Ret != CVI_SUCCESS){
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_CreateChn chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }
    stPool.hPicVbPool = pstVdecCtx->PicVbPool;
    stPool.hTmvVbPool = VB_INVALID_POOLID;
	/* Video channel binding pool */
    s32Ret = CVI_VDEC_AttachVbPool(VdecChn, &stPool);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_AttachVbPool chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

	/* Get video channel parameter */
    s32Ret = CVI_VDEC_GetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_GetChnParam chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    stChnParam.enPixelFormat      = pstVdecChnCfg->astChnParam.enPixelFormat; 
    stChnParam.u32DisplayFrameNum = pstVdecChnCfg->astChnParam.u32DisplayFrameNum;
	/* Set vidoe channel parameter */
    s32Ret = CVI_VDEC_SetChnParam(VdecChn, &stChnParam);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_SetChnParam chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

	/* Video decoder start recevie stream */
    s32Ret = CVI_VDEC_StartRecvStream(VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_StartRecvStream chn[%d] failed for %#x!\n", VdecChn, s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec init ------------------> done \n");

    return CVI_SUCCESS;
}

