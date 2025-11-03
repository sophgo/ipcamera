#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "app_ipcam_paramparse.h"

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
static pthread_t g_pthMisc;
static CVI_BOOL g_bMisc;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int app_ipcam_Exit(void);

static CVI_VOID app_ipcam_ExitSig_handle(CVI_S32 signo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if ((SIGINT == signo) || (SIGTERM == signo)) {
        s32Ret = app_ipcam_Exit();
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate\n", signo);
    }

    exit(s32Ret);
}

static CVI_VOID app_ipcam_Usr1Sig_handle(CVI_S32 signo)
{
    if (SIGUSR1 == signo) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate and start trigger a picture\n", signo);
        #ifdef MPI_VENC_MODULE_SUPPORT
        app_ipcam_JpgCapFlag_Set(CVI_TRUE);
        #endif
    }
}

static int app_ipcam_Peripheral_Init(void)
{
    /* do peripheral Init here */

#ifdef SDCARD_SUPPORT
    app_ipcam_SdCard_Init();
#endif
#ifdef IRCUT_MODULE_SUPPORT
    app_ipcam_IRCut_Init();
#endif
#ifdef PANEL_SUPPORT
    app_ipcam_panel_init();
#endif

    return CVI_SUCCESS;
}

static int app_ipcam_Peripheral_UnInit(void)
{
    /* do peripheral Init here */
#ifdef SDCARD_SUPPORT
    app_ipcam_SdCard_UnInit();
#endif
#ifdef IRCUT_MODULE_SUPPORT
    app_ipcam_IrCut_DeInit();
#endif
#ifdef PANEL_SUPPORT
    app_ipcam_panel_uninit();
#endif

    return CVI_SUCCESS;
}

/*
* this thread handle a series of small tasks include
* a. send AI framerate to Web-client
* b.
*/
static void *ThreadMisc(void *arg)
{
    while (g_bMisc) {
        #ifdef WEB_SOCKET
        app_ipcam_WebSocket_AiFps_Send();
        #endif
        sleep(1);
    }

    return NULL;
}

