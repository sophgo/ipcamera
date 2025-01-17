#include "app_ipcam_comm.h"
#include "app_ipcam_sys.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_vpss.h"
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
APP_PARAM_GDC_CFG_T g_stGdcCfg;
APP_PARAM_GDC_CFG_T *g_pstGdcCfg = &g_stGdcCfg;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_GDC_CFG_T *app_ipcam_Gdc_Param_Get(void)
{
    return g_pstGdcCfg;
}

static inline CVI_VOID COMMON_GetPicBufferConfig(CVI_U32 u32Width, CVI_U32 u32Height,
	PIXEL_FORMAT_E enPixelFormat, DATA_BITWIDTH_E enBitWidth,
	COMPRESS_MODE_E enCmpMode, CVI_U32 u32Align, VB_CAL_CONFIG_S *pstCalConfig)
{
	CVI_U8  u8BitWidth = 0;
	CVI_U32 u32VBSize = 0;
	CVI_U32 u32AlignHeight = 0;
	CVI_U32 u32MainStride = 0;
	CVI_U32 u32CStride = 0;
	CVI_U32 u32MainSize = 0;
	CVI_U32 u32YSize = 0;
	CVI_U32 u32CSize = 0;

	/* u32Align: 0 is automatic mode, alignment size following system. Non-0 for specified alignment size */
	if (u32Align == 0)
		u32Align = DEFAULT_ALIGN;
	else if (u32Align > MAX_ALIGN)
		u32Align = MAX_ALIGN;

	switch (enBitWidth) {
	case DATA_BITWIDTH_8: {
		u8BitWidth = 8;
		break;
	}
	case DATA_BITWIDTH_10: {
		u8BitWidth = 10;
		break;
	}
	case DATA_BITWIDTH_12: {
		u8BitWidth = 12;
		break;
	}
	case DATA_BITWIDTH_14: {
		u8BitWidth = 14;
		break;
	}
	case DATA_BITWIDTH_16: {
		u8BitWidth = 16;
		break;
	}
	default: {
		u8BitWidth = 0;
		break;
	}
	}

	if ((enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
	 || (enPixelFormat == PIXEL_FORMAT_NV12)
	 || (enPixelFormat == PIXEL_FORMAT_NV21)) {
		u32AlignHeight = ALIGN(u32Height, 2);
	} else
		u32AlignHeight = u32Height;

	if (enCmpMode == COMPRESS_MODE_NONE) {
		u32MainStride = ALIGN((u32Width * u8BitWidth + 7) >> 3, u32Align);
		u32YSize = u32MainStride * u32AlignHeight;

		if (enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
			u32CStride = ALIGN(((u32Width >> 1) * u8BitWidth + 7) >> 3, u32Align);
			u32CSize = (u32CStride * u32AlignHeight) >> 1;

			u32MainStride = u32CStride * 2;
			u32YSize = u32MainStride * u32AlignHeight;
			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else if (enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
			u32CStride = ALIGN(((u32Width >> 1) * u8BitWidth + 7) >> 3, u32Align);
			u32CSize = u32CStride * u32AlignHeight;

			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else if (enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR ||
			   enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR ||
			   enPixelFormat == PIXEL_FORMAT_HSV_888_PLANAR ||
			   enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
			u32CStride = u32MainStride;
			u32CSize = u32YSize;

			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else if (enPixelFormat == PIXEL_FORMAT_RGB_BAYER_12BPP) {
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		} else if (enPixelFormat == PIXEL_FORMAT_YUV_400) {
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		} else if (enPixelFormat == PIXEL_FORMAT_NV12 || enPixelFormat == PIXEL_FORMAT_NV21) {
			u32CStride = ALIGN((u32Width * u8BitWidth + 7) >> 3, u32Align);
			u32CSize = (u32CStride * u32AlignHeight) >> 1;

			u32MainSize = u32YSize + u32CSize;
			pstCalConfig->plane_num = 2;
		} else if (enPixelFormat == PIXEL_FORMAT_NV16 || enPixelFormat == PIXEL_FORMAT_NV61) {
			u32CStride = ALIGN((u32Width * u8BitWidth + 7) >> 3, u32Align);
			u32CSize = u32CStride * u32AlignHeight;

			u32MainSize = u32YSize + u32CSize;
			pstCalConfig->plane_num = 2;
		} else if (enPixelFormat == PIXEL_FORMAT_YUYV || enPixelFormat == PIXEL_FORMAT_YVYU ||
			   enPixelFormat == PIXEL_FORMAT_UYVY || enPixelFormat == PIXEL_FORMAT_VYUY) {
			u32MainStride = ALIGN(((u32Width * u8BitWidth + 7) >> 3) * 2, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		} else if (enPixelFormat == PIXEL_FORMAT_ARGB_1555 || enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
			// packed format
			u32MainStride = ALIGN((u32Width * 16 + 7) >> 3, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		} else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
			// packed format
			u32MainStride = ALIGN((u32Width * 32 + 7) >> 3, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		} else if (enPixelFormat == PIXEL_FORMAT_FP32_C3_PLANAR) {
			u32MainStride = ALIGN(((u32Width * u8BitWidth + 7) >> 3) * 4, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32CStride = u32MainStride;
			u32CSize = u32YSize;
			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else if (enPixelFormat == PIXEL_FORMAT_FP16_C3_PLANAR ||
				enPixelFormat == PIXEL_FORMAT_BF16_C3_PLANAR) {
			u32MainStride = ALIGN(((u32Width * u8BitWidth + 7) >> 3) * 2, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32CStride = u32MainStride;
			u32CSize = u32YSize;
			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else if (enPixelFormat == PIXEL_FORMAT_INT8_C3_PLANAR ||
				enPixelFormat == PIXEL_FORMAT_UINT8_C3_PLANAR) {
			u32CStride = u32MainStride;
			u32CSize = u32YSize;
			u32MainSize = u32YSize + (u32CSize << 1);
			pstCalConfig->plane_num = 3;
		} else {
			// packed format
			u32MainStride = ALIGN(((u32Width * u8BitWidth + 7) >> 3) * 3, u32Align);
			u32YSize = u32MainStride * u32AlignHeight;
			u32MainSize = u32YSize;
			pstCalConfig->plane_num = 1;
		}

		u32VBSize = u32MainSize;
	} else {
		// TODO: compression mode
		pstCalConfig->plane_num = 0;
	}

	pstCalConfig->u32VBSize = u32VBSize;
	pstCalConfig->u32MainStride = u32MainStride;
	pstCalConfig->u32CStride = u32CStride;
	pstCalConfig->u32MainYSize = u32YSize;
	pstCalConfig->u32MainCSize = u32CSize;
	pstCalConfig->u32MainSize = u32MainSize;
	pstCalConfig->u16AddrAlign = u32Align;
}

CVI_S32 GDC_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
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

static CVI_S32 gdc_add_tsk(APP_GDC_CFG_T *pstGdcCfg)
{
	FISHEYE_ATTR_S *FisheyeAttr;
	AFFINE_ATTR_S *affineAttr;
	LDC_ATTR_S *LDCAttr;
	ROTATION_E enRotation;

	CVI_S32 s32Ret = CVI_FAILURE;

	if (!pstGdcCfg) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstGdcCfg is null ptr\n");
		return CVI_FAILURE;
	}

	switch (pstGdcCfg->u32Operation) {
	case GDC_FISHEYE:
		FisheyeAttr = &pstGdcCfg->FisheyeAttr;

		s32Ret = CVI_GDC_AddCorrectionTask(pstGdcCfg->hHandle, &pstGdcCfg->stTask, FisheyeAttr);
		if (s32Ret) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_AddCorrectionTask failed!\n");
		}
		break;
	case GDC_ROT:
		enRotation = pstGdcCfg->enRotation;

		s32Ret = CVI_GDC_AddRotationTask(pstGdcCfg->hHandle, &pstGdcCfg->stTask, enRotation);
		if (s32Ret) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_AddRotationTask failed!\n");
		}
		break;
	case GDC_LDC:
		LDCAttr = &pstGdcCfg->LdcAttr.stAttr;
		enRotation = (ROTATION_E)pstGdcCfg->stTask.reserved;

		s32Ret = CVI_GDC_AddLDCTask(pstGdcCfg->hHandle, &pstGdcCfg->stTask, LDCAttr, enRotation);
		if (s32Ret) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	case GDC_AFFINE:
		affineAttr = &pstGdcCfg->AffineAttr.stAffineAttr;

		s32Ret = CVI_GDC_AddAffineTask(pstGdcCfg->hHandle, &pstGdcCfg->stTask, affineAttr);
		if (s32Ret) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_AddAffineTask failed!\n");
		}
		break;
	default:
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "not allow this op(%d) fail\n", pstGdcCfg->u32Operation);
		break;
	}

	return s32Ret;
}

int app_ipcam_Gdc_SendFrame(APP_GDC_CFG_T *pstGdcCfg, VIDEO_FRAME_INFO_S *pstVideoFrame) {
    CVI_S32 s32Ret = CVI_FAILURE;
    SIZE_S size_out;
    PIXEL_FORMAT_E enPixelFormat;
    APP_GDC_CFG_T *pstgdccfg = pstGdcCfg;
    size_out.u32Width = pstGdcCfg->size_out.u32Width;
    size_out.u32Height = pstGdcCfg->size_out.u32Height;
    enPixelFormat = pstGdcCfg->enPixelFormat;
    memset(pstgdccfg->stTask.au64privateData, 0, sizeof(pstgdccfg->stTask.au64privateData));
	memcpy(&pstgdccfg->stTask.stImgIn, pstVideoFrame, sizeof(pstgdccfg->stTask.stImgIn));
    s32Ret = GDC_COMM_PrepareFrame(&size_out, enPixelFormat, &pstgdccfg->stTask.stImgOut);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "GDC_COMM_PrepareFrame failed!\n");
        return s32Ret;
    }
    s32Ret = CVI_GDC_BeginJob(&pstgdccfg->hHandle);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_BeginJob failed!\n");
        return s32Ret;
    }
    s32Ret = CVI_GDC_SetJobIdentity(pstGdcCfg->hHandle, &pstGdcCfg->identity);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_SetJobIdentity failed!\n");
        return s32Ret;
    }
    s32Ret = gdc_add_tsk(pstgdccfg);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "gdc_add_tsk. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }
    s32Ret = CVI_GDC_EndJob(pstGdcCfg->hHandle);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_EndJob failed!\n");
        return s32Ret;
    }

    return s32Ret;
}

