# Usage:
# local host: make
# riscv64 musl: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: make CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
COMMON_DIR ?= $(CURDIR)/../common
##
SDIR		:= $(CURDIR)/src
SRCS_C		:= $(SDIR)/rtsp.c
SRCS_C		+= $(SDIR)/network.c
SRCS_C		+= $(SDIR)/rtp.c
#
RINGBUFFER_DIR	?= $(CURDIR)/../ringbuffer
OSAL_DIR	?= $(CURDIR)/../osal
#
INCS		+= -I$(COMMON_DIR)/
INCS		+= -I$(SDIR)/inc
INCS		+= -I$(CURDIR)/include
INCS		+= -I$(RINGBUFFER_DIR)/include
INCS		+= -I$(OSAL_DIR)/include
#
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcomp_rtsp.a
TARGET_SO	:= $(TARGET_DIR)/libcomp_rtsp.so
# log level.
#CFLAGS		+= -DBUILT_LOG_LEVEL=CVI_LOG_DEBUG
#
include $(COMMON_DIR)/common_lib.mk
