/*
 * 版权所有 (C) Cvitek Co., Ltd. 2023
 * 文件名称: app_ipcam_panel_i2c.h
 * 描述: 面板 I2C 初始化接口
 */

#ifndef __APP_IPCAM_PANEL_I2C_H__
#define __APP_IPCAM_PANEL_I2C_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "linux/cvi_type.h"
#include "linux/cvi_comm_vo.h"
#include "app_ipcam_panel.h"

CVI_S32 app_ipcam_Panel_I2c_Init(VO_DEV VoDev, const PANEL_I2C_CFG_T* const pstI2cCfg);
CVI_S32 app_ipcam_Panel_I2c_Exit(VO_DEV VoDev);
CVI_S32 app_ipcam_Panel_I2c_WriteReg(VO_DEV VoDev, CVI_U8 u8Addr, CVI_U8 u8Data,
    const PANEL_I2C_CFG_T* const pstI2cCfg);
CVI_S32 app_ipcam_Panel_I2c_SendInit(VO_DEV VoDev, const PANEL_TYPE_E enPanelType,
    const PANEL_I2C_CFG_T* const pstI2cCfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
