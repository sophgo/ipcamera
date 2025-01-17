#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <signal.h>
#include "cvi_math.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_dpu.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_buffer.h"
#include "cvi_dpu.h"

#include "app_ipcam_gdc.h"


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_DPU_CFG_S g_stDpuCfg;
APP_PARAM_DPU_CFG_S *g_pstDpuCfg = &g_stDpuCfg;
static pthread_t g_DPUThreadHandle[DPU_MAX_GRP_NUM];
static volatile bool Dpu_Flag = true;
static IVE_DST_MEM_INFO_S dstTblU, dstTblV, dstTblY;
static CVI_U8 FixMapU[256] = {184, 186, 186, 187, 189, 189, 191, 192, 192, 194, 196, 196, 198, 199, 199, 201,
                              203, 203, 205, 206, 206, 208, 208, 210, 212, 212, 214, 215, 215, 217, 219, 219,
                              220, 222, 222, 224, 225, 225, 227, 229, 229, 231, 233, 233, 234, 236, 236, 238,
                              238, 237, 236, 233, 232, 231, 229, 227, 226, 224, 223, 222, 220, 219, 217, 215,
                              214, 213, 212, 209, 208, 207, 204, 203, 202, 200, 199, 196, 195, 193, 190, 190,
                              187, 185, 184, 182, 179, 178, 176, 173, 172, 170, 167, 165, 162, 158, 153, 151,
                              146, 141, 139, 134, 130, 128, 123, 118, 116, 111, 109, 104, 102, 97, 95, 90, 86,
                              83, 78, 76, 71, 69, 65, 62, 58, 55, 51, 48, 43, 41, 36, 32, 30, 25, 23, 17, 18,
                              20, 21, 24, 25, 27, 28, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45,
                              47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 61, 62, 63, 64, 65, 66, 67, 68,
                              69, 71, 72, 73, 74, 75, 77, 78, 79, 80, 80, 81, 82, 84, 84, 85, 86, 87, 87, 88,
                              89, 91, 91, 91, 92, 92, 92, 93, 93, 93, 94, 95, 95, 95, 96, 96, 97, 97, 97, 98,
                              98, 98, 99, 100, 100, 100, 101, 101, 101, 102, 102, 102, 103, 103, 103, 103, 103,
                              103, 104, 104, 104, 104, 104, 104, 105, 105, 105, 106, 106, 106, 106, 106, 106,
                              106, 107, 107, 107, 107, 107, 107, 108, 108, 108};

static CVI_U8 FixMapV[256] = {115, 115, 115, 114, 114, 114, 114, 113, 113, 113, 112, 112, 112, 111, 111, 111,
                              110, 110, 110, 110, 110, 110, 110, 110, 109, 109, 109, 108, 108, 108, 107, 107,
                              107, 106, 106, 106, 105, 105, 105, 105, 105, 104, 104, 104, 103, 103, 103, 103,
                              101, 98, 96, 92, 90, 88, 84, 82, 80, 75, 74, 72, 67, 66, 63, 60, 57, 55, 53, 49,
                              47, 45, 40, 39, 37, 32, 31, 26, 25, 20, 16, 14, 10, 6, 3, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 1, 4, 10, 16, 18, 24, 30, 32, 39, 41, 47, 50, 55, 59, 64, 70,
                              73, 78, 82, 87, 89, 96, 98, 104, 107, 113, 116, 121, 124, 130, 136, 139, 144, 147,
                              153, 155, 160, 161, 166, 167, 172, 174, 178, 180, 181, 184, 186, 188, 190, 192,
                              194, 196, 198, 200, 203, 204, 207, 209, 210, 213, 215, 217, 219, 221, 223, 225,
                              227, 229, 231, 233, 235, 238, 239, 241, 244, 246, 247, 250, 252, 254, 255, 255,
                              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                              255, 253, 251, 251, 248, 246, 244, 244, 241, 239, 239, 236, 236, 236, 234, 234,
                              234, 231, 231, 231, 229, 229, 229, 226, 226, 226, 224, 224, 224, 222, 222, 222,
                              222, 219, 219, 219, 217, 217, 217, 214, 214, 214, 211, 211, 211};

