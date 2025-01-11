#include <sys/stat.h>
#include <sys/prctl.h>
#include "stdlib.h"
#include "errno.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_blacklight.h"
#include "cvi_ae.h"
#include "cvi_sys.h"
#include "app_ipcam_gdc.h"
#include "app_ipcam_vpss.h"
#include "app_ipcam_venc.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define CNVMAPY_FILENAME "./cnvMapY.txt"
#define BLACKLIGHT_POOL_ID      0
#define ALIGN_SIZE              64 // depend on GDC HW
#define H26X_MAX_NUM_PACKS      8
#define LUT_SIZE                256
#define IMAGE_DEFAULT_VALUE     255
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_BLACK_LIGHT_CTX_S g_stBLCtx, *g_pstBLCtx = &g_stBLCtx;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
APP_PARAM_BLACK_LIGHT_CTX_S *app_ipcam_BlackLight_Param_Get(void)
{
    return g_pstBLCtx;
}

static CVI_S32 app_ipcam_BlackLight_Gen_Weight(IVE_HANDLE iveHandle
                                    , IVE_SRC_MEM_INFO_S *iveTable
                                    , VIDEO_FRAME_INFO_S *pstFrameIR
                                    , VIDEO_FRAME_INFO_S *pstFrameRGB
                                    , VIDEO_FRAME_INFO_S *pstFrameWeight)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    PIXEL_FORMAT_E enFmtTmp = PIXEL_FORMAT_MAX;
    IVE_SRC_IMAGE_S stIveImageSrc[2] = {0};
    IVE_DST_IMAGE_S stIveImageDst = {0};

    if (pstFrameIR == NULL || pstFrameRGB == NULL || pstFrameWeight == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Input param is invalid. \n");
        return CVI_FAILURE;
    }

    enFmtTmp = pstFrameIR->stVFrame.enPixelFormat;
    pstFrameIR->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_400;
    memset(&stIveImageSrc[0], 0, sizeof(IVE_SRC_IMAGE_S));
    s32Ret = CVI_IVE_VideoFrameInfo2Image(pstFrameIR, &stIveImageSrc[0]);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR
            , "CVI_IVE_VideoFrameInfo2Image failed with %#x\n", s32Ret);
        return s32Ret;
    }
    pstFrameIR->stVFrame.enPixelFormat = enFmtTmp;

    enFmtTmp = pstFrameRGB->stVFrame.enPixelFormat;
    pstFrameRGB->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_400;
    memset(&stIveImageSrc[1], 0, sizeof(IVE_SRC_IMAGE_S));
    s32Ret = CVI_IVE_VideoFrameInfo2Image(pstFrameRGB, &stIveImageSrc[1]);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR
            , "CVI_IVE_VideoFrameInfo2Image failed with %#x\n", s32Ret);
        return s32Ret;
    }
    pstFrameRGB->stVFrame.enPixelFormat = enFmtTmp;

    stIveImageDst.enType = IVE_IMAGE_TYPE_U8C1;
    stIveImageDst.u64VirAddr[0] = (CVI_U64)(uintptr_t)pstFrameWeight->stVFrame.pu8VirAddr[0];
    stIveImageDst.u64PhyAddr[0] = pstFrameWeight->stVFrame.u64PhyAddr[0];
    stIveImageDst.u32Stride[0]  = pstFrameWeight->stVFrame.u32Stride[0];
    stIveImageDst.u32Width      = pstFrameWeight->stVFrame.u32Width;
    stIveImageDst.u32Height     = pstFrameWeight->stVFrame.u32Height;

    IVE_MAP_CTRL_S mapCtrl = {0};
	mapCtrl.enMode = IVE_MAP_MODE_U8;
    s32Ret = CVI_IVE_Map(iveHandle, &stIveImageSrc[1], iveTable, &stIveImageDst, &mapCtrl, CVI_FALSE);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_IVE_Map failed with %#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static void *Thread_Black_Light_Proc(void *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VB_BLK blk;
    ISP_EXP_INFO_S stExpInfo = {0};
    VB_CAL_CONFIG_S stVbCalConfig = {0};
    VIDEO_FRAME_INFO_S stFrameIR = {0};
    VIDEO_FRAME_INFO_S stFrameRGB = {0};
    VIDEO_FRAME_INFO_S stFrmaeWeight = {0};

    APP_PARAM_BLACK_LIGHT_CTX_S *pstBlackLightCtx = (APP_PARAM_BLACK_LIGHT_CTX_S *)pArgs;

    CVI_CHAR TaskName[64] = {'\0'};
    sprintf(TaskName, "Thread_Black_Light_Proc");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Black light start running. \n");

    COMMON_GetPicBufferConfig(pstBlackLightCtx->frameWidth
                            , pstBlackLightCtx->frameHeight
                            , PIXEL_FORMAT_NV21, DATA_BITWIDTH_8
                            , COMPRESS_MODE_NONE, 1, &stVbCalConfig);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Get block >> w:%d, h:%d, size:%d. \n"
		, pstBlackLightCtx->frameWidth, pstBlackLightCtx->frameHeight, stVbCalConfig.u32VBSize);
    blk = CVI_VB_GetBlock(BLACKLIGHT_POOL_ID, stVbCalConfig.u32VBSize);
    if (blk == VB_INVALID_HANDLE) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VB_GetBlock(size:%d) fail\n"
            , stVbCalConfig.u32VBSize)
        goto EXIT1;
    }

    stFrmaeWeight.u32PoolId = BLACKLIGHT_POOL_ID;
    stFrmaeWeight.stVFrame.enPixelFormat = PIXEL_FORMAT_NV21;
    stFrmaeWeight.stVFrame.u32Width = pstBlackLightCtx->frameWidth;
    stFrmaeWeight.stVFrame.u32Height = pstBlackLightCtx->frameHeight;
    stFrmaeWeight.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
    stFrmaeWeight.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
    stFrmaeWeight.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
    stFrmaeWeight.stVFrame.u32Length[0] = ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
    stFrmaeWeight.stVFrame.u32Length[1] = ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
    stFrmaeWeight.stVFrame.u32Length[2] = ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
    stFrmaeWeight.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
    stFrmaeWeight.stVFrame.u64PhyAddr[1] = stFrmaeWeight.stVFrame.u64PhyAddr[0] + stFrmaeWeight.stVFrame.u32Length[0];
    stFrmaeWeight.stVFrame.u64PhyAddr[2] = stFrmaeWeight.stVFrame.u64PhyAddr[1] + stFrmaeWeight.stVFrame.u32Length[1];
    for (int plane_idx = 0; plane_idx < 3; plane_idx++) {
		stFrmaeWeight.stVFrame.pu8VirAddr[plane_idx] =
			CVI_SYS_Mmap(stFrmaeWeight.stVFrame.u64PhyAddr[plane_idx], stFrmaeWeight.stVFrame.u32Length[plane_idx]);
    }

	while (pstBlackLightCtx->bStart) {
		memset(&stExpInfo, 0, sizeof(stExpInfo));
		s32Ret = CVI_ISP_QueryExposureInfo(0, &stExpInfo);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ISP_QueryExposureInfo failed with %#x\n", s32Ret)
			break;
		}

		if (access("/tmp/bl_force_en", F_OK) == 0) {
			stExpInfo.u32ISO = 60000;
		}

		if (stExpInfo.u32ISO < 30000) {
			pstBlackLightCtx->bBlackLight = CVI_FALSE;
            usleep(1000);
			continue;
		}

		pstBlackLightCtx->bBlackLight = CVI_TRUE;

		s32Ret = CVI_VPSS_GetChnFrame(pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR, &stFrameIR, 3000);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) CVI_VPSS_GetChnFrame failed with %#x\n"
				, pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR, s32Ret);
			continue;
		}

		s32Ret = CVI_VPSS_GetChnFrame(pstBlackLightCtx->VpssGrpRGB, pstBlackLightCtx->VpssChnRGB, &stFrameRGB, 3000);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) CVI_VPSS_GetChnFrame failed with %#x\n"
				, pstBlackLightCtx->VpssGrpRGB, pstBlackLightCtx->VpssChnRGB, s32Ret);
			CVI_VPSS_ReleaseChnFrame(pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR, &stFrameIR);
			continue;
		}

		stFrameIR.stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(stFrameIR.stVFrame.u64PhyAddr[0], stFrameIR.stVFrame.u32Length[0]);
		stFrameRGB.stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(stFrameRGB.stVFrame.u64PhyAddr[0], stFrameRGB.stVFrame.u32Length[0]);

		// only use Y component
		s32Ret = app_ipcam_BlackLight_Gen_Weight(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->iveTable
                                                , &stFrameIR, &stFrameRGB, &stFrmaeWeight);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR
				, "app_ipcam_BlackLight_Gen_Weight failed with %#x\n", s32Ret);
			goto EXIT2;
		}

		CVI_U32 IRLen = stFrameIR.stVFrame.u32Length[0] + stFrameIR.stVFrame.u32Length[1] + stFrameIR.stVFrame.u32Length[2];
		CVI_U32 RGBLen = stFrameRGB.stVFrame.u32Length[0] + stFrameRGB.stVFrame.u32Length[1] + stFrameRGB.stVFrame.u32Length[2];
		CVI_U32 WeightLen = stFrmaeWeight.stVFrame.u32Length[0] + stFrmaeWeight.stVFrame.u32Length[1] + stFrmaeWeight.stVFrame.u32Length[2];
		APP_PROF_LOG_PRINT(LEVEL_DEBUG, "s1>> w:%d,h:%d,size:%d. s2>> w:%d,h:%d,size:%d. alpha>> w:%d,h:%d,size:%d.\n"
			, stFrameIR.stVFrame.u32Width, stFrameIR.stVFrame.u32Height, IRLen
			, stFrameRGB.stVFrame.u32Width, stFrameRGB.stVFrame.u32Height, RGBLen
			, stFrmaeWeight.stVFrame.u32Width, stFrmaeWeight.stVFrame.u32Height, WeightLen);
		s32Ret = CVI_IVE_Blend_Pixel_Y(pstBlackLightCtx->tpuIveHandle, &stFrameIR, &stFrameRGB, &stFrmaeWeight);//ir rgb weight
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) and Grp(%d)-Chn(%d) CVI_IVE_Blend_Pixel_Y failed with %#x\n"
				, pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR
				, pstBlackLightCtx->VpssGrpRGB, pstBlackLightCtx->VpssChnRGB
				, s32Ret);
			goto EXIT2;
		}

		CVI_SYS_Munmap(stFrameIR.stVFrame.pu8VirAddr[0], stFrameIR.stVFrame.u32Length[0]);
		CVI_SYS_Munmap(stFrameRGB.stVFrame.pu8VirAddr[0], stFrameRGB.stVFrame.u32Length[0]);

		s32Ret = CVI_VPSS_SendFrame(pstBlackLightCtx->VpssGrpBlackLight, &stFrameRGB, -1);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d) CVI_VPSS_SendFrame failed with %#x\n", pstBlackLightCtx->VpssGrpBlackLight, s32Ret);
		}
