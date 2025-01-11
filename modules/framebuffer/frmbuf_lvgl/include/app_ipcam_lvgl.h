#ifndef __APP_IPCAM_LVGL_H__
#define __APP_IPCAM_LVGL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct APP_PARAM_LVGL_CFG_T {
    lv_indev_t *indev_mouse;
    RUN_THREAD_PARAM stThrParam;
}APP_PARAM_LVGL_CFG_S;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif