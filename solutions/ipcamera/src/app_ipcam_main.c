#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "app_ipcam_paramparse.h"
#include "app_ipcam_ircut.h"

#ifdef WEB_SOCKET
#include "app_ipcam_websocket.h"
#include "app_ipcam_netctrl.h"
#endif

#ifdef CVI_UVC_SUPPORT
#include "cvi_uvc.h"
#endif

#ifdef CVI_UAC_SUPPORT
#include "cvi_audio_uac.h"
#endif


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
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if ((SIGINT == signo) || (SIGTERM == signo)) {
        app_ipcam_Exit();
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate\n", signo);
    }

    exit(-1);
}

static CVI_VOID app_ipcam_Usr1Sig_handle(CVI_S32 signo)
{
    if (SIGUSR1 == signo) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate and start trigger a picture\n", signo);
        for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < VENC_CHN_MAX; s32ChnIdx++) {
            app_ipcam_JpgCapFlag_Set(CVI_TRUE, s32ChnIdx);
        }
    }
}

static int app_ipcam_Peripheral_Init(void)
{
    /* do peripheral Init here */

    app_ipcam_IRCut_Init();

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

    #ifdef CVI_UVC_SUPPORT
    app_uvc_exit();
    #endif
    #ifdef CVI_UAC_SUPPORT
    app_uac_exit();
    #endif

    #ifdef RECORD_SUPPORT
    APP_CHK_RET(app_ipcam_Record_UnInit(), "SD Record UnInit");
    #endif

    APP_CHK_RET(app_ipcam_Osdc_DeInit(), "OSDC DeInit");

    #ifdef AI_SUPPORT
    #if defined AUDIO_SUPPORT && defined AI_BABYCRY_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Cry_Stop(), "AI Cry Stop");
    #endif
    #ifdef PD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_PD_Stop(), "PD Stop");
    #endif
    #ifdef MD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_MD_Stop(), "MD Stop");
    #endif
    #ifdef FACE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_FD_Stop(), "FD Stop");
    #endif
    #ifdef HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Stop(), "Human Keypoint Detection Stop");
    #endif
    #ifdef OBJECT_TRACK_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Object_Track_Stop(), "ObjectTrack Stop");
    #endif
    #ifdef KEYPOINT_HAND_GESTURE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Keypoint_Hand_Gesture_Stop(), "Keypoint Hand Gesture Detection Stop");
    #endif
    #ifdef IMG_TXT_CLIP_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Img_Txt_Clip_Stop(), "Img Txt Clip Stop");
    #endif
    #endif

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_UnInit(), "Audio UnInit");
    #endif

    #ifdef DISP_FRMBUF
    APP_CHK_RET(app_ipcam_Disp_FrmBuf_Stop(), "DISP FRMBUF Stop");
    #endif

    #ifdef FRMBUF
    APP_CHK_RET(app_ipcam_FrmBuf_DeInit(), "FrmBuf DeInit");
    #endif

    #ifdef DISPLAY
    APP_CHK_RET(app_ipcam_Display_Exit(), "Display Exit");
    #endif

    #ifdef DPU_SUPPORT
    APP_CHK_RET(app_ipcam_Dpu_Stop(), "DPU Stop");
    APP_CHK_RET(app_ipcam_Dpu_UnInit(), "DPU UnInit");
    #endif

    #ifdef STITCH_SUPPORT
    APP_CHK_RET(app_ipcam_Stitch_UnInit(), "Stitch UnInit");
    #endif

    #ifdef GDC_SUPPORT
    APP_CHK_RET(app_ipcam_Gdc_DeInit(), "GDC DeInit");
    #endif

    APP_CHK_RET(app_ipcam_Venc_Stop(APP_VENC_ALL), "VENC Stop");
    APP_CHK_RET(app_ipcam_Vpss_DeInit(), "VPSS DeInit");

    #ifdef VDEC_SOFT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Stop(APP_VDEC_SOFT_ALL), "VDEC SOFT Stop");
    APP_CHK_RET(app_ipcam_Vdec_Soft_DeInit(), "VDEC SOFT DeInit");
    #endif

    #ifdef VDEC
    APP_CHK_RET(app_ipcam_Vdec_Stop(), "VDEC Stop");
    #endif

    APP_CHK_RET(app_ipcam_Vi_DeInit(), "VI DeInit");
    APP_CHK_RET(app_ipcam_Sys_DeInit(), "System DeInit");
#ifdef RTSP_SUPPORT
    APP_CHK_RET(app_ipcam_rtsp_Server_Destroy(), "RTSP Server Destroy");
#endif
    return CVI_SUCCESS;
}

