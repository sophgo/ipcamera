#include "app_ipcam_comm.h"
#include "app_ipcam_sdcard.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/mount.h>

static RUN_THREAD_PARAM s_stSdCardThread;
static sdcard_status_e s_stSdCardStatus = SDCARD_STATUS_NOT_INSERT;

static int app_ipcam_FileExists(const char* filePath)
{
    int iRet = 0;
    if(filePath)
    {
        struct stat st;
        int result = stat(filePath, &st);
        iRet = (result == 0) ? 1 : 0;
    }
    return iRet; 
}

static int app_ipcam_SdCard_CheckInsert()
{
    return (access(SDCARD_DEV_MAIN_PARTITION_NAME,F_OK) == 0) ? 1 : 0;
}

static int app_ipcam_SdCard_CheckMount()
{
    int iRet = -1;
    struct statfs statFS;
    if (statfs(SDCARD_MOUNT_PATH, &statFS) == -1)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s is not mounted, f_type:0x%lx\n", SDCARD_MOUNT_PATH, statFS.f_type);
        return -1;
    }

    if (statFS.f_type == 0x4d44) //MSDOS_SUPER_MAGIC
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "%s is mounted\n", SDCARD_MOUNT_PATH);
        iRet = 0;
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s is not mounted, f_type:0x%lx\n", SDCARD_MOUNT_PATH, statFS.f_type);
        return -1;
    }

    return iRet;
}

static int app_ipcam_SdCard_UMount(void)
{
    int ret = umount2(SDCARD_MOUNT_PATH, MNT_DETACH|MNT_FORCE);
    if(ret != 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unmount (%s) failed: %s\n", SDCARD_MOUNT_PATH, strerror(errno));
    }

    return ret;
}

static int app_ipcam_SdCard_Mount(void)
{
    int ret = 0;
    char stMountDev[128] = {0};
    snprintf(stMountDev, sizeof(stMountDev), SDCARD_DEV_PARTITION_NAME, 1);
    if(app_ipcam_FileExists(stMountDev))
    {
        ret = mount(stMountDev, SDCARD_MOUNT_PATH, "vfat", 0, NULL);
        if(ret != 0)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "mount %s to %s failed: %s\n", stMountDev, SDCARD_MOUNT_PATH, strerror(errno));
            return -1;
        }
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "mount sdcard successed %s\n", stMountDev);
    }
    
    return 0;
}

static void* app_ipcam_SdCard_Proc(void *args)
{
    int iRet = 0;
    int bInsertSdCard = 0;

    s_stSdCardStatus = SDCARD_STATUS_NOT_INSERT;

    app_ipcam_SdCard_UMount();
    while (s_stSdCardThread.bRun_flag)
    {
        if(bInsertSdCard != app_ipcam_SdCard_CheckInsert())
        {
            bInsertSdCard = !bInsertSdCard;
            if(bInsertSdCard)
            {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "Sdcard insert status\n");
                iRet = app_ipcam_SdCard_CheckMount();
                if (0 != iRet)
                {
                    iRet = app_ipcam_SdCard_Mount();
                    if(iRet == 0)
                    {
                        s_stSdCardStatus = SDCARD_STATUS_NORMAL;
                    }
                }
                else
                {
                    s_stSdCardStatus = SDCARD_STATUS_NORMAL;
                }
                if(s_stSdCardStatus)
                {
                    //record init
                }
            }
            else
            {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "Sdcard not insert status\n");
                app_ipcam_SdCard_UMount();
                //record unit
                s_stSdCardStatus = SDCARD_STATUS_NOT_INSERT;

            }
        }
        sleep(1);
    }

    return NULL;
}

int app_ipcam_SdCard_Init()
{
    int iRet = 0;
    memset(&s_stSdCardThread, 0, sizeof(s_stSdCardThread));
    s_stSdCardThread.bRun_flag = 1;
    iRet = pthread_create(&s_stSdCardThread.mRun_PID, NULL, app_ipcam_SdCard_Proc, (void *)&s_stSdCardThread);
    if (iRet)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_SdCard_Proc pthread_create failed:0x%x\n", iRet);
    }
    return iRet;
}

int app_ipcam_SdCard_UnInit()
{
    s_stSdCardThread.bRun_flag = 0;
    if (s_stSdCardThread.mRun_PID != 0)
    {
        pthread_join(s_stSdCardThread.mRun_PID, NULL);
        s_stSdCardThread.mRun_PID = 0;
    }
    return 0;
}

int app_ipcam_SdCard_Format()
{
    char cmd[128] = {0};
    app_ipcam_SdCard_UMount();
    snprintf(cmd, sizeof(cmd), "mkfs.vfat " SDCARD_DEV_PARTITION_NAME, 1);
    system(cmd);
    app_ipcam_SdCard_Mount();
    return 0;
}

sdcard_status_e ts_sdcard_getStatus()
{
    return s_stSdCardStatus;
}
