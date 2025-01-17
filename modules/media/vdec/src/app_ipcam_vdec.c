#include <sys/prctl.h>
#include <stdlib.h>
#include "cvi_sys.h"
#include "cvi_vo.h"
#include "cvi_vdec.h"
#include "cvi_buffer.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_vdec.h"

/* Send frame to decoder time statistics */
// #define SHOW_STATISTICS_1
/* Send frame to video out time statistics */
// #define SHOW_STATISTICS_2

/* save yuv to /mnt/sd */
// #define SAVE_YUV

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define VO_SOURCE_FLAG "/tmp/vdec"

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
static pthread_t g_Vdec_sendstream_pthread[VDEC_MAX_CHN_NUM];

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_VDEC_CTX_S *app_ipcam_Vdec_Param_Get()
{
    return g_pstVdecCtx;
}

APP_VDEC_CHN_CFG_S *app_ipcam_VdecChnCfg_Get(VDEC_CHN VdecChn)
{
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg[VdecChn];

    return pstVdecChnCfg;
}

static void get_chroma_size_shift_factor(PIXEL_FORMAT_E enPixelFormat, CVI_S32 *w_shift, CVI_S32 *h_shift)
{
    switch (enPixelFormat) {
    case PIXEL_FORMAT_YUV_PLANAR_420:
        *w_shift = 1;
        *h_shift = 1;
        break;
    case PIXEL_FORMAT_YUV_PLANAR_422:
        *w_shift = 1;
        *h_shift = 0;
        break;
    case PIXEL_FORMAT_YUV_PLANAR_444:
        *w_shift = 0;
        *h_shift = 0;
        break;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
        *w_shift = 0;
        *h_shift = 1;
        break;
    case PIXEL_FORMAT_YUV_400: // no chroma
    default:
        *w_shift = 31;
        *h_shift = 31;
        break;
    }
}

static int app_ipcam_Vdec_YUV_Save(FILE *out_f, VIDEO_FRAME_S stVFrame)
{
    CVI_S32 c_w_shift, c_h_shift; // chroma width/height shift
    CVI_U8 *w_ptr;

    APP_PROF_LOG_PRINT(LEVEL_DEBUG,"u32Width = %d, u32Height = %d\n",
            stVFrame.u32Width, stVFrame.u32Height);
    APP_PROF_LOG_PRINT(LEVEL_DEBUG,"u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
            stVFrame.u32Stride[0], stVFrame.u32Stride[1], stVFrame.u32Stride[2]);
    APP_PROF_LOG_PRINT(LEVEL_DEBUG,"u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
            stVFrame.u32Length[0], stVFrame.u32Length[1], stVFrame.u32Length[2]);

    get_chroma_size_shift_factor(stVFrame.enPixelFormat, &c_w_shift, &c_h_shift);

    w_ptr = stVFrame.pu8VirAddr[0];
    for (CVI_U32 i = 0; i < stVFrame.u32Height; i++) {
        fwrite(w_ptr + i * stVFrame.u32Stride[0], 1, stVFrame.u32Width, out_f);
    }

    if (stVFrame.u32Length[1]) {
        w_ptr = stVFrame.pu8VirAddr[1];
        for (CVI_U32 i = 0; i < (stVFrame.u32Height >> c_h_shift); i++) {
            fwrite(w_ptr + i * stVFrame.u32Stride[1], 1, stVFrame.u32Width >> c_w_shift, out_f);
        }
    }

    if (stVFrame.u32Length[2]) {
        w_ptr = stVFrame.pu8VirAddr[2];
        for (CVI_U32 i = 0; i < (stVFrame.u32Height >> c_h_shift); i++) {
            fwrite(w_ptr + i * stVFrame.u32Stride[2], 1, stVFrame.u32Width >> c_w_shift, out_f);
        }
    }

    return CVI_SUCCESS;
}

CVI_VOID *threadSendFramesToDecoder(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_VDEC_CHN_CFG_S *pstVdecChnCfg = (APP_VDEC_CHN_CFG_S *)arg;
    VDEC_CHN VdecChn = pstVdecChnCfg->VdecChn;

    FILE *fpStrm = NULL;
    FILE *fpYuv = NULL;
    CVI_U8 *pu8Buf = NULL;
    CVI_S32 bufSize = 0;
    CVI_S32 s32UsedBytes = 0;
    CVI_S32 s32ReadLen = 0;
    CVI_U64 u64PTS = 0;
    CVI_U32 u32Start = 0;
    CVI_U32 cur_cnt = 0;
    VDEC_STREAM_S stStream = {0};
    VIDEO_FRAME_INFO_S stVdecFrame = {0};
    CVI_U32 s32Cnt = 0;

    CVI_BOOL bBindMode = CVI_FALSE;
    MMF_BIND_DEST_S stDests = {0};
    MMF_CHN_S stSrcChn = {0};
    stSrcChn.enModId = CVI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdecChn;

#ifdef SHOW_STATISTICS_1
    struct timeval pre_tv, tv1, tv2;
    int pre_cnt = 0;
    int accum_ms = 0;

    pre_tv.tv_usec = 0;
    pre_tv.tv_sec = 0;
#endif

    CVI_CHAR TaskName[64] = {'\0'};
    sprintf(TaskName, "Thread_Vdec%d_Proc", VdecChn);
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec channel_%d start running\n", VdecChn);

    fpStrm = fopen(pstVdecChnCfg->decode_file_name, "rb");
    if (fpStrm == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"open file err, filename:%s\n"
            , pstVdecChnCfg->decode_file_name);
        return (CVI_VOID *)(CVI_FAILURE);
    }

    bufSize = (pstVdecChnCfg->u32Width * pstVdecChnCfg->u32Height * 3) >> 1;
    pu8Buf = malloc(bufSize);
    if (pu8Buf == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"chn %d can't alloc %d in send stream thread!\n"
                            , VdecChn, bufSize);
        if (fpStrm) {
            fclose(fpStrm);
            fpStrm = NULL;
        }
        return (CVI_VOID *)(CVI_FAILURE);
    }
    memset(pu8Buf, 0, bufSize);

    pstVdecChnCfg->bStart = CVI_TRUE;

    s32Ret = CVI_SYS_GetBindbySrc(&stSrcChn, &stDests);
    if (s32Ret == CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,"CVI_SYS_GetBindbySrc[vdec%d] success! Skip Get chn frame...\n", VdecChn);
        bBindMode = CVI_TRUE;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN,"CVI_SYS_GetBindbySrc[vdec%d] not found! Get chn frame...\n", VdecChn);
        bBindMode = CVI_FALSE;
    }

    while (pstVdecChnCfg->bStart) {
        u32Start = 0;

        fseek(fpStrm, s32UsedBytes, SEEK_SET);
        s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
        if (s32ReadLen == 0) {
            s32UsedBytes = 0;
            fseek(fpStrm, 0, SEEK_SET);
            s32ReadLen = fread(pu8Buf, 1, bufSize, fpStrm);
        }
        /* CV186x VDEC support PT_JPEG/PT_MJPEG/PT_H264/PT_H265 */
        if (pstVdecChnCfg->astChnAttr.enMode == VIDEO_MODE_FRAME
            && pstVdecChnCfg->astChnAttr.enType == PT_H264) {
            s32Ret = h264Parse(pu8Buf, &s32ReadLen);
            if (s32Ret != CVI_SUCCESS) {
                if (s32ReadLen >= bufSize) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"can not find H264 start code! "
                        "s32ReadLen %d, s32UsedBytes %d.!\n",
                        s32ReadLen, s32UsedBytes);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Not a complete frame! ");
                }
            }
        } else if (pstVdecChnCfg->astChnAttr.enMode == VIDEO_MODE_FRAME
            && pstVdecChnCfg->astChnAttr.enType == PT_H265) {
            s32Ret = h265Parse(pu8Buf, &s32ReadLen);
            if (s32Ret != CVI_SUCCESS) {
                if (s32ReadLen >= bufSize) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"can not find H265 start code! "
                        "s32ReadLen %d, s32UsedBytes %d.!\n",
                        s32ReadLen, s32UsedBytes);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Not a complete frame! ");
                }
            }
        } else if (pstVdecChnCfg->astChnAttr.enType == PT_MJPEG
            || pstVdecChnCfg->astChnAttr.enType == PT_JPEG) {
            s32Ret = mjpegParse(pu8Buf, &s32ReadLen, &u32Start);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"Can not find JPEG start code! "
                    "s32ReadLen %d, s32UsedBytes %d.!\n",
                    s32ReadLen, s32UsedBytes);
            }
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Not support enMode:%d, enType:%d.\n"
                , pstVdecChnCfg->astChnAttr.enMode
                , pstVdecChnCfg->astChnAttr.enType);
            break;
        }

        stStream.u64PTS = u64PTS;
        stStream.pu8Addr = pu8Buf + u32Start;
        stStream.u32Len = s32ReadLen;
        stStream.bEndOfFrame = (pstVdecChnCfg->astChnAttr.enMode == VIDEO_MODE_FRAME) ? CVI_TRUE : CVI_FALSE;
        stStream.bEndOfStream = CVI_TRUE;
        stStream.bDisplay = 1;

        usleep(20*1000);

