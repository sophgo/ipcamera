#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>

#include "cvi_mipi_tx.h"
#include "cvi_vo.h"

#include "dsi_hx8394_evb.h"
#include "dsi_ili9881c.h"
#include "dsi_ili9881d.h"
#include "dsi_jd9366ab.h"
#include "dsi_nt35521.h"
#include "dsi_ota7290b.h"
#include "dsi_ota7290b_1920.h"
#include "dsi_icn9707.h"
#include "dsi_3aml069lp01g.h"
#include "dsi_st7701.h"
#include "dsi_hx8399_1080p.h"
#include "dsi_gm8775c.h"
#include "dsi_lt9611.h"
#include "lvds_lcm185x56.h"
#include "hw_mcu_st7789v3.h"
#include "bt656_pt1000k.h"
#include "bt1120_pt1000k.h"
#include "bt656_tp2803.h"
#include "bt1120_nvp6021.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_panel.h"

const char* panel_type_name[PANEL_MAX] = {
    [DSI_PANEL_3AML069LP01G]                      = "DSI_PANEL_3AML069LP01G",
    [DSI_PANEL_GM8775C]                           = "DSI_PANEL_GM8775C",
    [DSI_PANEL_HX8394_EVB]                        = "DSI_PANEL_HX8394_EVB",
    [DSI_PANEL_HX8399_1080P]                      = "DSI_PANEL_HX8399_1080P",
    [DSI_PANEL_ICN9707]                           = "DSI_PANEL_ICN9707",
    [DSI_PANEL_ILI9881C]                          = "DSI_PANEL_ILI9881C",
    [DSI_PANEL_ILI9881D]                          = "DSI_PANEL_ILI9881D",
    [DSI_PANEL_JD9366AB]                          = "DSI_PANEL_JD9366AB",
    [DSI_PANEL_LT9611_1920x1080_60]               = "DSI_PANEL_LT9611_1920x1080_60",
    [DSI_PANEL_LT9611_1920x1080_30]               = "DSI_PANEL_LT9611_1920x1080_30",
    [DSI_PANEL_LT9611_1280x720_60]                = "DSI_PANEL_LT9611_1280x720_60",
    [DSI_PANEL_LT9611_1024x768_60]                = "DSI_PANEL_LT9611_1024x768_60",
    [DSI_PANEL_LT9611_1280x1024_60]               = "DSI_PANEL_LT9611_1280x1024_60",
    [DSI_PANEL_LT9611_1600x1200_60]               = "DSI_PANEL_LT9611_1600x1200_60",
    [DSI_PANEL_NT35521]                           = "DSI_PANEL_NT35521",
    [DSI_PANEL_OTA7290B_1920]                     = "DSI_PANEL_OTA7290B_1920",
    [DSI_PANEL_OTA7290B]                          = "DSI_PANEL_OTA7290B",
    [DSI_PANEL_ST7701]                            = "DSI_PANEL_ST7701",
    [LVDS_PANEL_LCM185X56]                        = "LDVS_PANEL_LCM185X56",
    [BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M]   = "BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M",
    [BT_PANEL_PT1000K_BT656_1920x1080_30FPS_148M] = "BT_PANEL_PT1000K_BT656_1920x1080_30FPS_148M",
    [BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M] = "BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M",
    [BT_PANEL_TP2803_BT656_1280x720_25FPS_72M]    = "BT_PANEL_TP2803_BT656_1280x720_25FPS_72M",
    [BT_PANEL_NVP6021_BT1120_1920x1080_25FPS_72M] = "BT_PANEL_NVP6021_BT1120_1920x1080_25FPS_72M",
    [I80_PANEL_ST7789V3_HW_MCU_240x320_60FPS]     = "I80_PANEL_ST7789V3_HW_MCU_RGB565_240x320_60FPS",
};

// static PANEL_DESC_S g_st_panel_cfg = {
// 	.panel_num = 1,
// 	.panel_interface = PANEL_IF_DSI,
// 	.panel_type = DSI_PANEL_HX8394_EVB,
// 	.dsi_cfg.dev_cfg = &dev_cfg_hx8394_720x1280,
// 	.dsi_cfg.hs_timing_cfg = &hs_timing_cfg_hx8394_720x1280,
// 	.dsi_cfg.dsi_init_cmds = dsi_init_cmds_hx8394_720x1280,
// 	.dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280),
// };

