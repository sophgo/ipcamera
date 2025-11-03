#ifndef __APP_IPCAM_PARAM_PARSE_H__
#define __APP_IPCAM_PARAM_PARSE_H__

#include "linux/cvi_common.h"
#include "linux/cvi_comm_video.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_sys.h"

#ifdef MPI_VI_MODULE_SUPPORT
#include "app_ipcam_vi.h"
#endif

#ifdef MPI_VPSS_MODULE_SUPPORT
#include "app_ipcam_vpss.h"
#endif

#ifdef MPI_OSD_MODULE_SUPPORT
#include "app_ipcam_osd.h"
#endif

#ifdef MPI_VENC_MODULE_SUPPORT
#include "app_ipcam_venc.h"
#endif

#ifdef MPI_VO_MODULE_SUPPORT
#include "app_ipcam_vo.h"
#endif

#ifdef RTSP_SUPPORT
#include "app_ipcam_rtsp.h"
#endif

#ifdef MPI_VDEC_MODULE_SUPPORT
#include "app_ipcam_vdec.h"
#endif

#ifdef FRMBUF
#include "app_ipcam_frmbuf.h"
#endif

#ifdef VDEC_SOFT_SUPPORT
#include "app_ipcam_vdec_soft.h"
#endif

#ifdef FRMBUF_DISP_SUPPORT
#include "app_ipcam_frmbuf_disp.h"
#endif

#ifdef FRMBUF
#include "app_ipcam_frmbuf.h"
#endif

#ifdef MPI_AUDIO_MODULE_SUPPORT
#include "app_ipcam_audio.h"
#endif

#ifdef TDL_SUPPORT
#include "app_ipcam_ai.h"
#endif

#ifdef RECORD_SUPPORT
#include "app_ipcam_record.h"
#endif

#ifdef MPI_STITCH_MODULE_SUPPORT
#include "app_ipcam_stitch.h"
#endif

#ifdef FRMBUF_LVGL_SUPPORT
#include "app_ipcam_frmbuf_lvgl.h"
#endif

#ifdef MPI_GDC_MODULE_SUPPORT
#include "app_ipcam_gdc.h"
#endif

#ifdef BLACKLIGHT_SUPPORT
#include "app_ipcam_blacklight.h"
#endif

#ifdef PANEL_SUPPORT
#include "app_ipcam_panel.h"
#endif

#ifdef IRCUT_MODULE_SUPPORT
#include "app_ipcam_ircut.h"
#endif

#ifdef SDCARD_SUPPORT
#include "app_ipcam_sdcard.h"
#endif

#ifdef MBUF_SUPPORT
#include "cvi_mbuf.h"
#endif

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

#ifdef LOG_SUPPORT
#include "cvi_log.h"
#endif

#ifdef CLOUD_SUPPORT
#include "plt_common_hal.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PARAM_STRING_LEN (64)
#define PARAM_STRING_NAME_LEN (128)

#define VI_FRAMERATE			15

int app_ipcam_Param_Convert_StrName_to_EnumNum(const char *str_name, const char *str_enum[], const int enum_upper_bound, int * const enum_num);
//common attribute
const char ** app_ipcam_Param_get_pixel_format();
const char ** app_ipcam_Param_get_data_bitwidth();
const char ** app_ipcam_Param_get_compress_mode();
const char ** app_ipcam_Param_get_vi_vpss_mode();
const char ** app_ipcam_Param_get_mode_id();
const char ** app_ipcam_Param_get_video_format();
const char ** app_ipcam_Param_get_payload_type();

int app_ipcam_Param_Load(void);

int app_ipcam_Opts_Parse(int argc, char *argv[]);

#ifdef TDL_SUPPORT
const char ** app_ipcam_Param_get_ai_supported_model();
#endif

#ifdef __cplusplus
}
#endif

#endif
