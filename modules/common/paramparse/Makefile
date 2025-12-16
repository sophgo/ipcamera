include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-y									+= paramparse_common
module-$(CONFIG_MODULE_MEDIA_SYS)			+= sys
module-$(CONFIG_MODULE_MEDIA_VI)			+= vi
module-$(CONFIG_MODULE_MEDIA_VPSS)			+= vpss
module-$(CONFIG_MODULE_MEDIA_VENC)			+= venc
module-$(CONFIG_MODULE_MEDIA_DEC)			+= vdec
module-$(CONFIG_MODULE_MEDIA_DECSOFT)		+= vdec_soft
module-$(CONFIG_MODULE_DISPLAY)				+= display
module-$(CONFIG_MODULE_RTSP)				+= rtsp
module-$(CONFIG_MODULE_MEDIA_OSD)			+= osd
module-$(CONFIG_MODULE_MEDIA_STITCH)		+= stitch
module-$(CONFIG_MODULE_MEDIA_GDC)			+= gdc
module-$(CONFIG_MODULE_MEDIA_BLACKLIGHT)	+= blacklight
module-$(CONFIG_MODULE_GPIO)				+= gpio
module-$(CONFIG_MODULE_PWM)					+= pwm
module-$(CONFIG_MODULE_FRMBUF)				+= frmbuf
module-$(CONFIG_MODULE_MEDIA_AUDIO)			+= audio
module-$(CONFIG_MODULE_TDL)			        += ai
module-$(CONFIG_MODULE_TDL_CAPTURE)		    += ai_capture
module-$(CONFIG_MODULE_TDL_FD_FACE)		    += ai_face
module-$(CONFIG_MODULE_TDL_PD)		        += ai_pd
module-$(CONFIG_MODULE_TDL_MD)		        += ai_md
module-$(CONFIG_MODULE_TDL_HUMAN_KEYPOINT)	+= ai_human_keypoint_detect
module-$(CONFIG_MODULE_TDL_MOTION)		    += ai_motion
module-$(CONFIG_MODULE_TDL_SOUND_CLS)	    += ai_babycry
module-$(CONFIG_MODULE_TDL_OBJECT_TRACK)	+= ai_object_track

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
