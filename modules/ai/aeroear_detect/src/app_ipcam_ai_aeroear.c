/*
 * app_ipcam_ai_aeroear.c — AeroEar acoustic drone detection module
 *
 * Uses TDL_SDK standard API (TDL_OpenModel / TDL_Classification) for model
 * loading and inference. Preprocessing, TPU encoder inference, and CPU GRU
 * detection are all encapsulated in tdl_sdk's AeroEarClassification class.
 * Audio buffering and zero-padding logic remain on the ipcamera side.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "app_ipcam_ai.h"
#include "app_ipcam_audio.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/* Audio sample rate */
#define AEROEAR_SR                      48000

/* PCM frame size (48kHz 16bit mono, 5 seconds) */
#define AEROEAR_FRAME_BYTES             (AEROEAR_SR * 2 * 5)

/*
 * Initial audio fill for first detection (seconds).
 * Zero-pad front, fill last INITIAL_SECOND with real audio.
 * Reduces first-detection latency from 5s to 2s.
 */
#define AEROEAR_INITIAL_SECOND          2

/*
 * Recent audio window for subsequent calls (seconds).
 * Zero-pad front, fill last RECENT_SECOND with real audio from ring buffer.
 * Trades off detection speed vs clearing speed:
 * - More recent audio → faster detection, slower clearing
 * - Less recent audio → slower detection, faster clearing
 * 2.5s (78 frames) gives balanced performance.
 */
#define AEROEAR_RECENT_SECOND           2.5

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

static APP_PARAM_AI_AEROEAR_CFG_S g_stAeroearCfg;
static APP_PARAM_AI_AEROEAR_CFG_S *g_pstAeroearCfg = &g_stAeroearCfg;
static volatile bool g_bAeroearRunning = CVI_FALSE;
static volatile bool g_bAeroearPause = CVI_FALSE;
static pthread_t g_AeroearThreadHandle;
static TDLHandle g_AeroearHandle = NULL;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_AEROEAR_CFG_S *app_ipcam_Ai_Aeroear_Param_Get(void)
{
    return g_pstAeroearCfg;
}

CVI_VOID app_ipcam_Ai_Aeroear_ProcStatus_Set(CVI_BOOL flag)
{
    g_bAeroearRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Aeroear_ProcStatus_Get(void)
{
    return g_bAeroearRunning;
}

CVI_VOID app_ipcam_Ai_Aeroear_Pause_Set(CVI_BOOL flag)
{
    g_bAeroearPause = flag;
}

CVI_BOOL app_ipcam_Ai_Aeroear_Pause_Get(void)
{
    return g_bAeroearPause;
}

/*
 * Check if audio parameters meet AeroEar requirements:
 * 48kHz sample rate, 16bit, mono.
 */
static CVI_S32 app_ipcam_Ai_Aeroear_Check_Audio_Param(void)
{
    APP_PARAM_AUDIO_CFG_T *pstAudioCfg = app_ipcam_Audio_Param_Get();

    if (pstAudioCfg->astAudioCfg.enSamplerate != AUDIO_SAMPLE_RATE_48000)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI Aeroear model require 48kHz samplerate, current %d!\n",
            pstAudioCfg->astAudioCfg.enSamplerate);
        return CVI_FAILURE;
    }

    if (pstAudioCfg->astAudioCfg.enSoundmode != AUDIO_SOUND_MODE_MONO)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "AI Aeroear recommend mono sound mode\n");
    }

    return CVI_SUCCESS;
}

/*
 * Detection main loop.
 *
 * Audio buffer: [zero-padded front | recent audio from ring buffer tail].
 * First iteration: 3s zeros + 2s real audio.
 * Subsequent iterations: 2.5s zeros + 2.5s real audio.
 * Zero-padding avoids CNN receptive field pollution from old drone audio.
 */
static CVI_VOID *Thread_Aeroear_PROC(CVI_VOID *pArgs)
{
    prctl(PR_SET_NAME, "Thread_Ai_Aeroear_Proc", 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Aeroear start running!\n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    /* Allocate audio buffer */
    CVI_U32 u32BufferSize = AEROEAR_FRAME_BYTES;
    CVI_U8 *buffer = (CVI_U8 *)malloc(u32BufferSize);
    if (!buffer) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Aeroear: buffer malloc failed\n");
        pthread_exit(NULL);
        return NULL;
    }
    memset(buffer, 0, u32BufferSize);

    /* Batch counter */
    int batch_count = 0;

    /* Frame struct for TDL_Classification */
    VIDEO_FRAME_INFO_S Frame;
    memset(&Frame, 0, sizeof(VIDEO_FRAME_INFO_S));
    Frame.stVFrame.pu8VirAddr[0] = buffer;
    Frame.stVFrame.u32Height = 1;
    Frame.stVFrame.u32Width = u32BufferSize;

    while (app_ipcam_Ai_Aeroear_ProcStatus_Get()) {
        if (app_ipcam_Ai_Aeroear_Pause_Get()) {
            usleep(1000 * 1000);
            continue;
        }

        CVI_U32 t_loop_start = GetCurTimeInMsec();

        /*
         * Fill audio buffer: zero-padded front + recent audio from ring buffer tail.
         * First call uses INITIAL_SECOND; subsequent calls use RECENT_SECOND.
         */
        if (batch_count == 0) {
            CVI_U32 initial_bytes = AEROEAR_SR * 2 * AEROEAR_INITIAL_SECOND;
            CVI_U32 pad_bytes = u32BufferSize - initial_bytes;
            memset(buffer, 0, pad_bytes);
            s32Ret = app_ipcam_Ai_Aeroear_Audio_Buffer_Get(
                buffer + pad_bytes, initial_bytes);
        } else {
            CVI_U32 recent_bytes = AEROEAR_SR * 2 * AEROEAR_RECENT_SECOND;
            CVI_U32 pad_bytes = u32BufferSize - recent_bytes;
            memset(buffer, 0, pad_bytes);
            s32Ret = app_ipcam_Ai_Aeroear_Audio_Buffer_Get(
                buffer + pad_bytes, recent_bytes);
        }

        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "Aeroear: Audio_Buffer_Get failed, ret=%d\n", s32Ret);
            usleep(500 * 1000);
            continue;
        }

        CVI_U32 t_audio_ready = GetCurTimeInMsec();

        /*
         * Call TDL_SDK classification API.
         * tdl_sdk internally performs:
         *   int16→float32 → dual-window log-mel → TPU encoder → CPU GRU → threshold
         */
        TDLImage image_handle = TDL_WrapFrame((void*)&Frame, false, false);
        if (image_handle == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Aeroear: TDL_WrapFrame failed\n");
            usleep(500 * 1000);
            continue;
        }

        TDLClassInfo obj_info = {0};
        s32Ret = TDL_Classification(g_AeroearHandle,
                                      g_pstAeroearCfg->model_id,
                                      image_handle, &obj_info);
        if (s32Ret == CVI_SUCCESS) {
            CVI_U32 t_infer_done = GetCurTimeInMsec();
            CVI_U32 dt_audio = t_audio_ready - t_loop_start;
            CVI_U32 dt_infer = t_infer_done - t_audio_ready;
            if (obj_info.class_id == 1) {
                APP_PROF_LOG_PRINT(LEVEL_WARN,
                    "Aeroear: drone detected! (score=%.3f, batch=%d, "
                    "t_start=%ums, audio=%ums, infer=%ums, total=%ums)\n",
                    obj_info.score, batch_count,
                    t_loop_start, dt_audio, dt_infer,
                    t_infer_done - t_loop_start);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "Aeroear: no drone (score=%.3f, batch=%d, "
                    "t_start=%ums, audio=%ums, infer=%ums, total=%ums)\n",
                    obj_info.score, batch_count,
                    t_loop_start, dt_audio, dt_infer,
                    t_infer_done - t_loop_start);
            }
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "Aeroear: TDL_Classification failed, ret=%d\n", s32Ret);
        }

        TDL_DestroyImage(image_handle);

        batch_count++;

        /* No sleep needed — TPU inference (~6s) naturally paces the loop. */
    }

    free(buffer);

    pthread_exit(NULL);
    return NULL;
}

/* Initialize AeroEar: create TDLHandle and load model via TDL_OpenModel. */
static CVI_S32 app_ipcam_Ai_Aeroear_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Aeroear init ------------------> start\n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_AeroearHandle != NULL) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Aeroear: TDLHandle already created\n");
        return CVI_SUCCESS;
    }

    /* Create TDL handle */
    g_AeroearHandle = TDL_CreateHandle(0);
    if (g_AeroearHandle == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Aeroear: TDL_CreateHandle failed\n");
        return CVI_FAILURE;
    }

    /* Load model */
    s32Ret = TDL_OpenModel(g_AeroearHandle,
                            g_pstAeroearCfg->model_id,
                            g_pstAeroearCfg->model_path,
                            NULL, 0);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "Aeroear: TDL_OpenModel failed with %#x! model_path=%s\n",
            s32Ret, g_pstAeroearCfg->model_path);
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Aeroear init ------------------> done\n");
    return CVI_SUCCESS;
}

/*
 * Stop AeroEar: unload model and destroy handle via TDL_CloseModel / TDL_DestroyHandle.
 */
int app_ipcam_Ai_Aeroear_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstAeroearCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Aeroear not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Aeroear_ProcStatus_Get()) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Aeroear not running\n");
        return s32Ret;
    }

    app_ipcam_Ai_Aeroear_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_AeroearThreadHandle) {
        pthread_join(g_AeroearThreadHandle, NULL);
        g_AeroearThreadHandle = 0;
    }

    /* Unload model and destroy handle */
    if (g_AeroearHandle) {
        TDL_CloseModel(g_AeroearHandle, g_pstAeroearCfg->model_id);
        s32Ret = TDL_DestroyHandle(g_AeroearHandle);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "Aeroear: TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        }
        g_AeroearHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "AI Aeroear Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

/* Start AeroEar module. */
int app_ipcam_Ai_Aeroear_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstAeroearCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Aeroear not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bAeroearRunning) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Aeroear already started\n");
        return CVI_SUCCESS;
    }

    /* Check audio parameters */
    s32Ret = app_ipcam_Ai_Aeroear_Check_Audio_Param();
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "app_ipcam_Ai_Aeroear_Check_Audio_Param failed!\n");
        return s32Ret;
    }

    /* Initialize model */
    s32Ret = app_ipcam_Ai_Aeroear_Proc_Init();
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "app_ipcam_Ai_Aeroear_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Aeroear_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_AeroearThreadHandle, NULL,
                            Thread_Aeroear_PROC, NULL);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI Aeroear pthread_create failed!\n");
        return s32Ret;
    }

    return s32Ret;
}

CVI_S32 app_ipcam_Ai_Aeroear_StatusGet(void)
{
    return g_AeroearHandle ? 1 : 0;
}