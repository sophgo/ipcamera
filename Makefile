SHELL = /bin/bash
SRCTREE := $(CURDIR)
TARGET_OUT_DIR := $(CURDIR)/out
TOPTARGETS := all clean install clean_all
TOPSUBDIRS := ipcamera


export SOC_SEGMENT := $(shell echo $(CHIP_SEGMENT) | tr a-z A-Z)
export SOC_SEGMENT_LOWER := $(shell echo $(SOC_SEGMENT) | tr A-Z a-z)

ifeq ($(SOC_SEGMENT), CV181X)
export SOC_NICK_NAME := CV181X
export SOC_NICK_NAME_LOWER := $(shell echo $(SOC_NICK_NAME) | tr A-Z a-z)
endif
ifeq ($(SOC_SEGMENT), CV180X)
export SOC_NICK_NAME := CV180X
export SOC_NICK_NAME_LOWER := $(shell echo $(SOC_NICK_NAME) | tr A-Z a-z)
endif

$(info SOC_SEGMENT=$(SOC_SEGMENT))
$(info SOC_NICK_NAME=$(SOC_NICK_NAME))
$(info SOC_NICK_NAME_LOWER=$(SOC_NICK_NAME_LOWER))

## setup path ##
ifeq ($(findstring $(SOC_SEGMENT), CV182X CV183X CV180X CV181X), )
	$(error UNKNOWN chip series - $(SOC_SEGMENT))
endif

TMP_STRING = $(foreach f,$(TOPSUBDIRS),$(findstring $f,$(MAKECMDGOALS)))
PROJECT = $(strip $(TMP_STRING))
export SRCTREE PROJECT TARGET_OUT_DIR

$(PROJECT): .config
	$(MAKE) -C solutions/$@ $(strip $(subst $@,,$(MAKECMDGOALS)))


$(TOPSUBDIRS): $(PROJECT)

$(TOPTARGETS): $(TOPSUBDIRS)

#Use defconfig cv181x_ipcamera_defconfig or cv180x_ipcamera_defconfig
.config :
ifeq ($(SOC_SEGMENT), CV181X)
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild cv181x_ipcamera_defconfig
else ifeq ($(SOC_SEGMENT), CV180X)
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild cv180x_ipcamera_defconfig
endif

#.PHONY: $(TOPTARGETS) $(TOPSUBDIRS)
all : $(SRCTREE)/.config $(SUBDIRS) $(TARGET)


menuconfig:
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild $@
%_defconfig:
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild $@

include $(SRCTREE)/build/build.mk

# What changed: Export record libraries and public headers into the target-specific component package.
RECORD_EXPORT_TARGET_MACHINE := $(shell $(CROSS_COMPILE)gcc -dumpmachine)
RECORD_EXPORT_COMPONENT_DIR := $(SRCTREE)/components/comps_install/$(RECORD_EXPORT_TARGET_MACHINE)/record
record_component: .config
	$(MAKE) -C modules/record
	@mkdir -p $(RECORD_EXPORT_COMPONENT_DIR)/lib $(RECORD_EXPORT_COMPONENT_DIR)/include
	@cp -f $(TARGET_OUT_DIR)/lib/libapp_recorder.a $(RECORD_EXPORT_COMPONENT_DIR)/lib/
	@cp -f $(TARGET_OUT_DIR)/lib/libapp_file_recover.a $(RECORD_EXPORT_COMPONENT_DIR)/lib/
	@cp -a modules/record/include/. $(RECORD_EXPORT_COMPONENT_DIR)/include/

.PHONY: $(SUBDIRS) record_component