static CVI_U8 FixMapY[256] = {15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21,
                              21, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 26,
                              26, 27, 27, 27, 28, 28, 28, 29, 31, 34, 36, 41, 43, 46, 50, 53, 55, 60, 62, 64,
                              69, 71, 74, 78, 81, 83, 85, 90, 92, 95, 100, 102, 104, 109, 111, 116, 118, 123,
                              128, 130, 135, 139, 142, 146, 151, 154, 158, 163, 165, 170, 175, 179, 180, 181,
                              183, 184, 185, 187, 187, 189, 190, 191, 193, 194, 195, 196, 197, 198, 199, 201,
                              201, 203, 204, 205, 207, 207, 209, 210, 211, 212, 213, 214, 215, 216, 218, 218,
                              220, 221, 222, 224, 224, 226, 224, 219, 217, 212, 210, 205, 203, 198, 196, 194,
                              191, 189, 187, 184, 182, 180, 177, 175, 173, 170, 168, 165, 163, 161, 158, 156,
                              154, 151, 149, 147, 144, 142, 140, 137, 135, 133, 130, 128, 126, 123, 121, 119,
                              116, 114, 111, 109, 107, 104, 102, 100, 97, 97, 95, 93, 90, 90, 88, 86, 83, 83,
                              81, 79, 76, 76, 75, 74, 74, 73, 72, 71, 71, 69, 68, 67, 67, 66, 65, 63, 63, 62,
                              61, 60, 60, 59, 57, 56, 56, 55, 54, 54, 53, 53, 53, 51, 51, 51, 50, 50, 50, 49,
                              49, 49, 48, 48, 48, 47, 47, 47, 45, 45, 45, 45, 44, 44, 44, 43, 43, 43, 42, 42,
                              42, 41, 41, 41};

