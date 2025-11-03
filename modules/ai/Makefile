include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-$(CONFIG_MODULE_TDL_MOTION)			    += motion
module-$(CONFIG_MODULE_TDL_CAPTURE)			    += capture
module-$(CONFIG_MODULE_TDL_MD)					+= md
module-$(CONFIG_MODULE_TDL_PD)					+= pd
module-$(CONFIG_MODULE_TDL_FD_FACE)				+= face
module-$(CONFIG_MODULE_TDL_HUMAN_KEYPOINT)		+= human_keypoint_detect
module-$(CONFIG_MODULE_TDL_SOUND_CLS)		    += baby_cry

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
