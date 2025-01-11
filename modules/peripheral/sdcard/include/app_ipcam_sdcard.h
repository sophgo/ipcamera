#ifndef __APP_IPCAM_SDCARD_H__
#define __APP_IPCAM_SDCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SDCARD_DEV_MAIN_PARTITION_NAME            "/dev/mmcblk0"
#define SDCARD_DEV_PARTITION_NAME                 SDCARD_DEV_MAIN_PARTITION_NAME "p%d"
#define SDCARD_MOUNT_PATH                         "/mnt/sd"


typedef enum{
    SDCARD_STATUS_NOT_INSERT,
    SDCARD_STATUS_NOT_INIT,
    SDCARD_STATUS_INIT_CHECK,
    SDCARD_STATUS_NORMAL,
    SDCARD_STATUS_FORMAT,
    SDCARD_STATUS_ERROR
} sdcard_status_e;

int app_ipcam_SdCard_Init();

int app_ipcam_SdCard_UnInit();


int app_ipcam_SdCard_Format();

sdcard_status_e ts_sdcard_getStatus();

#ifdef __cplusplus
};
#endif

#endif