PANEL_DESC_S g_st_panel_cfg, *g_pst_panel_cfg = &g_st_panel_cfg;

PANEL_DESC_S *app_ipcam_panel_param_get(void)
{
    return g_pst_panel_cfg;
}

CVI_S32 app_ipcam_panel_panel_desc_get(PANEL_DESC_S* const pst_panel_desc)
{
    _NULL_POINTER_CHECK_(pst_panel_desc, CVI_FAILURE);

	switch (pst_panel_desc->panel_type) {
	case DSI_PANEL_ILI9881C:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_ili9881c_720x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_ili9881c_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_ili9881c_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ili9881c_720x1280);
		break;
	case DSI_PANEL_ILI9881D:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_ili9881d_720x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_ili9881d_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_ili9881d_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ili9881d_720x1280);
		break;
	case DSI_PANEL_JD9366AB:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_jd9366ab_800x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_jd9366ab_800x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_jd9366ab_800x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_jd9366ab_800x1280);
		break;
	case DSI_PANEL_NT35521:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_nt35521_800x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_nt35521_800x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_nt35521_800x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_nt35521_800x1280);
		break;
	case DSI_PANEL_OTA7290B:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_ota7290b_320x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_ota7290b_320x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_ota7290b_320x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ota7290b_320x1280);
		break;
	case DSI_PANEL_OTA7290B_1920:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_ota7290b_440x1920;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_ota7290b_440x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_ota7290b_440x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ota7290b_440x1920);
		break;
	case DSI_PANEL_ICN9707:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_icn9707_480x1920;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_icn9707_480x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_icn9707_480x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_icn9707_480x1920);
		break;
	case DSI_PANEL_3AML069LP01G:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_3AML069LP01G_600x1024;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_3AML069LP01G_600x1024;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_3AML069LP01G_600x1024;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_3AML069LP01G_600x1024);
		break;
	case DSI_PANEL_ST7701:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_st7701_480x800;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_st7701_480x800;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_st7701_480x800;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_st7701_480x800);
		break;
	case DSI_PANEL_HX8399_1080P:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_hx8399_1080x1920;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_hx8399_1080x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_hx8399_1080x1920;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8399_1080x1920);
		break;
	case DSI_PANEL_GM8775C:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_gm8775c;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_gm8775c;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_gm8775c;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_gm8775c);
		break;
	case DSI_PANEL_LT9611_1920x1080_60:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1920x1080_60Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1920x1080_30:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1920x1080_30Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1280x720_60:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1280x720_60Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1024x768_60:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1024x768_60Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1280x1024_60:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1280x1024_60Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1600x1200_60:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_lt9611_1600x1200_60Hz;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = NULL;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_HX8394_EVB:
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280);
		break;
	case LVDS_PANEL_LCM185X56:
		pst_panel_desc->panel_interface = PANEL_IF_LVDS;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_LCD_24BIT;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stLcm185x56_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 60
		, .u16Vact = 768, .u16Vbb = 20, .u16Vfb = 10
		, .u16Hact = 1366, .u16Hbb = 100, .u16Hfb = 88
		, .u16Vpw = 2, .u16Hpw = 20, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stLcm185x56_SyncInfo;
		pst_panel_desc->vo_pub_attr.stLvdsAttr = lvds_lcm185x56_cfg;
		break;
	case BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M:
		pst_panel_desc->panel_interface = PANEL_IF_BT;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_BT656;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stPt1000kbt656_720p_25fps_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 720, .u16Vbb = 20, .u16Vfb = 5
		, .u16Hact = 1280, .u16Hbb = 220, .u16Hfb = 440
		, .u16Vpw = 5, .u16Hpw = 40, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stPt1000kbt656_720p_25fps_SyncInfo;
		pst_panel_desc->vo_pub_attr.stBtAttr = stpt1000kbt656cfg;
		break;
	case BT_PANEL_PT1000K_BT656_1920x1080_30FPS_148M:
		pst_panel_desc->panel_interface = PANEL_IF_BT;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_BT656;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stPt1000kbt656_1080p_30fps_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 30
		, .u16Vact = 1080, .u16Vbb = 20, .u16Vfb = 20
		, .u16Hact = 1920, .u16Hbb = 120, .u16Hfb = 120
		, .u16Vpw = 5, .u16Hpw = 40, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stPt1000kbt656_1080p_30fps_SyncInfo;
		pst_panel_desc->vo_pub_attr.stBtAttr = stpt1000kbt656cfg;
		break;
	case BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M:
		pst_panel_desc->panel_interface = PANEL_IF_BT;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_BT1120;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stPt1000kbt1120_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 1080, .u16Vbb = 20, .u16Vfb = 20
		, .u16Hact = 1920, .u16Hbb = 356, .u16Hfb = 356
		, .u16Vpw = 5, .u16Hpw = 8, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stPt1000kbt1120_SyncInfo;
		pst_panel_desc->vo_pub_attr.stBtAttr = stpt1000kbt1120cfg;
		break;
	case BT_PANEL_TP2803_BT656_1280x720_25FPS_72M:
		pst_panel_desc->panel_interface = PANEL_IF_BT;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_BT656;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stTp2803_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 720, .u16Vbb = 20, .u16Vfb = 5
		, .u16Hact = 1280, .u16Hbb = 200, .u16Hfb = 400
		, .u16Vpw = 5, .u16Hpw = 40, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stTp2803_SyncInfo;
		pst_panel_desc->vo_pub_attr.stBtAttr = stTP2803Cfg;
		break;
	case BT_PANEL_NVP6021_BT1120_1920x1080_25FPS_72M:
		pst_panel_desc->panel_interface = PANEL_IF_BT;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_BT1120;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stNvp6021_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 1080, .u16Vbb = 36, .u16Vfb = 4
		, .u16Hact = 1920, .u16Hbb = 148, .u16Hfb = 528
		, .u16Vpw = 5, .u16Hpw = 44, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		pst_panel_desc->vo_pub_attr.stSyncInfo = stNvp6021_SyncInfo;
		pst_panel_desc->vo_pub_attr.stBtAttr = stNVP6021Cfg;
		break;
	case I80_PANEL_ST7789V3_HW_MCU_240x320_60FPS:
		pst_panel_desc->panel_interface = PANEL_IF_MCU;
		pst_panel_desc->vo_pub_attr.enIntfType = VO_INTF_HW_MCU;
		pst_panel_desc->vo_pub_attr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S st7789V3_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 60
		, .u16Vact = 320, .u16Vbb = 0, .u16Vfb = 32
		, .u16Hact = 240, .u16Hbb = 0, .u16Hfb = 16
		, .u16Vpw = 2, .u16Hpw = 2, .bIdv = 0, .bIhs = 1, .bIvs = 1};
		pst_panel_desc->vo_pub_attr.stSyncInfo = st7789V3_SyncInfo;
		pst_panel_desc->vo_pub_attr.stMcuCfg = st7789v3Cfg;
		break;
	default:
		printf("default\n");
		pst_panel_desc->panel_interface = PANEL_IF_DSI;
		pst_panel_desc->dsi_cfg.dev_cfg = &dev_cfg_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.hs_timing_cfg = &hs_timing_cfg_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds = dsi_init_cmds_hx8394_720x1280;
		pst_panel_desc->dsi_cfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 app_ipcam_panel_mipitx_enable(const CVI_U32 dev_num, PANEL_DESC_S* const pst_panel_desc)
{
    CVI_S32 ret = CVI_FAILURE;

    _NULL_POINTER_CHECK_(pst_panel_desc, CVI_FAILURE);

    pst_panel_desc->mipi_tx_fd[dev_num] = open(MIPI_TX_NAME, O_RDWR | O_NONBLOCK, 0);
    if (pst_panel_desc->mipi_tx_fd[dev_num] < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Cannot open '%s': %d, %s\n", MIPI_TX_NAME, errno, strerror(errno));
        return CVI_FAILURE;
    }

	DSI_CONFIG_DESC_S *pst_dsi_cfg = (DSI_CONFIG_DESC_S *)&pst_panel_desc->dsi_cfg;

    // 配置 MIPI TX
    ret = CVI_MIPI_TX_Cfg(pst_panel_desc->mipi_tx_fd[dev_num],
                          (struct combo_dev_cfg_s *)pst_dsi_cfg->dev_cfg);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MIPI_TX_Cfg failed: %d\n", ret);
        goto fail_close_fd;
    }

    // 发送初始化命令序列
    for (CVI_S32 i = 0; i < pst_dsi_cfg->dsi_init_cmds_size; i++) {
        if (pst_dsi_cfg->dsi_init_cmds[i].size > 0 &&
            pst_dsi_cfg->dsi_init_cmds[i].data == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Null cmd data at %d with non-zero size\n", i);
            ret = CVI_FAILURE;
            goto fail_close_fd;
        }

        struct cmd_info_s stCmdInfo = {
            .devno     = dev_num,
            .cmd_size  = pst_dsi_cfg->dsi_init_cmds[i].size,
            .data_type = pst_dsi_cfg->dsi_init_cmds[i].data_type,
            .cmd       = (void *)pst_dsi_cfg->dsi_init_cmds[i].data
        };

        ret = CVI_MIPI_TX_SendCmd(pst_panel_desc->mipi_tx_fd[dev_num], &stCmdInfo);

        if (pst_dsi_cfg->dsi_init_cmds[i].delay) {
            CVI_U32 us = pst_dsi_cfg->dsi_init_cmds[i].delay * 1000UL;
            usleep(us);
        }

        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "[mipi_tx_send_cmd] didn't return success at cmd %d (type=0x%x, size=%d)\n",
                               i,
                               pst_dsi_cfg->dsi_init_cmds[i].data_type,
                               pst_dsi_cfg->dsi_init_cmds[i].size);
            goto fail_close_fd;
        }
    }

    // 设置 HS timing
    ret = CVI_MIPI_TX_SetHsSettle(pst_panel_desc->mipi_tx_fd[dev_num], pst_dsi_cfg->hs_timing_cfg);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MIPI_TX_SetHsSettle failed: %d\n", ret);
        goto fail_close_fd;
    }

    // 使能 MIPI TX
    ret = CVI_MIPI_TX_Enable(pst_panel_desc->mipi_tx_fd[dev_num]);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MIPI_TX_Enable failed: %d\n", ret);
        goto fail_close_fd;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Enable MIPI-TX driver for panel (%d)[%s].\n",
                       pst_panel_desc->panel_type,
					   panel_type_name[pst_panel_desc->panel_type]);

    return CVI_SUCCESS;