int app_ipcam_Gdc_GetFrame(APP_GDC_CFG_T *pstGdcCfg, VIDEO_FRAME_INFO_S *pstVideoFrame) {
    CVI_S32 s32Ret = CVI_FAILURE;
    s32Ret = CVI_GDC_GetChnFrame(&pstGdcCfg->identity, pstVideoFrame, 5000);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_GetChnFrame failed!\n");
        return s32Ret;
    }

    return s32Ret;
}

int app_ipcam_Gdc_Frame_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame) {
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "open data file error\n");
		return CVI_FAILURE;
	}
	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		APP_PROF_LOG_PRINT(LEVEL_INFO, "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		APP_PROF_LOG_PRINT(LEVEL_INFO, " data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 GDCFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	CVI_U32 u32len;
	CVI_S32 Ret;
	int i;
	FILE *fp;

	if (!pstVideoFrame) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstVideoFrame is null\n");
		return CVI_FAILURE;
	}

	if (!filename) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "filename is null\n");
		return CVI_FAILURE;
	}

	Ret = GDC_COMM_PrepareFrame(stSize, enPixelFormat, pstVideoFrame);
	if (Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "GDC_COMM_PrepareFrame FAIL, get VB fail\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_PhysAddr2Handle(pstVideoFrame->stVFrame.u64PhyAddr[0]);

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "open data file[%s] error\n", filename);
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "file to frame: fread plane%d error\n", i);
			CVI_VB_ReleaseBlock(blk);
			Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	if (Ret)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);

	return Ret;
}

