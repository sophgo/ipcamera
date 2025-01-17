include $(SRCTREE)/build/rule.mk

ifeq ($(SDK_VER), 64bit)
	CROSS_COMPILE = $(SRCTREE)/../host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
else ifeq ($(SDK_VER), 32bit)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
else ifeq ($(SDK_VER), glibc_riscv64)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
else ifeq ($(SDK_VER), musl_riscv64)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
endif

TOPTARGETS := all
module-$(CONFIG_MODULE_RTSP)		+= cvi_osal
module-$(CONFIG_MODULE_RTSP)		+= cvi_rtsp
module-$(CONFIG_MODULE_RTSP)		+= ringbuffer
module-$(CONFIG_MODULE_RECORD)		+= ringbuffer
module-$(CONFIG_MODULE_MBUF)        += mbuf

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean :
	$(MAKE) -C cvi_osal clean
	$(MAKE) -C cvi_rtsp clean
	$(MAKE) -C ringbuffer clean
	$(MAKE) -C mbuf clean

install :

.PHONY: $(TOPTARGETS) $(SUBDIRS)