static int app_ipcam_Init(void)
{
    APP_CHK_RET(app_ipcam_Peripheral_Init(), "Init Peripheral");
    APP_CHK_RET(app_ipcam_Sys_Init(), "Init Systerm");
    APP_CHK_RET(app_ipcam_Vi_Init(), "Init VI");

    #ifdef VDEC
    APP_CHK_RET(app_ipcam_Vdec_Init(), "Init VDEC");
    #endif

    #ifdef VDEC_SOFT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Init(), "Init VDEC SOFT");
    #endif

    APP_CHK_RET(app_ipcam_Vpss_Init(), "Init VPSS");
    #ifdef GDC_SUPPORT
    APP_CHK_RET(app_ipcam_Gdc_Init(), "Init GDC");
    #endif
    APP_CHK_RET(app_ipcam_Osdc_Init(), "Init Draw Osdc");

    #ifdef STITCH_SUPPORT
    APP_CHK_RET(app_ipcam_Stitch_Init(), "Init Stitch");
    #endif

    #ifdef DPU_SUPPORT
    APP_CHK_RET(app_ipcam_Dpu_Init(), "Init DPU");
    #endif

    #ifdef WEB_SOCKET
    APP_CHK_RET(app_ipcam_NetCtrl_Init(), "Init Net Ctrl");
    APP_CHK_RET(app_ipcam_WebSocket_Init(), "Init Websocket");
    #endif

    APP_CHK_RET(app_ipcam_Venc_Init(APP_VENC_ALL), "Init VENC");

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_Init(), "Init Audio");
    #endif

    #ifdef CVI_UVC_SUPPORT
    app_uvc_init();
    #endif

    #ifdef CVI_UAC_SUPPORT
    app_uac_init();
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

    /* init modules include <Peripheral; Sys; VI; VB; OSD; Venc; AI; Audio; etc.> */
    APP_CHK_RET(app_ipcam_Init(), "Init Ipcam App");

    /* create rtsp server */
#ifdef RTSP_SUPPORT
    APP_CHK_RET(app_ipcam_Rtsp_Server_Create(), "Create RTSP Server");
#endif

    /* start video encode */
    APP_CHK_RET(app_ipcam_Venc_Start(APP_VENC_ALL), "Start VENC");

    /* start video decode */
    #ifdef VDEC
    APP_CHK_RET(app_ipcam_Vdec_Start(), "Start VDEC");
    #endif

    #ifdef VDEC_SOFT
    APP_CHK_RET(app_ipcam_Vdec_Soft_Start(APP_VDEC_SOFT_ALL), "Start VDEC SOFT");
    #endif

    #ifdef DISPLAY
    APP_CHK_RET(app_ipcam_Display_Init(), "Init Display");
    #endif

    #ifdef FRMBUF
    APP_CHK_RET(app_ipcam_FrmBuf_Init(), "Init FrameBuffer");
    #endif

    #ifdef DISP_FRMBUF
    APP_CHK_RET(app_ipcam_Disp_FrmBuf_Start(), "Start DISP FRMBUF");
    #endif

    #ifdef AI_SUPPORT
    #ifdef PD_SUPPORT
    /* start AI PD (Pedestrian Detection) */
    APP_CHK_RET(app_ipcam_Ai_PD_Start(), "running AI PD");
    #endif

    #ifdef MD_SUPPORT
    /* start AI MD (Motion Detection)*/
    APP_CHK_RET(app_ipcam_Ai_MD_Start(), "running AI MD");
    #endif

    /* start AI FD (Face Detection)*/
    #ifdef FACE_SUPPORT
    #ifdef IR_FACE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_IR_FD_Start(), "running AI IR FD");
    #else
    APP_CHK_RET(app_ipcam_Ai_FD_Start(), "running AI FD");
    #endif
    #endif

    #ifdef AUDIO_SUPPORT
    #ifdef AI_BABYCRY_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Cry_Start(), "running AI CRY");
    #endif
    #endif

    #ifdef HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Start(), "Human Keypoint Detection Start");
    #endif

    #ifdef OBJECT_TRACK_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Object_Track_Start(), "ObjectTrack Start");
    #endif

    #ifdef KEYPOINT_HAND_GESTURE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Keypoint_Hand_Gesture_Start(), "Keypoint Hand Gesture Detection Start");
    #endif

    #ifdef IMG_TXT_CLIP_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Img_Txt_Clip_Start(), "Img Txt Clip Stop");
    #endif
    #endif

    #ifdef RECORD_SUPPORT
    /* start SD Record */
    APP_CHK_RET(app_ipcam_Record_Recover_Init(), "Init SD Record Recover");
    APP_CHK_RET(app_ipcam_Record_Init(), "Init SD Record");
    #endif

    #ifdef DPU_SUPPORT
    APP_CHK_RET(app_ipcam_Dpu_Start(), "Start dpu");
    #endif
    /* enable receive a command form another progress for test ipcam */
    // APP_CHK_RET(app_ipcam_CmdTask_Create(), "Create CMD Test");

    while (1) {
        sleep(1);
    };

    return CVI_SUCCESS;
}