EXIT2:
		s32Ret = CVI_VPSS_ReleaseChnFrame(pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR, &stFrameIR);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) CVI_VPSS_ReleaseChnFrame failed with %#x\n"
				, pstBlackLightCtx->VpssGrpIR, pstBlackLightCtx->VpssChnIR, s32Ret);
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(pstBlackLightCtx->VpssGrpRGB, pstBlackLightCtx->VpssChnRGB, &stFrameRGB);
		if (s32Ret != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) CVI_VPSS_ReleaseChnFrame failed with %#x\n"
				, pstBlackLightCtx->VpssGrpRGB, pstBlackLightCtx->VpssChnRGB, s32Ret);
		}
	}

EXIT1:
    for (int plane_idx = 0; plane_idx < 3; plane_idx++) {
		CVI_SYS_Munmap(stFrmaeWeight.stVFrame.pu8VirAddr[plane_idx], stFrmaeWeight.stVFrame.u32Length[plane_idx]);
    }
    if (blk != VB_INVALID_HANDLE)
        CVI_VB_ReleaseBlock(blk);

    return (CVI_VOID *) CVI_SUCCESS;
}

int app_ipcam_BlackLight_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PARAM_BLACK_LIGHT_CTX_S *pstBlackLightCtx = NULL;
    APP_PARAM_VPSS_CFG_T *pstVpssCtx = NULL;
    APP_PARAM_GDC_CFG_S *pstGDCCtx = NULL;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_BlackLight_Init start. \n");

    pstBlackLightCtx = app_ipcam_BlackLight_Param_Get();
    pstVpssCtx = app_ipcam_Vpss_Param_Get();
    pstGDCCtx = app_ipcam_GDC_Param_Get();

    pstBlackLightCtx->frameWidth =
        pstVpssCtx->astVpssGrpCfg[pstBlackLightCtx->VpssGrpRGB].astVpssChnAttr[pstBlackLightCtx->VpssChnRGB].u32Width;
    pstBlackLightCtx->frameHeight =
        pstVpssCtx->astVpssGrpCfg[pstBlackLightCtx->VpssGrpRGB].astVpssChnAttr[pstBlackLightCtx->VpssChnRGB].u32Height;

    if (pstGDCCtx->stGDCVpssCfg[pstBlackLightCtx->VpssGrpRGB].bEnable) {
        pstBlackLightCtx->frameWidth = ALIGN(pstBlackLightCtx->frameWidth, ALIGN_SIZE);
        pstBlackLightCtx->frameHeight = ALIGN(pstBlackLightCtx->frameHeight, ALIGN_SIZE);
    }

    pstBlackLightCtx->iveHandle = NULL;
    pstBlackLightCtx->iveHandle = CVI_IVE_CreateHandle();
    if (pstBlackLightCtx->iveHandle == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "iveHandle is null.\n");
        return CVI_FAILURE;
    }

    pstBlackLightCtx->tpuIveHandle = NULL;
    pstBlackLightCtx->tpuIveHandle = CVI_TPU_IVE_CreateHandle();
    if (pstBlackLightCtx->tpuIveHandle == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "tpuIveHandle is null.\n");
        goto EXIT;
    }

    /**
     * Read cnvMapY.txt and request memory for it
     */
    CVI_U8 lutAlpha[LUT_SIZE];
    FILE *fpmap = fopen(CNVMAPY_FILENAME, "r");
    if (fpmap == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Open %s failed. \n", CNVMAPY_FILENAME);
        goto EXIT;
    }
    for(int i = 0; i < LUT_SIZE; i++) {
        fscanf(fpmap, "%hhu", &lutAlpha[i]);
    }
    fclose(fpmap);

    s32Ret = CVI_IVE_CreateImage(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->imageMap, IVE_IMAGE_TYPE_U8C1
                                , pstBlackLightCtx->frameWidth, pstBlackLightCtx->frameHeight);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_IVE_CreateImage failed. errCode:%d.\n", s32Ret);
        goto EXIT;
    }
    s32Ret = CVI_IVE_ResetImage(&pstBlackLightCtx->imageMap, IMAGE_DEFAULT_VALUE);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_IVE_ResetImage failed. errCode:%d.\n", s32Ret);
        goto EXIT;
    }
    pstBlackLightCtx->iveTable.u32Size = LUT_SIZE * sizeof(CVI_U8);
    s32Ret = CVI_IVE_CreateMemInfo(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->iveTable, pstBlackLightCtx->iveTable.u32Size);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_IVE_CreateMemInfo failed. errCode:%d.\n", s32Ret);
        goto EXIT;
    }
    for (CVI_U32 i = 0; i < LUT_SIZE; i++) {
        ((CVI_U8 *)(uintptr_t)pstBlackLightCtx->iveTable.u64VirAddr)[i] = (CVI_U8)lutAlpha[i];
    }

    pstBlackLightCtx->bStart = CVI_TRUE;
	pstBlackLightCtx->bBlackLight = CVI_FALSE;

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 80;
    pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    s32Ret = pthread_create(&pstBlackLightCtx->blackLightThread, &attr, Thread_Black_Light_Proc, (CVI_VOID *)pstBlackLightCtx);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create failed:0x%x\n", s32Ret);
        goto EXIT;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_BlackLight_Init done. \n");

    return s32Ret;
