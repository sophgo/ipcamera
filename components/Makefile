include $(SRCTREE)/build/rule.mk

ifeq ($(SDK_VER), 64bit)
	ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM64_V930), y)
		CROSS_COMPILE = $(SRCTREE)/../host-tools/gcc/gcc-buildroot-9.3.0-aarch64-linux-gnu/bin/aarch64-linux-
	else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM64_V1131), y)
		CROSS_COMPILE = $(SRCTREE)/../host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
	else
		CROSS_COMPILE = $(SRCTREE)/../host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
	endif
else ifeq ($(SDK_VER), 32bit)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
else ifeq ($(SDK_VER), glibc_riscv64)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
else ifeq ($(SDK_VER), musl_riscv64)
	CROSS_COMPILE := $(SRCTREE)/../host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
endif

TOPTARGETS := all
module-$(CONFIG_MODULE_RTSP)		+= osal
module-$(CONFIG_MODULE_RTSP)		+= rtsp
module-$(CONFIG_MODULE_RTSP)		+= ringbuffer
module-$(CONFIG_MODULE_RECORD)		+= ringbuffer
module-$(CONFIG_MODULE_MBUF)        += mbuf

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean :
	$(MAKE) -C osal clean
	$(MAKE) -C rtsp clean
	$(MAKE) -C ringbuffer clean
	$(MAKE) -C mbuf clean

clean_all : clean
	$(MAKE) -C osal clean_all
	$(MAKE) -C rtsp clean_all
	$(MAKE) -C ringbuffer clean_all
	$(MAKE) -C mbuf clean_all

install :

.PHONY: $(TOPTARGETS) $(SUBDIRS)