IVE_HANDLE g_handle = NULL;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static inline CVI_S32 CHECK_DPU_GRP_VALID(DPU_GRP grp)
{
	if ((grp >= DPU_MAX_GRP_NUM) || (grp < 0)) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "DpuGrp(%d) exceeds Max(%d)\n", grp, DPU_MAX_GRP_NUM);
		return CVI_ERR_DPU_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static int app_ipcam_Dpu_GetFrameTofile(VIDEO_FRAME_INFO_S *stVideoFrame, char *filename)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		return CVI_FAILURE;
	}
    CVI_U32 u32DataLen = 0;
	for (int i = 0; i < 1; ++i) {
		u32DataLen = stVideoFrame->stVFrame.u32Length[i];
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		stVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(stVideoFrame->stVFrame.u64PhyAddr[i], stVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(stVideoFrame->stVFrame.u64PhyAddr[i],
					   stVideoFrame->stVFrame.pu8VirAddr[i],
					   stVideoFrame->stVFrame.u32Length[i]);
		fwrite(stVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);

		CVI_SYS_Munmap(stVideoFrame->stVFrame.pu8VirAddr[i], stVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);

	return s32Ret;
}

static CVI_S32 _VideoFrameInfo2Image(VIDEO_FRAME_INFO_S *pstVFISrc, IVE_IMAGE_S *pstIIDst) {
    CVI_U32 u32Channel = 1;
    VIDEO_FRAME_S *pstVFSrc = &pstVFISrc->stVFrame;
    IVE_IMAGE_TYPE_E img_type = IVE_IMAGE_TYPE_U8C1;

    switch (pstVFSrc->enPixelFormat) {
        case PIXEL_FORMAT_YUV_400: {
            img_type = IVE_IMAGE_TYPE_U8C1;
        } break;
        case PIXEL_FORMAT_NV21:
        case PIXEL_FORMAT_NV12: {
            img_type = IVE_IMAGE_TYPE_YUV420SP;
            u32Channel = 2;
        } break;
        case PIXEL_FORMAT_YUV_PLANAR_420: {
            img_type = IVE_IMAGE_TYPE_YUV420P;
            u32Channel = 3;
        } break;
        case PIXEL_FORMAT_YUV_PLANAR_422: {
            img_type = IVE_IMAGE_TYPE_YUV422P;
            u32Channel = 3;
        } break;
        case PIXEL_FORMAT_RGB_888:
        case PIXEL_FORMAT_BGR_888: {
            img_type = IVE_IMAGE_TYPE_U8C3_PACKAGE;
        } break;
        case PIXEL_FORMAT_RGB_888_PLANAR: {
            img_type = IVE_IMAGE_TYPE_U8C3_PLANAR;
            u32Channel = 3;
        } break;
        case PIXEL_FORMAT_INT16_C1: {
            img_type = IVE_IMAGE_TYPE_S16C1;
        } break;
        case PIXEL_FORMAT_UINT16_C1: {
            img_type = IVE_IMAGE_TYPE_U16C1;
        } break;
        default: {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Unsupported conversion type: %u.\n", pstVFSrc->enPixelFormat);
            return CVI_FAILURE;
        }
    }
    for (CVI_U32 i = 0; i < u32Channel; i++) {
        pstIIDst->u64VirAddr[i] =
        pstIIDst->u64PhyAddr[i] = pstVFSrc->u64PhyAddr[i];
        pstIIDst->u32Stride[i] = pstVFSrc->u32Stride[i];
    }
    pstIIDst->enType = img_type;
    pstIIDst->u32Width = pstVFSrc->u32Width;
    pstIIDst->u32Height = pstVFSrc->u32Height;
    return CVI_SUCCESS;
}

void app_ipcam_Dpu_Color_Map_Init()
{
    CVI_U32 dstTblByteSize = 256;
    g_handle = CVI_IVE_CreateHandle();
    CVI_U32 i = 0;

    CVI_IVE_CreateMemInfo(g_handle, &dstTblU, dstTblByteSize);
    CVI_IVE_CreateMemInfo(g_handle, &dstTblV, dstTblByteSize);
    CVI_IVE_CreateMemInfo(g_handle, &dstTblY, dstTblByteSize);

    for (i = 0; i < dstTblByteSize; i++) {
        ((char *)(uintptr_t)dstTblU.u64VirAddr)[i] = FixMapU[i];
    }
    for (i = 0; i < dstTblByteSize; i++) {
        ((char *)(uintptr_t)dstTblV.u64VirAddr)[i] = FixMapV[i];
    }
    for (i = 0; i < dstTblByteSize; i++) {
        ((char *)(uintptr_t)dstTblY.u64VirAddr)[i] = FixMapY[i];
    }
}

static void app_ipcam_Dpu_Color_Map_DeInit()
{
    CVI_IVE_DestroyHandle(g_handle);
}

CVI_VOID app_ipcam_Dpu_Color_Map(VIDEO_FRAME_INFO_S *stDpuFrame)
{
    if (stDpuFrame->stVFrame.enPixelFormat != PIXEL_FORMAT_YUV_PLANAR_420) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"only PIXEL_FORMAT_YUV_PLANAR_420 can be colored");
        return;
    }
	CVI_U32 VpssGrp = 2;
	CVI_U32 VpssChn = 0;
	VIDEO_FRAME_INFO_S stFrame2Ive = {0};
	IVE_SRC_IMAGE_S iveInput, src[1], src_dpu;
	IVE_DST_IMAGE_S g_output_1, g_output_2, g_output_3;

	IVE_MAP_CTRL_S stMapCtrl;

	CVI_S32 s32Ret = CVI_SUCCESS;

    VIDEO_FRAME_INFO_S stFrameVpssGet = {0};
    // VPSS resize image form dpu
    s32Ret = CVI_VPSS_SendFrame(VpssGrp, stDpuFrame, 3000);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[color map]CVI_VPSS_SendFrame fail with %x\n", s32Ret);
    }

    s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrameVpssGet, 3000);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(
            LEVEL_ERROR, "[color map]CVI_VPSS_GetChnFrame grp%d chn%d failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
    }

    //Do VIDEO_FRAME_INFO_S to IVE_SRC_IMAGE_S for vpss img
    memcpy(&stFrame2Ive, &stFrameVpssGet, sizeof(VIDEO_FRAME_INFO_S));
    stFrame2Ive.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_400;
    _VideoFrameInfo2Image(&stFrame2Ive, &iveInput);
    memcpy(&src[0], &iveInput, sizeof(IVE_SRC_IMAGE_S));

	//Do VIDEO_FRAME_INFO_S to IVE_SRC_IMAGE_S for dpu img
	memcpy(&stFrame2Ive, stDpuFrame, sizeof(VIDEO_FRAME_INFO_S));
	stFrame2Ive.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_400;
	_VideoFrameInfo2Image(&stFrame2Ive, &iveInput);
	memcpy(&src_dpu, &iveInput, sizeof(IVE_SRC_IMAGE_S));

	// YUV Map
	stMapCtrl.enMode = IVE_MAP_MODE_U8;
	g_output_1.enType = IVE_IMAGE_TYPE_U8C1;
	g_output_1.u64VirAddr[0] = (CVI_U64)(uintptr_t)stDpuFrame->stVFrame.pu8VirAddr[1];
	g_output_1.u64PhyAddr[0] = stDpuFrame->stVFrame.u64PhyAddr[1];
	g_output_1.u32Stride[0] = stDpuFrame->stVFrame.u32Stride[1];
	g_output_1.u32Width = stFrameVpssGet.stVFrame.u32Width;
	g_output_1.u32Height = stFrameVpssGet.stVFrame.u32Height;
	CVI_IVE_Map(g_handle, &src[0], &dstTblU, &g_output_1, &stMapCtrl, 1);

	g_output_2.enType = IVE_IMAGE_TYPE_U8C1;
	g_output_2.u64VirAddr[0] = (CVI_U64)(uintptr_t)stDpuFrame->stVFrame.pu8VirAddr[2];
	g_output_2.u64PhyAddr[0] = stDpuFrame->stVFrame.u64PhyAddr[2];
	g_output_2.u32Stride[0] = stDpuFrame->stVFrame.u32Stride[2];
	g_output_2.u32Width = stFrameVpssGet.stVFrame.u32Width;
	g_output_2.u32Height = stFrameVpssGet.stVFrame.u32Height;
	CVI_IVE_Map(g_handle, &src[0], &dstTblV, &g_output_2, &stMapCtrl, 1);

	g_output_3.enType = IVE_IMAGE_TYPE_U8C1;
	g_output_3.u64VirAddr[0] = (CVI_U64)(uintptr_t)stDpuFrame->stVFrame.pu8VirAddr[0];
	g_output_3.u64PhyAddr[0] = stDpuFrame->stVFrame.u64PhyAddr[0];
	g_output_3.u32Stride[0] = stDpuFrame->stVFrame.u32Stride[0];
	g_output_3.u32Width = stFrameVpssGet.stVFrame.u32Width;
	g_output_3.u32Height = stFrameVpssGet.stVFrame.u32Height;
	CVI_IVE_Map(g_handle, &src_dpu, &dstTblY, &g_output_3, &stMapCtrl, 1);

	CVI_VPSS_ReleaseChnFrame(VpssGrp,  VpssChn, &stFrameVpssGet);
}