int app_ipcam_Gdc_SendFile(APP_GDC_CFG_T *pstGdcCfg) {
	CVI_S32 s32Ret = CVI_FAILURE;
	VIDEO_FRAME_INFO_S stInVideoFrame;
	memset(&stInVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = GDCFileToFrame(&pstGdcCfg->size_in, pstGdcCfg->enPixelFormat, pstGdcCfg->filename_in, &stInVideoFrame);
	if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "GDCFileToFrame failed!\n");
        return s32Ret;
    }
	s32Ret = app_ipcam_Gdc_SendFrame(pstGdcCfg, &stInVideoFrame);
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Gdc_SendFrame failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

int app_ipcam_Gdc_Init(void) {
    CVI_S32 s32Ret = CVI_FAILURE;
    CVI_CHAR filename[64];
	APP_PARAM_GDC_CFG_T *pastGdcCfg = app_ipcam_Gdc_Param_Get();
	if (pastGdcCfg->bUserEnable == 0) {
		return 0;
	}
    s32Ret = CVI_GDC_Init();
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_Init failed!\n");
		return s32Ret;
	}
    VIDEO_FRAME_INFO_S stInVideoFrame;
    VIDEO_FRAME_INFO_S stOutVideoFrame;

	// send frame 默认从 vpss 0 chn 0 get frame
    s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stInVideoFrame, 1000);
    s32Ret = app_ipcam_Gdc_SendFrame(&pastGdcCfg->astGdcCfg[0], &stInVideoFrame);

	// or send file, 二选一
	// s32Ret = app_ipcam_Gdc_SendFile(&pastGdcCfg->astGdcCfg[0]);
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Gdc_SendFrame failed!\n");
		return s32Ret;
	}
    s32Ret = app_ipcam_Gdc_GetFrame(&pastGdcCfg->astGdcCfg[0], &stOutVideoFrame);
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Gdc_GetFrame failed!\n");
		return s32Ret;
	}
    snprintf(filename, 64, pastGdcCfg->astGdcCfg[0].filename_out);
    s32Ret = app_ipcam_Gdc_Frame_SaveToFile(filename, &stOutVideoFrame);
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Gdc_SaveToFile failed!\n");
		return s32Ret;
	}
    return s32Ret;
}

int app_ipcam_Gdc_DeInit(void) {
    CVI_S32 s32Ret = CVI_FAILURE;
	APP_PARAM_GDC_CFG_T *pastGdcCfg = app_ipcam_Gdc_Param_Get();
	if (pastGdcCfg->bUserEnable == 0) {
		return 0;
	}
    s32Ret = CVI_GDC_DeInit();
    if (s32Ret != CVI_SUCCESS) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_GDC_DeInit failed!\n");
		return s32Ret;
	}

    return s32Ret;
}