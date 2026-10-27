SHELL = /bin/bash
SRCTREE := $(CURDIR)
TARGET_OUT_DIR := $(CURDIR)/out
TOPTARGETS := all clean install clean_all
TOPSUBDIRS := ipcamera

export SOC_SEGMENT := $(shell echo $(CHIP_SEGMENT) | tr a-z A-Z)
export SOC_SEGMENT_LOWER := $(shell echo $(SOC_SEGMENT) | tr A-Z a-z)

ifneq ($(filter CV186AH 88A2 BM1688, $(SOC_SEGMENT)),)
export SOC_NICK_NAME := CV186X
export SOC_NICK_NAME_LOWER := $(shell echo $(SOC_NICK_NAME) | tr A-Z a-z)
endif

$(info SOC_SEGMENT=$(SOC_SEGMENT))
$(info SOC_SEGMENT_LOWER=$(SOC_SEGMENT_LOWER))
$(info SOC_NICK_NAME=$(SOC_NICK_NAME))
$(info SOC_NICK_NAME_LOWER=$(SOC_NICK_NAME_LOWER))

## setup path ##
ifeq ($(filter CV186AH 88A2 BM1688, $(SOC_SEGMENT)),)
    $(error UNKNOWN chip series - $(SOC_SEGMENT))
endif

TMP_STRING = $(foreach f,$(TOPSUBDIRS),$(findstring $f,$(MAKECMDGOALS)))
PROJECT = $(strip $(TMP_STRING))
export SRCTREE PROJECT TARGET_OUT_DIR

$(PROJECT): .config
	$(MAKE) -C solutions/$@ $(strip $(subst $@,,$(MAKECMDGOALS)))


$(TOPSUBDIRS): $(PROJECT)

$(TOPTARGETS): $(TOPSUBDIRS)

#use defconfig cv186ah_autotest_defconfig
.config :
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild cv186ah_autotest_defconfig

#.PHONY: $(TOPTARGETS) $(TOPSUBDIRS)
all : $(SRCTREE)/.config $(SUBDIRS) $(TARGET)


menuconfig:
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild $@
%_defconfig:
	$(MAKE) -f $(SRCTREE)/build/Makefile.kbuild $@

include $(SRCTREE)/build/build.mk

RECORD_EXPORT_TARGET_MACHINE := $(shell $(CROSS_COMPILE)gcc -dumpmachine)
RECORD_EXPORT_COMPONENT_DIR := $(SRCTREE)/components/comps_install/$(RECORD_EXPORT_TARGET_MACHINE)/record
record_component: .config
	$(MAKE) -C modules/record
	@mkdir -p $(RECORD_EXPORT_COMPONENT_DIR)/lib $(RECORD_EXPORT_COMPONENT_DIR)/include
	@cp -f $(TARGET_OUT_DIR)/lib/libapp_recorder.a $(RECORD_EXPORT_COMPONENT_DIR)/lib/
	@cp -f $(TARGET_OUT_DIR)/lib/libapp_file_recover.a $(RECORD_EXPORT_COMPONENT_DIR)/lib/
	@cp -a modules/record/include/. $(RECORD_EXPORT_COMPONENT_DIR)/include/

.PHONY: $(SUBDIRS) record_component
