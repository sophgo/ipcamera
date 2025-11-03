#ifndef __APP_IPCAM_PANEL_H__
#define __APP_IPCAM_PANEL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "linux/cvi_type.h"
#include "linux/cvi_comm_vo.h"

#define APP_IPCAM_VO_MAX_NUM (VO_MAX_DEV_NUM)

typedef enum _panel_interface_e {
	PANEL_IF_DSI,
	PANEL_IF_LVDS,
	PANEL_IF_BT,
	PANEL_IF_MCU,
    PANEL_IF_MUX,
} PANEL_INTERFACE_E;

typedef enum _panel_type_e {
    DSI_PANEL_3AML069LP01G,
    DSI_PANEL_GM8775C,
    DSI_PANEL_HX8394_EVB,
    DSI_PANEL_HX8399_1080P,
    DSI_PANEL_ICN9707,
    DSI_PANEL_ILI9881C,
    DSI_PANEL_ILI9881D,
    DSI_PANEL_JD9366AB,
    DSI_PANEL_LT9611_1920x1080_60,
    DSI_PANEL_LT9611_1920x1080_30,
    DSI_PANEL_LT9611_1280x720_60,
    DSI_PANEL_LT9611_1024x768_60,
    DSI_PANEL_LT9611_1280x1024_60,
    DSI_PANEL_LT9611_1600x1200_60,
    DSI_PANEL_NT35521,
    DSI_PANEL_OTA7290B_1920,
    DSI_PANEL_OTA7290B,
    DSI_PANEL_ST7701,
    LVDS_PANEL_LCM185X56,
    BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M,
    BT_PANEL_PT1000K_BT656_1920x1080_30FPS_148M,
    BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M,
    BT_PANEL_TP2803_BT656_1280x720_25FPS_72M,
    BT_PANEL_NVP6021_BT1120_1920x1080_25FPS_72M,
    I80_PANEL_ST7789V3_HW_MCU_240x320_60FPS,
    PANEL_MAX
} PANEL_TYPE_E;
extern const char* panel_type_name[PANEL_MAX];

typedef struct _dsi_config_desc_s {
	struct combo_dev_cfg_s *dev_cfg;
	const struct hs_settle_s *hs_timing_cfg;
	const struct dsc_instr *dsi_init_cmds;
	CVI_S32 dsi_init_cmds_size;
} DSI_CONFIG_DESC_S;

typedef struct _panel_desc_s {
    CVI_U32 panel_num;
    PANEL_INTERFACE_E panel_interface;
	PANEL_TYPE_E panel_type;
	union {
		DSI_CONFIG_DESC_S dsi_cfg;
		VO_PUB_ATTR_S vo_pub_attr;
	};
    CVI_S32 mipi_tx_fd[APP_IPCAM_VO_MAX_NUM];
}PANEL_DESC_S;

PANEL_DESC_S *app_ipcam_panel_param_get(void);
CVI_S32 app_ipcam_panel_init(void);
CVI_S32 app_ipcam_panel_uninit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