static CVI_S32 DPU_GDC_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "Null pointer!\n");
		return CVI_FAILURE;
	}

	//COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		//, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN, &stVbCalConfig);
	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, 1, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}
	for (int i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i] = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		memset(pstVideoFrame->stVFrame.pu8VirAddr[i], 0, pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonFlushCache(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Dpu_Gdc_Proc(VIDEO_FRAME_INFO_S* VideoFrame, APP_GDC_CFG_T* astGdcCfg)
{
	CVI_S32 s32Ret;
	LDC_ATTR_S LDCAttr;
	ROTATION_E enRotation;
    SIZE_S stSize;
    PIXEL_FORMAT_E enPixelFormat;
    stSize.u32Width = VideoFrame->stVFrame.u32Width;
    stSize.u32Height = VideoFrame->stVFrame.u32Height;
    enPixelFormat = VideoFrame->stVFrame.enPixelFormat;
    memset(astGdcCfg->stTask.au64privateData, 0, sizeof(astGdcCfg->stTask.au64privateData));
	memcpy(&astGdcCfg->stTask.stImgIn, VideoFrame, sizeof(astGdcCfg->stTask.stImgIn));

    s32Ret = DPU_GDC_COMM_PrepareFrame(&stSize, enPixelFormat, &astGdcCfg->stTask.stImgOut);
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"MEDIA_DPU_PrepareFrame failed!\n");
		goto exit2;
	}

    s32Ret = CVI_GDC_BeginJob(&astGdcCfg->hHandle);
	if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_GDC_BeginJob failed!\n");
		goto exit2;
	}

	s32Ret = CVI_GDC_SetJobIdentity(astGdcCfg->hHandle, &astGdcCfg->identity);
	if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_GDC_SetJobIdentity failed!\n");
		goto exit2;
	}

	memcpy(&LDCAttr, &astGdcCfg->LdcAttr.stAttr,sizeof(LDCAttr));
	enRotation = 0;
	s32Ret = CVI_GDC_AddLDCTask(astGdcCfg->hHandle, &astGdcCfg->stTask, &LDCAttr, enRotation);
	if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_GDC_EndJob(astGdcCfg->hHandle);
	if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_GDC_EndJob failed!\n");
		goto exit2;
	}

	return s32Ret;

exit2:
	if (s32Ret != CVI_SUCCESS)
		if (astGdcCfg->hHandle)
			s32Ret |= CVI_GDC_CancelJob(astGdcCfg->hHandle);
	s32Ret |= CVI_GDC_DeInit();
	APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_GDC_DeInit fail.\n");
	return CVI_FAILURE;
}

static int count = 0;
static CVI_VOID *Thread_DPU_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_DPU_GRP_CFG_T GrpCfg = *((APP_DPU_GRP_CFG_T*)arg);
    VIDEO_FRAME_INFO_S stInVideoFrame_L;
    VIDEO_FRAME_INFO_S stInVideoFrame_R;
    VIDEO_FRAME_INFO_S stOutVideoFrame;
    CVI_U32 timeout = 1000;
    char file[64] = {0};
    struct timeval tv1;
    VB_BLK blk_r_out;
    APP_PARAM_GDC_CFG_T *stGdcCfg = app_ipcam_Gdc_Param_Get();
    if (GrpCfg.bColorMap) app_ipcam_Dpu_Color_Map_Init();
    if (GrpCfg.bGdcGrid)  CVI_GDC_Init();

    while (Dpu_Flag) {
        count++;
        s32Ret = CVI_VPSS_GetChnFrame(GrpCfg.VpssGrpL, GrpCfg.VpssChnL, &stInVideoFrame_L, timeout);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_GetChnFrame left frame failed with %#x\n", s32Ret);
            continue;
        }

        s32Ret = CVI_VPSS_GetChnFrame(GrpCfg.VpssGrpR, GrpCfg.VpssChnR, &stInVideoFrame_R, timeout);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_GetChnFrame rignt frame failed with %#x\n", s32Ret);
            continue;
        }

        if (GrpCfg.bGdcGrid) {
            app_ipcam_Dpu_Gdc_Proc(&stInVideoFrame_L, &stGdcCfg->astGdcCfg[GrpCfg.GdcCfgL]);
            app_ipcam_Dpu_Gdc_Proc(&stInVideoFrame_R, &stGdcCfg->astGdcCfg[GrpCfg.GdcCfgR]);
            s32Ret = CVI_DPU_SendFrame(GrpCfg.DpuGrp, &stGdcCfg->astGdcCfg[GrpCfg.GdcCfgL].stTask.stImgOut, 
                                    &stGdcCfg->astGdcCfg[GrpCfg.GdcCfgR].stTask.stImgOut, timeout);
        } else {
            s32Ret = CVI_DPU_SendFrame(GrpCfg.DpuGrp, &stInVideoFrame_L,
                                    &stInVideoFrame_R, timeout);
        }

        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_SendFrame failed with %#x\n", s32Ret);
            continue;
        }

        s32Ret = CVI_DPU_GetFrame(GrpCfg.DpuGrp, 0, &stOutVideoFrame, timeout);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_GetFrame failed with %#x\n", s32Ret);
            continue;
        }

        //release gdc output
        if (GrpCfg.bGdcGrid) {
            blk_r_out = CVI_VB_PhysAddr2Handle(stGdcCfg->astGdcCfg[GrpCfg.GdcCfgR].stTask.stImgOut.stVFrame.u64PhyAddr[0]);
            if(blk_r_out != VB_INVALID_HANDLE){
                s32Ret =  CVI_VB_ReleaseBlock(blk_r_out);
                if (s32Ret != CVI_SUCCESS)
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VB_ReleaseBlock(blk_r out) failed!\n");
            }
            blk_r_out = CVI_VB_PhysAddr2Handle(stGdcCfg->astGdcCfg[GrpCfg.GdcCfgL].stTask.stImgOut.stVFrame.u64PhyAddr[0]);
            if(blk_r_out != VB_INVALID_HANDLE){
                s32Ret =  CVI_VB_ReleaseBlock(blk_r_out);
                if (s32Ret != CVI_SUCCESS)
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VB_ReleaseBlock(blk_r out) failed!\n");
            }
        }

        CVI_VPSS_ReleaseChnFrame(1, 1, &stInVideoFrame_R);
        CVI_VPSS_ReleaseChnFrame(0, 1, &stInVideoFrame_L);

        if (GrpCfg.bColorMap) app_ipcam_Dpu_Color_Map(&stOutVideoFrame);

        if (GrpCfg.SendTo == APP_DPU_DATA_SEND_TO_FILE && count % 10 == 0) {
            gettimeofday(&tv1, NULL);
            memset(file, 0, sizeof(file));
            snprintf(file, sizeof(file), "/mnt/sd/vpss_dpu%d.bin", count);
            app_ipcam_Dpu_GetFrameTofile(&stOutVideoFrame, file);
        }  else if (GrpCfg.SendTo == APP_DPU_DATA_SEND_TO_VENC) {
            CVI_VENC_SendFrame(GrpCfg.VencId, &stOutVideoFrame, timeout);
        }

        CVI_DPU_ReleaseFrame(GrpCfg.DpuGrp, 0, &stOutVideoFrame);

        if (GrpCfg.stDpuGrpAttr.bIsBtcostOut) {
            VIDEO_FRAME_INFO_S stOutBtVideoFrame;

            s32Ret = CVI_DPU_GetFrame(GrpCfg.DpuGrp, 1 , &stOutBtVideoFrame, timeout);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_GetFrame failed with %#x\n", s32Ret);
                continue;
            }

            if (GrpCfg.SendTo == APP_DPU_DATA_SEND_TO_FILE) {
                gettimeofday(&tv1, NULL);
                memset(file, 0, sizeof(file));
                snprintf(file, sizeof(file), "/mnt/sd/dpu_image_bt_grp_%d_w_%d_h_%d_tv_%ld_%ld.bin", GrpCfg.DpuGrp,
                        stOutBtVideoFrame.stVFrame.u32Width, stOutBtVideoFrame.stVFrame.u32Height, tv1.tv_sec, tv1.tv_usec);
                app_ipcam_Dpu_GetFrameTofile(&stOutBtVideoFrame, file);
            }

            CVI_DPU_ReleaseFrame(GrpCfg.DpuGrp, 1, &stOutBtVideoFrame);
        }
    }
    if (GrpCfg.bColorMap) app_ipcam_Dpu_Color_Map_DeInit();
    if (GrpCfg.bGdcGrid) CVI_GDC_DeInit();
    return NULL;;
}