static int app_ipcam_MiscThread_Init(void)
{
    int s32Ret = CVI_SUCCESS;

    g_bMisc = CVI_TRUE;
    s32Ret = pthread_create(&g_pthMisc, NULL, ThreadMisc, NULL);
    if (s32Ret != 0) {
        printf("pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static int app_ipcam_MiscThread_DeInit(void)
{
    g_bMisc = CVI_FALSE;

    if (g_pthMisc) {
        pthread_cancel(g_pthMisc);
        pthread_join(g_pthMisc, NULL);
        g_pthMisc = 0;
    }

    return CVI_SUCCESS;
}

static int app_ipcam_Exit(void)
{
    APP_CHK_RET(app_ipcam_MiscThread_DeInit(), "MiscThread DeInit");

    #ifdef BLACKLIGHT_SUPPORT
    APP_CHK_RET(app_ipcam_BlackLight_DeInit(), "Deinit black light.");
    #endif

    #ifdef MPI_GDC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_GDC_DeInit(), "Deinit gdc.");
    #endif

    #ifdef FRMBUF_LVGL_SUPPORT
    APP_CHK_RET(app_ipcam_FrmBuf_LVGL_Stop(), "Stop LVGL.");
    APP_CHK_RET(app_ipcam_FrmBuf_LVGL_DeInit(), "Deinit LVGL.");
    #endif

    #ifdef RTSP_SUPPORT
    APP_CHK_RET(app_ipcam_rtsp_Server_Destroy(), "RTSP Server Destroy");
    #endif

    #ifdef CLOUD_SUPPORT
    APP_CHK_RET(app_hal_platform_deinit(), "Cloud DeInit");
    #endif

    #ifdef CVI_UVC_SUPPORT
    app_uvc_exit();
    #endif
    #ifdef CVI_UAC_SUPPORT
    app_uac_exit();
    #endif

    #ifdef RECORD_SUPPORT
    APP_CHK_RET(app_ipcam_Record_UnInit(), "SD Record UnInit");
    #endif

    #ifdef MPI_OSD_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Osdc_DeInit(), "OSDC DeInit");
    #endif

    #ifdef TDL_SUPPORT
    #ifdef TDL_CAPTURE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Capture_Stop(), "Capture Stop");
    #endif

    #ifdef TDL_FD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_FD_Stop(), "Stop FD");
    #endif

    #ifdef TDL_MD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_MD_Stop(), "Stop MD");
    #endif

    #ifdef TDL_PD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_PD_Stop(), "Stop PD");
    #endif

    #ifdef TDL_HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Stop(), "Stop Human Keypoint Detection");
    #endif

    #ifdef TDL_MOTION_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Motion_Stop(), "Stop Ai Motion");
    #endif

    #if defined(MPI_AUDIO_MODULE_SUPPORT) && defined(TDL_SOUND_CLS)
    APP_CHK_RET(app_ipcam_Ai_Cry_Stop(), "Stop Sound cls");
    #endif
    #endif

    #ifdef MPI_AUDIO_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_UnInit(), "Audio UnInit");
    #endif

    #ifdef MPI_VO_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_vo_stop(), "Stop VO");
    #endif

    #ifdef MPI_VDEC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Stop(), "VDEC Stop");
    #endif

    #ifdef FRMBUF_DISP_SUPPORT
    APP_CHK_RET(app_ipcam_FrmBuf_Disp_Stop(), "DISP FRMBUF Stop");
    #endif

    #ifdef VDEC_SOFT_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Stop(APP_VDEC_SOFT_ALL), "VDEC SOFT Stop");
    APP_CHK_RET(app_ipcam_Vdec_Soft_DeInit(), "VDEC SOFT DeInit");
    #endif

    #ifdef FRMBUF
    APP_CHK_RET(app_ipcam_FrmBuf_DeInit(), "FrmBuf DeInit");
    #endif

    #ifdef MPI_STITCH_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Stitch_UnInit(), "Stitch UnInit");
    #endif

    #ifdef MPI_VPSS_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vpss_DeInit(), "VPSS DeInit");
    #endif

    #ifdef MPI_VENC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Venc_Stop(APP_VENC_ALL), "VENC Stop");
    #endif

    #ifdef MPI_VI_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vi_DeInit(), "VI DeInit");
    #endif
    APP_CHK_RET(app_ipcam_Sys_DeInit(), "System DeInit");

    #ifdef MBUF_SUPPORT
    APP_CHK_RET(app_ipcam_Mbuf_UnInit(), "UnInit Mbuf");
    #endif

    #ifdef PERIPHERAL_SUPPORT
    APP_CHK_RET(app_ipcam_Peripheral_UnInit(), "UnInit Peripheral");
    #endif

    return CVI_SUCCESS;
}

static int app_ipcam_Init(void)
{
    #ifdef PERIPHERAL_SUPPORT
    APP_CHK_RET(app_ipcam_Peripheral_Init(), "Init Peripheral");
    #endif

    APP_CHK_RET(app_ipcam_Sys_Init(), "Init Systerm");

    #ifdef MPI_VI_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vi_Init(), "Init VI");
    #endif

    #ifdef MPI_VPSS_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vpss_Init(), "Init VPSS");
    #endif

    #ifdef MPI_OSD_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Osdc_Init(), "Init Draw Osdc");
    #endif

    #ifdef MPI_STITCH_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Stitch_Init(), "Init Stitch");
    #endif

    #ifdef WEB_SOCKET
    APP_CHK_RET(app_ipcam_NetCtrl_Init(), "Init Net Ctrl");
    APP_CHK_RET(app_ipcam_WebSocket_Init(), "Init Websocket");
    #endif

    #ifdef MPI_VENC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Venc_Init(APP_VENC_ALL), "Init VENC");
    #endif

    #ifdef MPI_VDEC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Init(), "Init VDEC");
    #endif

    #ifdef VDEC_SOFT_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Init(), "Init VDEC SOFT");
    #endif

    #ifdef MPI_AUDIO_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_Init(), "Init Audio");
    #endif

    #ifdef CVI_UVC_SUPPORT
    app_uvc_init();
    #endif

    #ifdef CVI_UAC_SUPPORT
    app_uac_init();
    #endif

    #ifdef FRMBUF
    APP_CHK_RET(app_ipcam_FrmBuf_Init(), "Init FrameBuffer");
    #endif

    #ifdef FRMBUF_LVGL_SUPPORT
    APP_CHK_RET(app_ipcam_FrmBuf_LVGL_Init(), "Init LVGL.");
    #endif

    #ifdef MPI_GDC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_GDC_Init(), "Init gdc.");
    #endif

    #ifdef BLACKLIGHT_SUPPORT
    APP_CHK_RET(app_ipcam_BlackLight_Init(), "Init black light.");
    #endif

    #ifdef CLOUD_SUPPORT
    APP_CHK_RET(app_hal_platform_init(), "Init Cloud");
    #endif

    APP_CHK_RET(app_ipcam_MiscThread_Init(), "Init Misc");

    return CVI_SUCCESS;
}

int main(int argc, char *argv[])
{
    system("echo /mnt/nfs/core-%e-%p-%t > /proc/sys/kernel/core_pattern");
    APP_CHK_RET(app_ipcam_Opts_Parse(argc, argv), "Parse Optinos");

    signal(SIGINT, app_ipcam_ExitSig_handle);
    signal(SIGTERM, app_ipcam_ExitSig_handle);
    signal(SIGUSR1, app_ipcam_Usr1Sig_handle);

    /* load each moudles parameter from param_config.ini */
    APP_CHK_RET(app_ipcam_Param_Load(), "Load Global Parameter");

    #ifdef MBUF_SUPPORT
    APP_CHK_RET(app_ipcam_Mbuf_Init(), "Init Mbuf");
    #endif

    /* init modules include <Peripheral; Sys; VI; VB; OSD; Venc; AI; Audio; etc.> */
    APP_CHK_RET(app_ipcam_Init(), "Init Ipcam App");

    /* Configuration print level */
    #ifdef LOG_SUPPORT
    CVI_LOG_SET_LEVEL(CVI_LOG_WARN);
    #endif

    /* create rtsp server */
    #ifdef RTSP_SUPPORT
    APP_CHK_RET(app_ipcam_Rtsp_Server_Create(), "Create RTSP Server");
    #endif

    /* start video encode */
    #ifdef MPI_VENC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Venc_Start(APP_VENC_ALL), "Start VENC");
    #endif

    /* start video decode */
    #ifdef MPI_VDEC_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Start(), "Start VDEC");
    #endif

    /* start video output */
    #ifdef MPI_VO_MODULE_SUPPORT
    APP_CHK_RET(app_ipcam_vo_start(), "Start VO");
    #endif

    #ifdef VDEC_SOFT_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Start(APP_VDEC_SOFT_ALL), "Start VDEC SOFT");
    #endif

    #ifdef FRMBUF_DISP_SUPPORT
    APP_CHK_RET(app_ipcam_FrmBuf_Disp_Start(), "Start DISP FRMBUF");
    #endif

    #ifdef TDL_SUPPORT
    #ifdef TDL_CAPTURE_SUPPORT
    /* start AI Capture */
    APP_CHK_RET(app_ipcam_Ai_Capture_Start(), "Start AI Capture");
    #endif

    #ifdef TDL_FD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_FD_Start(), "Start AI Detect");
    #endif

    #ifdef  TDL_MD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_MD_Start(), "Start MD");
    #endif

    #ifdef  TDL_PD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_PD_Start(), "Start PD");
    #endif

    #ifdef TDL_HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Start(), "Start Human Keypoint Detection");
    #endif

    #ifdef TDL_MOTION_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Motion_Start(), "Start AI Motion");
    #endif

    #if defined(MPI_AUDIO_MODULE_SUPPORT) && defined(TDL_SOUND_CLS)
    APP_CHK_RET(app_ipcam_Ai_Cry_Start(), "Start Sound cls");
    #endif
    #endif

    #ifdef RECORD_SUPPORT
    /* start SD Record */
    APP_CHK_RET(app_ipcam_Record_Recover_Init(), "Init SD Record Recover");
    APP_CHK_RET(app_ipcam_Record_Init(), "Init SD Record");
    #endif

    #ifdef FRMBUF_LVGL_SUPPORT
    APP_CHK_RET(app_ipcam_FrmBuf_LVGL_Start(), "Start LVGL.");
    #endif


    /* enable receive a command form another progress for test ipcam */
    // APP_CHK_RET(app_ipcam_CmdTask_Create(), "Create CMD Test");

    while (1) {
        sleep(1);
    };

    return CVI_SUCCESS;
}
