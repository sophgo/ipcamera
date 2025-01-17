#include <stdbool.h>
#include <stdio.h>

#include "cvi_comm_vo.h"
#include "cvi_comm_video.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_mipi_tx.h"
#include "app_ipcam_display.h"

APP_PARAM_MULTI_DISPLAY_CFG_T g_stDisplayCfg = {
    .vo_cfg[0]={
        .enPanelName = PANEL_DSI_HX8394_EVB,
        .stVoCfg = {
            .s32VoDev = 0,
            .stVoPubAttr = {
                .u32BgColor = 0x00000000,
                .enIntfType = VO_INTF_MIPI,
                .enIntfSync = VO_OUTPUT_720x1280_60,
                .stSyncInfo = {CVI_FALSE, CVI_FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, CVI_FALSE, CVI_FALSE, CVI_FALSE},
            },
            .stLayerAttr = {
                .stDispRect = {.s32X = 0, .s32Y = 0, .u32Width = 720, .u32Height = 1280},
                .stImageSize = {.u32Width = 720, .u32Height = 1280},
                .u32DispFrmRt = 60,
                .enPixFormat = PIXEL_FORMAT_NV21
            },
            .enVoMode = VO_MODE_1MUX,
            .enRotation = ROTATION_90,
            .u32DisBufLen = 3,
            .stSrcChn = {.enModId = CVI_ID_VPSS, .s32DevId = 0, .s32ChnId = 2},
            .stDstChn = {.enModId = CVI_ID_VO, .s32DevId = 1, .s32ChnId = 0}
        }
    },
    .vo_num=1
};

APP_PARAM_MULTI_DISPLAY_CFG_T *g_pstDisplayCfg = &g_stDisplayCfg;

APP_PARAM_MULTI_DISPLAY_CFG_T *app_ipcam_Display_Param_Get(void)
{
    return g_pstDisplayCfg;
}

CVI_S32 s32MipiTxFd[VO_MAX_DEV_NUM];

int app_ipcam_Display_SwitchOn(CVI_GPIO_NUM_E PANEL_PWM, CVI_GPIO_NUM_E PANEL_PWR, CVI_GPIO_NUM_E PANEL_RST, CVI_BOOL bEnable)
{
    if (bEnable) {
        app_ipcam_Gpio_Export(PANEL_PWM);
        app_ipcam_Gpio_Dir_Set(PANEL_PWM, CVI_GPIO_DIR_OUT);
        app_ipcam_Gpio_Export(PANEL_PWR);
        app_ipcam_Gpio_Dir_Set(PANEL_PWR, CVI_GPIO_DIR_OUT);
        app_ipcam_Gpio_Export(PANEL_RST);
        app_ipcam_Gpio_Dir_Set(PANEL_RST, CVI_GPIO_DIR_OUT);

        app_ipcam_Gpio_Value_Set(PANEL_PWM, CVI_GPIO_VALUE_H);
        usleep(200*1000);

        app_ipcam_Gpio_Value_Set(PANEL_PWR, CVI_GPIO_VALUE_H);
        usleep(200*1000);

        app_ipcam_Gpio_Value_Set(PANEL_RST, CVI_GPIO_VALUE_H);
        usleep(200*1000);
    }

    return CVI_SUCCESS;
}

int app_ipcam_Display_SwitchOff(CVI_GPIO_NUM_E PANEL_PWM, CVI_GPIO_NUM_E PANEL_PWR, CVI_GPIO_NUM_E PANEL_RST, CVI_BOOL bEnable)
{
    if (bEnable) {
        app_ipcam_Gpio_Value_Set(PANEL_PWM, CVI_GPIO_VALUE_L);
        app_ipcam_Gpio_Value_Set(PANEL_PWR, CVI_GPIO_VALUE_L);
        app_ipcam_Gpio_Value_Set(PANEL_RST, CVI_GPIO_VALUE_L);
        app_ipcam_Gpio_Unexport(PANEL_PWM);
        app_ipcam_Gpio_Unexport(PANEL_PWR);
        app_ipcam_Gpio_Unexport(PANEL_RST);
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Display_Init(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================================== Display Init START ========================================\n");
    PANEL_DESC_T stPanelDesc = {
        .pchPanelName = NULL,
        .enPanelType = 0,
        .pstDevCfg = NULL,
        .pstHsTimingCfg = NULL,
        .pstDsiInitCmds = NULL,
        .s32DsiInitCmdsSize = 0
    };
    CVI_U32 vonum = g_pstDisplayCfg->vo_num;
    APP_PARAM_GPIO_CFG_S *pstGpioCfg = app_ipcam_Gpio_Param_Get();

    app_ipcam_Display_SwitchOn(pstGpioCfg->PANEL_PWM, \
                                pstGpioCfg->PANEL_PWR,\
                                pstGpioCfg->PANEL_RST,\
                                pstGpioCfg->bPanelGpioEn);

    while(vonum > 0)
    {
        vonum--;
        s32MipiTxFd[vonum] = -1;
        app_ipcam_Panel_PanelDesc_Get(
            &g_pstDisplayCfg->vo_cfg[vonum].enPanelName,
            &g_pstDisplayCfg->vo_cfg[vonum].stVoCfg.s32VoDev,
            &stPanelDesc);
        app_ipcam_MipiTx_Enable(
            &g_pstDisplayCfg->vo_cfg[vonum].stVoCfg.s32VoDev,
            stPanelDesc.pchPanelName,
            stPanelDesc.pstDevCfg,
            stPanelDesc.pstHsTimingCfg,
            stPanelDesc.pstDsiInitCmds,
            &stPanelDesc.s32DsiInitCmdsSize,
            &s32MipiTxFd[vonum]
        );
        app_ipcam_Panel_FillIntfAttr(&g_pstDisplayCfg->vo_cfg[vonum].stVoCfg);
        app_ipcam_Vo_Start(&g_pstDisplayCfg->vo_cfg[vonum].stVoCfg);
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "========================================= Display Init END =========================================\n");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Display_Exit(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "======================================== Display Exit START ========================================\n");
    APP_PARAM_GPIO_CFG_S *pstGpioCfg = app_ipcam_Gpio_Param_Get();

    APP_CHK_RET(app_ipcam_Display_SwitchOff(pstGpioCfg->PANEL_PWM, \
                                            pstGpioCfg->PANEL_PWR,\
                                            pstGpioCfg->PANEL_RST,\
                                            pstGpioCfg->bPanelGpioEn), "app_ipcam_Display_SwitchOff");

    for (unsigned i = 0; i < g_pstDisplayCfg->vo_num; i++) {
        APP_CHK_RET(app_ipcam_Vo_Stop(&g_pstDisplayCfg->vo_cfg[i].stVoCfg), "app_ipcam_Vo_Stop");
        APP_CHK_RET(app_ipcam_MipiTx_Disable(&s32MipiTxFd[i]), "app_ipcam_MipiTx_Disable");
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "========================================= Display Exit END =========================================\n");

    return CVI_SUCCESS;
}