include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-$(CONFIG_MODULE_FRMBUF_LVGL)					+= frmbuf_lvgl
module-$(CONFIG_MODULE_FRMBUF_DISP)					+= frmbuf_disp
module-$(CONFIG_MODULE_FRMBUF)						+= frmbuf

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
