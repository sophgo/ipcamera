-include $(TOP_DIR)/build/.config
-include $(SRCTREE)/.config
-include $(SRCTREE)/build/peripheral/panel.mk
#
CROSS_COMPILE ?=
#
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CXX             = $(CROSS_COMPILE)g++
CPP             = $(CC) -E
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
ARFLAGS         = rcs
LDFLAGS_SO      = -shared -fPIC
# riscv64-unknown-linux-musl, riscv64-unknown-linux-gnu, aarch64-linux-gnu, arm-linux-gnueabihf
export TARGET_MACHINE	:= $(shell ${CC} -dumpmachine)
#
APP_PREBUILT_DIR := $(SRCTREE)/prebuilt
APP_RESOURCE_DIR := $(SRCTREE)/resource
APP_INSTALL_DIR  := $(SRCTREE)/install
APP_COMPONENTS_DIR := $(SRCTREE)/components

#default set CONFIG_SDK_DIR = TOP_DIR
ifeq ($(CONFIG_SDK_DIR), )
	CONFIG_SDK_DIR := $(TOP_DIR)
endif

# tdl_sdk path
TDL_PATH := $(TOP_DIR)/tdl_sdk

# cvi_mpi path
MW_PATH := $(TOP_DIR)/cvi_mpi

# isp path
ISP_INC := $(MW_PATH)/modules/isp/include/$(SOC_NICK_NAME_LOWER)

# SensorSupportList path
SENSOR_LIST_INC := $(MW_PATH)/component/isp/common

# kernel path
KERNEL_PATH ?= $(TOP_DIR)/linux_5.10

# FFMPEG 6.0
FFMPEG_6_0_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg6.0/lib

#NET
ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/libmusl_riscv64
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/libmusl_riscv64
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/libglibc_riscv64
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/libglibc_riscv64
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/lib32bit
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/lib32bit
else ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/lib64bit
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/lib64bit
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

# FFMPEG
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/lib32bit
else ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/lib64bit
else
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/musl_riscv
endif

#OSAL
CVI_OSAL_DIR = $(APP_COMPONENTS_DIR)/cvi_osal

#RINGBUFFER
RINGBUFFER_DIR = $(APP_COMPONENTS_DIR)/ringbuffer

#RTSP
CVI_RTSP_DIR = $(APP_COMPONENTS_DIR)/cvi_rtsp

#OPENSSL
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/lib32bit
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/libglibc_riscv64
else
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/lib64bit
endif

#LVGL
LVGL_LIB_DIR = $(APP_PREBUILT_DIR)/lvgl/lib

ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
SDK_VER := 64bit
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
SDK_VER := 32bit
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
SDK_VER := glibc_riscv64
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
SDK_VER := musl_riscv64
endif

include $(SRCTREE)/build/dep.mk
include $(SRCTREE)/build/link.mk