EXIT:
    if (pstBlackLightCtx->iveTable.u64VirAddr != 0x0)
        CVI_SYS_FreeM(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->iveTable);
    if (pstBlackLightCtx->imageMap.u64VirAddr[0] != 0x0)
        CVI_SYS_FreeI(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->imageMap);
    if (pstBlackLightCtx->tpuIveHandle)
        CVI_TPU_IVE_DestroyHandle(pstBlackLightCtx->tpuIveHandle);
    if (pstBlackLightCtx->iveHandle)
        CVI_IVE_DestroyHandle(pstBlackLightCtx->iveHandle);

    pstBlackLightCtx->iveTable.u64VirAddr = 0x0;
    pstBlackLightCtx->imageMap.u64VirAddr[0] = 0x0;
    pstBlackLightCtx->tpuIveHandle = NULL;
    pstBlackLightCtx->iveHandle = NULL;

    return s32Ret;
}

int app_ipcam_BlackLight_DeInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PARAM_BLACK_LIGHT_CTX_S *pstBlackLightCtx = NULL;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_BlackLight_DeInit start. \n");

    pstBlackLightCtx = app_ipcam_BlackLight_Param_Get();

    pstBlackLightCtx->bStart = CVI_FALSE;
    if (pstBlackLightCtx->blackLightThread != 0) {
            pthread_join(pstBlackLightCtx->blackLightThread, CVI_NULL);
            APP_PROF_LOG_PRINT(LEVEL_WARN, "Black Light Proc done \n");
            pstBlackLightCtx->blackLightThread = 0;
    }

    if (pstBlackLightCtx->iveTable.u64VirAddr != 0x0)
        CVI_SYS_FreeM(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->iveTable);
    if (pstBlackLightCtx->imageMap.u64VirAddr[0] != 0x0)
        CVI_SYS_FreeI(pstBlackLightCtx->iveHandle, &pstBlackLightCtx->imageMap);
    if (pstBlackLightCtx->tpuIveHandle)
        CVI_TPU_IVE_DestroyHandle(pstBlackLightCtx->tpuIveHandle);
    if (pstBlackLightCtx->iveHandle)
        CVI_IVE_DestroyHandle(pstBlackLightCtx->iveHandle);

    pstBlackLightCtx->iveTable.u64VirAddr = 0x0;
    pstBlackLightCtx->imageMap.u64VirAddr[0] = 0x0;
    pstBlackLightCtx->tpuIveHandle = NULL;
    pstBlackLightCtx->iveHandle = NULL;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_BlackLight_DeInit done. \n");

    return s32Ret;
}
