/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_uvc_gadget.h
 * Description:
 *   UVC gadget
 */

#ifndef __CVI_UVC_GADGET_H__
#define __CVI_UVC_GADGET_H__

#include <sys/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t UVC_GADGET_UnInit(void *dev);

int32_t UVC_GADGET_Init(void);

int32_t UVC_GADGET_DeviceOpen(const char *pDevPath);

int32_t UVC_GADGET_DeviceClose(void);

int32_t UVC_GADGET_DeviceCheck(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_UVC_GADGET_H__ */