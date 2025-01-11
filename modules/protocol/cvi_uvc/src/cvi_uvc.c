#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "errno.h"
#include <sys/prctl.h>

#include "app_ipcam_comm.h"
#include "cvi_uvc.h"
#include "cvi_uvc_gadget.h"

#define UVC_DEV_NAME "/dev/video0"
#define UVC_THREAD_PRIORITY 80

static bool s_uvc_init = false;

static RUN_THREAD_PARAM mUvcThread;
static pthread_mutex_t RsUvcMutex = PTHREAD_MUTEX_INITIALIZER;

static void *UVC_CheckTask(void *pvArg)
{
    prctl(PR_SET_NAME, "UVC_CheckTask", 0, 0, 0);
    int32_t ret = 0;
    while (mUvcThread.bRun_flag) {
        ret = UVC_GADGET_DeviceCheck();
        if (ret < 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_GADGET_DeviceCheck %x\n", ret);
            break;
        } else if (ret == 0) {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "Timeout Do Nothing\n");
        }
        usleep(1000);
    }
    return NULL;
}

static int UVC_Init()
{
    UVC_GADGET_Init();
    return 0;
}

static int UVC_Deinit(void)
{
    return 0;
}

static int UVC_Start()
{
    if (false == mUvcThread.bRun_flag) {
        if (UVC_GADGET_DeviceOpen(UVC_DEV_NAME)) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_GADGET_DeviceOpen Failed!\n");
            return -1;
        }

        mUvcThread.bRun_flag = true;
        pthread_attr_t pthread_attr;
        pthread_attr_init(&pthread_attr);

        struct sched_param param;
        param.sched_priority = UVC_THREAD_PRIORITY;
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
        pthread_attr_setschedparam(&pthread_attr, &param);
        pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);

        if (pthread_create(&mUvcThread.mRun_PID, &pthread_attr, UVC_CheckTask, NULL)) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_CheckTask create thread failed!\n");
            mUvcThread.bRun_flag = false;
            return -1;
        }
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "UVC_CheckTask create thread successful\n");
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "UVC already started\n");
    }

    return 0;
}

static int UVC_Stop(void)
{
    if (false == mUvcThread.bRun_flag) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "UVC not run\n");
        return 0;
    }

    mUvcThread.bRun_flag = false;
    pthread_join(mUvcThread.mRun_PID, NULL);

    return UVC_GADGET_DeviceClose();
}

void app_uvc_exit(void)
{
    if (!s_uvc_init) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "uvc not init\n");
        return;
    }
    pthread_mutex_lock(&RsUvcMutex);
    s_uvc_init = false;
    pthread_mutex_unlock(&RsUvcMutex);

    if (UVC_Stop() != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_Stop Failed!\n");
    }
    if (UVC_Deinit() != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_Deinit Failed!\n");
    }
}

int app_uvc_init(void)
{
    if (access(UVC_DEV_NAME, F_OK) != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "file %s not found\n", UVC_DEV_NAME);
        return -1;
    }

    if (UVC_Init() != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_Init Failed!\n");
        return -1;
    }

    if (UVC_Start() != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC_Start Failed!\n");
        return -1;
    }

    pthread_mutex_lock(&RsUvcMutex);
    s_uvc_init = true;
    pthread_mutex_unlock(&RsUvcMutex);
    return 0;
}