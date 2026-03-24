# VDEC_SOFT
DEFS-$(CONFIG_MODULE_MEDIA_DECSOFT) += -DVDEC_SOFT
INCS-$(CONFIG_MODULE_MEDIA_DECSOFT) += -I$(APP_PREBUILT_DIR)/ffmpeg6.0/include
INCS-y += -I$(MW_PATH)/include -I$(MW_PATH)/include/linux -I$(ISP_INC) -I$(MW_PATH)/include/isp/$(SOC_NICK_NAME_LOWER)
# DISPLAY
ifneq ($(SOC_SEGMENT), CV180X)
  INCS-$(CONFIG_MODULE_DISPLAY) += -I$(MW_PATH)/component/panel/$(SOC_NICK_NAME_LOWER)
  DEFS-$(CONFIG_MODULE_DISPLAY) += -DDISPLAY
endif

# SDCARD
DEFS-$(CONFIG_MODULE_SDCARD) += -DSDCARD_SUPPORT

# VDEC
DEFS-$(CONFIG_MODULE_MEDIA_DEC) += -DVDEC

# OSDC
DEFS-$(CONFIG_MODULE_MEDIA_OSD) += -DOSDC_SUPPORT

# FRMBUF
DEFS-$(CONFIG_MODULE_FRMBUF) += -DFRMBUF

# FRMBUF_DISP
DEFS-$(CONFIG_MODULE_FRMBUF_DISP) += -DFRMBUF_DISP

# FRMBUF_LVGL
DEFS-$(CONFIG_MODULE_FRMBUF_LVGL) += -DFRMBUF_LVGL
INCS-$(CONFIG_MODULE_FRMBUF_LVGL) += -I$(APP_PREBUILT_DIR)/lvgl/include

#AUDIO
DEFS-$(CONFIG_MODULE_MEDIA_AUDIO) += -DAUDIO_SUPPORT
ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y)
  DEFS-$(CONFIG_MODULE_MEDIA_AUDIO) += -DCVIAUDIO_STATIC
endif
DEFS-$(CONFIG_MODULE_AUDIO_MP3) += -DMP3_SUPPORT

#UAC
DEFS-$(CONFIG_MODULE_CVIUAC) += -DCVI_UAC_SUPPORT

#UVC
DEFS-$(CONFIG_MODULE_CVIUVC) += -DCVI_UVC_SUPPORT

#AI
INCS-$(CONFIG_MODULE_TDL)         += -I$(APP_PREBUILT_DIR)/jpegturbo/include
DEFS-$(CONFIG_MODULE_TDL)         += -DTDL_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_MD)      += -DTDL_MD_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_PD)      += -DTDL_PD_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_FD_FACE) += -DTDL_FD_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_HUMAN_KEYPOINT) += -DTDL_HUMAN_KEYPOINT_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_CAPTURE) += -DTDL_CAPTURE_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_MOTION)  += -DTDL_MOTION_SUPPORT
DEFS-$(CONFIG_MODULE_TDL_SOUND_CLS)  += -DTDL_SOUND_CLS
DEFS-$(CONFIG_MODULE_TDL_OBJECT_TRACK) += -DTDL_OBJECT_TRACK_SUPPORT
INCS-$(CONFIG_MODULE_TDL)+= -I$(TDL_PATH)/install/$(SOC_SEGMENT)/include/
INCS-$(CONFIG_MODULE_TDL)+= -I$(TDL_PATH)/install/$(SOC_SEGMENT)/include/c_apis
INCS-$(CONFIG_MODULE_TDL)+= -I$(TDL_PATH)/install/$(SOC_SEGMENT)/include/nn

#RTSP
INCS-$(CONFIG_MODULE_RTSP)    += -I$(APP_COMPONENTS_INSTALL_DIR)/rtsp/include
INCS-$(CONFIG_MODULE_RTSP)    += -I$(APP_COMPONENTS_INSTALL_DIR)/osal/include
DEFS-$(CONFIG_MODULE_RTSP)    += -DRTSP_SUPPORT
DEFS-$(CONFIG_RTSP_AUDIO_ENABLE) += -DRTSP_AUDIO_ENABLE

#PQTOOL
DEFS-$(CONFIG_MODULE_PQTOOL) += -DSUPPORT_ISP_PQTOOL

DEFS-$(CONFIG_MULTI_PROCESS_SUPPORT) += -DRPC_MULTI_PROCESS

#RECORD
DEFS-$(CONFIG_MODULE_RECORD) += -DRECORD_SUPPORT
INCS-$(CONFIG_MODULE_RECORD) += -I$(APP_COMPONENTS_INSTALL_DIR)/ringbuffer/include
INCS-$(CONFIG_MODULE_RECORD) += -I$(APP_PREBUILT_DIR)/ffmpeg/include

#STITCH
DEFS-$(CONFIG_MODULE_MEDIA_STITCH) += -DSTITCH_SUPPORT

#GDC
DEFS-$(CONFIG_MODULE_MEDIA_GDC) += -DGDC_SUPPORT

#BLACKLIGHT
DEFS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT) += -DBLACKLIGHT_SUPPORT
INCS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT) += -I$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_ive_sdk/ex_include

# websockets
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/thttpd/include
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets/protocols
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets/transports
DEFS-$(CONFIG_MODULE_NETWORK) += -DWEB_SOCKET
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/openssl/include/