APP_PARAM_DPU_CFG_S *app_ipcam_Dpu_Param_Get(void)
{
    return g_pstDpuCfg;
}

CVI_S32 app_ipcam_Dpu_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    for (CVI_U32 DpuGrp = 0; DpuGrp < g_pstDpuCfg->u32GrpCnt; DpuGrp ++) {
        if (!g_pstDpuCfg->stDpuGrpCfg[DpuGrp].bEnable)
            continue;

        s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
        if (s32Ret != CVI_SUCCESS)
            return s32Ret;

        s32Ret = CVI_DPU_CreateGrp(DpuGrp, &g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuGrpAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_CreateGrp(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 0, &g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuChnAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_SetChnAttr failed with %#x, chn id is 0\n", s32Ret);
            CVI_DPU_DestroyGrp(DpuGrp);
            return CVI_FAILURE;
        }

        s32Ret = CVI_DPU_EnableChn(DpuGrp, 0);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_EnableChn failed with %#x, chn id is 0\n", s32Ret);
            CVI_DPU_DisableChn(DpuGrp, 0);
            CVI_DPU_DestroyGrp(DpuGrp);
            return CVI_FAILURE;
        }

        if (g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuGrpAttr.bIsBtcostOut) {
            s32Ret = CVI_DPU_SetChnAttr(DpuGrp, 1, &g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuChnAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_SetChnAttr failed with %#x, chn id is 1\n", s32Ret);
                CVI_DPU_DestroyGrp(DpuGrp);
                return CVI_FAILURE;
            }

            s32Ret = CVI_DPU_EnableChn(DpuGrp, 1);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_EnableChn failed with %#x, chn id is 1\n", s32Ret);
                CVI_DPU_DisableChn(DpuGrp, 0);
                CVI_DPU_DisableChn(DpuGrp, 1);
                CVI_DPU_DestroyGrp(DpuGrp);
                return CVI_FAILURE;
            }
        }

        s32Ret = CVI_DPU_StartGrp(DpuGrp);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_DPU_StartGrp failed with %#x\n", s32Ret);
            CVI_DPU_DisableChn(DpuGrp, 0);
            if (g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuGrpAttr.bIsBtcostOut) CVI_DPU_DisableChn(DpuGrp, 1);
            CVI_DPU_DestroyGrp(DpuGrp);
            return CVI_FAILURE;
        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_Dpu_UnInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    for (CVI_U32 DpuGrp = 0; DpuGrp < g_pstDpuCfg->u32GrpCnt; DpuGrp ++) {
        if (!g_pstDpuCfg->stDpuGrpCfg[DpuGrp].bEnable)
            continue;

        s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
        if (s32Ret != CVI_SUCCESS)
            return s32Ret;

        s32Ret = CVI_DPU_DisableChn(DpuGrp, 0);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "DPU stop Grp %d channel %d failed! Please check param\n", DpuGrp, 0);
            return s32Ret;
        }

        if (g_pstDpuCfg->stDpuGrpCfg[DpuGrp].stDpuGrpAttr.bIsBtcostOut) {
            s32Ret = CVI_DPU_DisableChn(DpuGrp, 1);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "DPU stop Grp %d channel %d failed! Please check param\n", DpuGrp, 1);
                return s32Ret;
            }
        }

        s32Ret = CVI_DPU_StopGrp(DpuGrp);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "DPU Stop Grp %d failed! Please check param\n", DpuGrp);
            return CVI_FAILURE;
        }

        s32Ret = CVI_DPU_DestroyGrp(DpuGrp);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "DPU Destroy Grp %d failed! Please check\n", DpuGrp);
            return CVI_FAILURE;
        }
    }
    return s32Ret;
}

CVI_S32 app_ipcam_Dpu_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    Dpu_Flag = 1;
    for (CVI_U32 DpuGrp = 0; DpuGrp < g_pstDpuCfg->u32GrpCnt; DpuGrp ++) {
        if (!g_pstDpuCfg->stDpuGrpCfg[DpuGrp].bEnable)
            continue;

        s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
        if (s32Ret != CVI_SUCCESS)
            return s32Ret;

        APP_DPU_GRP_CFG_T *GrpCfg = &g_pstDpuCfg->stDpuGrpCfg[DpuGrp];
        s32Ret = pthread_create(&g_DPUThreadHandle[DpuGrp], NULL, Thread_DPU_PROC, GrpCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Dpu_pthread_create failed!, grp = %d\n", DpuGrp);
            return s32Ret;
        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_Dpu_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    Dpu_Flag = 0;
    for (CVI_U32 DpuGrp = 0; DpuGrp < g_pstDpuCfg->u32GrpCnt; DpuGrp ++) {
        if (!g_pstDpuCfg->stDpuGrpCfg[DpuGrp].bEnable)
            continue;

        s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
        if (s32Ret != CVI_SUCCESS)
            return s32Ret;

        pthread_join(g_DPUThreadHandle[DpuGrp], NULL);
    }
    return s32Ret;
}