SendAgain:
#ifdef SHOW_STATISTICS_1
        gettimeofday(&tv1, NULL);
#endif
        s32Ret = CVI_VDEC_SendStream(VdecChn, &stStream, -1);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_DEBUG,"%d dec chn CVI_VDEC_SendStream err ret=%d\n"
                                , VdecChn, s32Ret);
            if (!pstVdecChnCfg->bStart) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream stop.\n");
                break;
            }
            usleep(1000);
            goto SendAgain;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "send one frame success. PTS:%"PRIu64". \n", u64PTS);
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
                        , VdecChn, accum_ms / add_cnt, avg_fps);
                pre_tv = tv2;
                pre_cnt = cur_cnt;
                accum_ms = 0;
            }
        }
#endif

        /*VDEC Bind mode not support CVI_VDEC_GetFrame*/
        if (bBindMode) {
            usleep(1000);
            continue;
        }

        /*save frame to yuv*/
        s32Ret = CVI_VDEC_GetFrame(VdecChn, &stVdecFrame, 2000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"chn %d CVI_VDEC_GetFrame fail for s32Ret=0x%x!\n", VdecChn, s32Ret);
            usleep(1000);
            continue;
        }

        if (pstVdecChnCfg->save_file_name[0] != '\0'
            && s32Cnt < pstVdecChnCfg->u32Duration) {
            if (s32Cnt == 0) {
                fpYuv = fopen(pstVdecChnCfg->save_file_name,"wb");
                if (fpYuv == NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"open file err, filename:%s.\n", pstVdecChnCfg->save_file_name);
                    return (CVI_VOID *)(CVI_FAILURE);
                }
            }

            s32Ret = app_ipcam_Vdec_YUV_Save(fpYuv, stVdecFrame.stVFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"chn %d app_ipcam_Vdec_YUV_Save fail for s32Ret=0x%x!\n", VdecChn, s32Ret);
                CVI_VDEC_ReleaseFrame(VdecChn, &stVdecFrame);
                usleep(1000);
                continue;
            }
            APP_PROF_LOG_PRINT(LEVEL_DEBUG,"---------cnt:%d----------\n",s32Cnt);
        }
        s32Cnt++;

        s32Ret = CVI_VDEC_ReleaseFrame(VdecChn, &stVdecFrame);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"chn %d CVI_VDEC_ReleaseFrame fail for s32Ret=0x%x!\n", VdecChn, s32Ret);
            usleep(1000);
            continue;
        }
    }

    if ((pstVdecChnCfg->astChnAttr.enType == PT_H264)
        || (pstVdecChnCfg->astChnAttr.enType == PT_H265)) {
        /* send the flag of stream end */
        memset(&stStream, 0, sizeof(VDEC_STREAM_S));
        stStream.bEndOfStream = CVI_TRUE;
        s32Ret = CVI_VDEC_SendStream(VdecChn, &stStream, -1);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VDEC_SendStream failed. s32Ret:%d.\n", s32Ret);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "thread_vdec_send_stream %d exit\n", VdecChn);

    if(fpStrm) {
        fclose(fpStrm);
        fpStrm = NULL;
    }

    if (pu8Buf) {
        free(pu8Buf);
        pu8Buf = NULL;
    }

    return (CVI_VOID *)CVI_SUCCESS;
}

int app_ipcam_Vdec_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    for (VDEC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVdecCtx->s32VdecChnCnt; s32ChnIdx++) {
        APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg[s32ChnIdx];
        VDEC_CHN VdecChn = pstVdecChnCfg->VdecChn;

        if ((!pstVdecChnCfg->bEnable) || (pstVdecChnCfg->bStart))
            continue;

        struct sched_param param = {0};
        pthread_attr_t attr = {0};

        param.sched_priority = 80;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        s32Ret = pthread_create(
                    &g_Vdec_sendstream_pthread[VdecChn],
                    &attr,
                    threadSendFramesToDecoder,
                    (CVI_VOID *)pstVdecChnCfg);
        if (s32Ret != 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"Create thread_vdec_send_stream failed. errCode:%d.\n", s32Ret);
            return CVI_FAILURE;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Start done.\n");
    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    for (VDEC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVdecCtx->s32VdecChnCnt; s32ChnIdx++) {
        APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg[s32ChnIdx];

        if (!pstVdecChnCfg->bStart)
            continue;

        pstVdecChnCfg->bStart = CVI_FALSE;
    }

    for (VDEC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVdecCtx->s32VdecChnCnt; s32ChnIdx++) {
        APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg[s32ChnIdx];
        VDEC_CHN VdecChn = pstVdecChnCfg->VdecChn;

        if (g_Vdec_sendstream_pthread[VdecChn] != 0) {
            s32Ret = pthread_join(g_Vdec_sendstream_pthread[VdecChn], CVI_NULL);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"pthread_join vdec_thread failed. errCode:%d.\n", s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_VDEC_StopRecvStream(VdecChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_StopRecvStream chn[%d] failed for %#x!\n"
            , VdecChn, s32Ret);
            return s32Ret;
        }

        if (pstVdecChnCfg->enVdecVBSource == VB_SOURCE_USER) {
            /* Video channel un-binding pool */
            s32Ret = CVI_VDEC_DetachVbPool(VdecChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_DetachVbPool chn[%d] failed for %#x!\n"
                , VdecChn, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_VDEC_ResetChn(VdecChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_ResetChn chn[%d] failed for %#x!\n"
            , VdecChn, s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_VDEC_DestroyChn(VdecChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_DestroyChn chn[%d] failed for %#x!\n"
            , VdecChn, s32Ret);
            return s32Ret;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Stop done.\n");

    return CVI_SUCCESS;
}

int app_ipcam_Vdec_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec init ------------------> start \n");

    for (VDEC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVdecCtx->s32VdecChnCnt; s32ChnIdx++) {
        VDEC_CHN_ATTR_S stAttr = {0};
        VDEC_MOD_PARAM_S stModParam = {0};
        VDEC_CHN_PARAM_S stChnParam = {0};
        APP_VDEC_CHN_CFG_S *pstVdecChnCfg = &g_pstVdecCtx->astVdecChnCfg[s32ChnIdx];
        VDEC_CHN VdecChn = pstVdecChnCfg->VdecChn;

        if ((!pstVdecChnCfg->bEnable) || (pstVdecChnCfg->bStart))
            continue;

        APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Vdec_Init VdecChn:%d.\n", VdecChn);

        if (pstVdecChnCfg->enVdecVBSource != VB_SOURCE_COMMON) {
            CVI_VDEC_GetModParam(&stModParam);
            stModParam.enVdecVBSource = pstVdecChnCfg->enVdecVBSource;
            CVI_VDEC_SetModParam(&stModParam);
        }

        stAttr.enType           = pstVdecChnCfg->astChnAttr.enType;
        stAttr.enMode           = pstVdecChnCfg->astChnAttr.enMode;
        stAttr.u32PicWidth      = 0;
        stAttr.u32PicHeight     = 0;
        stAttr.u32FrameBufCnt   = pstVdecChnCfg->astChnAttr.u32FrameBufCnt;
        stAttr.u32StreamBufSize = ALIGN(pstVdecChnCfg->u32Width * pstVdecChnCfg->u32Height, 0x4000);

        if (pstVdecChnCfg->astChnAttr.enType == PT_JPEG
            || pstVdecChnCfg->astChnAttr.enType == PT_MJPEG) {
            stAttr.u32FrameBufSize = VDEC_GetPicBufferSize(
                    pstVdecChnCfg->astChnAttr.enType, pstVdecChnCfg->u32Width, pstVdecChnCfg->u32Height,
                    pstVdecChnCfg->astChnParam.enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
        }

        /* Create decoder */
        s32Ret = CVI_VDEC_CreateChn(VdecChn, &stAttr);
        if (s32Ret != CVI_SUCCESS){
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_CreateChn chn[%d] failed for %#x!\n", VdecChn, s32Ret);
            return s32Ret;
        }

        if (pstVdecChnCfg->enVdecVBSource == VB_SOURCE_USER) {
            VDEC_CHN_POOL_S stPool = {0};
            stPool.hPicVbPool = pstVdecChnCfg->AttachPool;
            stPool.hTmvVbPool = VB_INVALID_POOLID;

            /* Video channel binding pool */
            s32Ret = CVI_VDEC_AttachVbPool(VdecChn, &stPool);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VDEC_AttachVbPool chn[%d] failed for %#x!\n", VdecChn, s32Ret);
                return s32Ret;
            }
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
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec init ------------------> done \n");

    return CVI_SUCCESS;
}