# cloud
INCS-$(CONFIG_MODULE_AKYCLOUD)    += -I$(APP_PREBUILT_DIR)/cloud/akysmart/include
DEFS-$(CONFIG_MODULE_CLOUD)       += -DCLOUD_SUPPORT
DEFS-$(CONFIG_MODULE_AKYCLOUD)    += -DAKYCLOUD_SUPPORT

# SENSOR
INCS-y += -I$(SENSOR_LIST_INC)
include $(SENSOR_LIST_INC)/Kbuild
DEFS-y += $(KBUILD_DEFINES)

GDB_DEBUG = 0
ifeq ($(GDB_DEBUG), 1)
  CFLAGS += -g -O0
endif

#module include
INCS-$(CONFIG_MODULE_COMMON)			  += -I$(SRCTREE)/modules/common/include
INCS-$(CONFIG_MODULE_MEDIA_SYS)			+= -I$(SRCTREE)/modules/media/include/sys
INCS-$(CONFIG_MODULE_MEDIA_VI)			+= -I$(SRCTREE)/modules/media/include/vi
INCS-$(CONFIG_MODULE_MEDIA_VPSS)		+= -I$(SRCTREE)/modules/media/include/vpss
INCS-$(CONFIG_MODULE_MEDIA_VENC)		+= -I$(SRCTREE)/modules/media/include/venc
INCS-$(CONFIG_MODULE_MEDIA_OSD)			+= -I$(SRCTREE)/modules/media/include/osd
INCS-$(CONFIG_MODULE_MEDIA_VO)			+= -I$(SRCTREE)/modules/media/include/vo
INCS-$(CONFIG_MODULE_MEDIA_AUDIO)		+= -I$(SRCTREE)/modules/media/include/audio
INCS-$(CONFIG_MODULE_MEDIA_DEC)			+= -I$(SRCTREE)/modules/media/include/vdec
INCS-$(CONFIG_MODULE_MEDIA_DECSOFT)	+= -I$(SRCTREE)/modules/media/include/vdecsoft
INCS-$(CONFIG_MODULE_MEDIA_STITCH)	+= -I$(SRCTREE)/modules/media/include/stitch
INCS-$(CONFIG_MODULE_MEDIA_GDC)		 	+= -I$(SRCTREE)/modules/media/include/gdc
INCS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT)		+= -I$(SRCTREE)/modules/media/include/blacklight
INCS-$(CONFIG_MODULE_PARAMPARSE)	+= -I$(SRCTREE)/modules/common/paramparse/include
INCS-$(CONFIG_MODULE_TDL)				  += -I$(SRCTREE)/modules/ai/include
INCS-$(CONFIG_MODULE_CVIUAC)			+= -I$(SRCTREE)/modules/protocol/cvi_uac/include
INCS-$(CONFIG_MODULE_CVIUVC)			+= -I$(SRCTREE)/modules/protocol/cvi_uvc/include
INCS-$(CONFIG_MODULE_NETWORK)			+= -I$(SRCTREE)/modules/protocol/network/include
INCS-$(CONFIG_MODULE_RTSP)				+= -I$(SRCTREE)/modules/protocol/rtsp/include
INCS-$(CONFIG_MODULE_GPIO)				+= -I$(SRCTREE)/modules/peripheral/gpio/include
INCS-$(CONFIG_MODULE_PWM)				  += -I$(SRCTREE)/modules/peripheral/pwm/include
INCS-$(CONFIG_MODULE_SDCARD)		  += -I$(SRCTREE)/modules/peripheral/sdcard/include
INCS-$(CONFIG_MODULE_IRCUT)				+= -I$(SRCTREE)/modules/peripheral/ircut/include
INCS-$(CONFIG_MODULE_RECORD)			+= -I$(SRCTREE)/modules/record/include/record
INCS-$(CONFIG_MODULE_RECORD)			+= -I$(SRCTREE)/modules/record/include/file_recover
INCS-$(CONFIG_MODULE_DISPLAY)			+= -I$(SRCTREE)/modules/display/include
INCS-$(CONFIG_MODULE_DISPLAY)			+= -I$(SRCTREE)/modules/peripheral/panel/include
INCS-$(CONFIG_MODULE_FRMBUF)			+= -I$(SRCTREE)/modules/framebuffer/include/frmbuf
INCS-$(CONFIG_MODULE_FRMBUF_DISP)	+= -I$(SRCTREE)/modules/framebuffer/include/frmbuf_disp
INCS-$(CONFIG_MODULE_FRMBUF_LVGL) += -I$(SRCTREE)/modules/framebuffer/include/frmbuf_lvgl
INCS-$(CONFIG_MODULE_CLOUD)       += -I$(SRCTREE)/modules/cloud/hal_plt/include


INCS += $(INCS-y)
TARGETFLAGS += $(INCS)
TARGETFLAGS += -Os -Wl,--gc-sections -rdynamic

ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  CFLAGS		  += -MMD -Os -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
  TARGETFLAGS += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  CFLAGS		  += -MMD -Os -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
  TARGETFLAGS += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
else ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

ifeq ("$(SOC_SEGMENT)", "CV181X")
  CFLAGS += -D__CV181X__
endif
ifeq ("$(SOC_SEGMENT)", "CV180X")
  CFLAGS += -D__CV180X__
endif

CFLAGS += -std=gnu11 -g -Wall -Wextra -Werror -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu arm-none-linux-musleabihf),)
  CFLAGS += -mno-ldd
endif
CFLAGS += $(DEFS-y)
CFLAGS += -Wno-unused-parameter