fail_close_fd:
    if (pst_panel_desc->mipi_tx_fd[dev_num] >= 0) {
        close(pst_panel_desc->mipi_tx_fd[dev_num]);
        pst_panel_desc->mipi_tx_fd[dev_num] = -1;
    }
    return CVI_FAILURE;
}

CVI_S32 app_ipcam_panel_mipitx_disable(const CVI_S32 mipi_tx_fd)
{

    if (mipi_tx_fd < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid MIPI-TX fd (%d) for disable.\n", mipi_tx_fd);
        return CVI_FAILURE;
    }

    CVI_S32 ret = CVI_MIPI_TX_Disable(mipi_tx_fd);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MIPI_TX_Disable failed: %d\n", ret);
        return ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "MIPI-TX driver disabled.\n");
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_panel_init(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================== Panel Init Start ========================\n");
	CVI_S32 ret = CVI_SUCCESS;
    PANEL_DESC_S *pst_panel_descrition = app_ipcam_panel_param_get();
    CVI_U32 panel_num = g_pst_panel_cfg->panel_num;
    while(panel_num > 0)
    {
        panel_num--;
        app_ipcam_panel_panel_desc_get(pst_panel_descrition);
		if(g_pst_panel_cfg->panel_interface == PANEL_IF_DSI){
			ret = app_ipcam_panel_mipitx_enable(panel_num, pst_panel_descrition);
			if (ret != CVI_SUCCESS) {
				printf("app_ipcam_panel_mipitx_enable fail!\n");
				return CVI_FAILURE;
			}
		} else{
			ret = CVI_VO_SetPubAttr(panel_num, &g_pst_panel_cfg->vo_pub_attr);
			if (ret != CVI_SUCCESS) {
				printf("CVI_VO_SetPubAttr failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
		}
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================== Panel Init End. ========================\n");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_panel_uninit(void)
{
	CVI_S32 ret = CVI_SUCCESS;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================== Panel Uninit Start ========================\n");
    for (CVI_U32 i = 0; i < g_pst_panel_cfg->panel_num; i++) {
        ret = app_ipcam_panel_mipitx_disable(g_pst_panel_cfg->mipi_tx_fd[i]);
		if(ret != CVI_SUCCESS){
			printf("app_ipcam_panel_mipitx_disable failed!\n");
		}
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================== Panel Uninit End. ========================\n");

    return CVI_SUCCESS;
}