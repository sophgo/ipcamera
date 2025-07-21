ifeq ("$(STORAGE_TYPE)", "")
	FLASH_PARTITION_XML := $(BUILD_PATH)/boards/default/partition/partition_none.xml
else

ifeq ($(CONFIG_SKIP_UBOOT),y)
ifeq ($(CONFIG_MEDIA_SUPPORT_PROC),y)
	FLASH_PARTITION_XML := $(BUILD_PATH)/boards/$(shell echo $(CHIP_ARCH) | tr 'A-Z' 'a-z')$(BUILD_CHIP_ARCH)/$(PROJECT_FULLNAME)/partition/partition_fastboot_debug_$(STORAGE_TYPE).xml
else
	FLASH_PARTITION_XML := $(BUILD_PATH)/boards/$(shell echo $(CHIP_ARCH) | tr 'A-Z' 'a-z')$(BUILD_CHIP_ARCH)/$(PROJECT_FULLNAME)/partition/partition_fastboot_$(STORAGE_TYPE).xml
endif
else
	FLASH_PARTITION_XML := $(BUILD_PATH)/boards/$(shell echo $(CHIP_ARCH) | tr 'A-Z' 'a-z')$(BUILD_CHIP_ARCH)/$(PROJECT_FULLNAME)/partition/partition_$(STORAGE_TYPE).xml
endif
endif

# PACK_PARAM
define pack_param
	@dd if=${1} of=$(APP_INSTALL_DIR)/rawimages/param.bin conv=sync ibs=61440
	@python3 $(COMMON_TOOLS_PATH)/image_tool/packpq.py -i $(APP_INSTALL_DIR)/rawimages/param.bin -o $(OUTPUT_DIR)/rawimages/param.bin
	@python3 $(COMMON_TOOLS_PATH)/image_tool/raw2cimg.py $(OUTPUT_DIR)/rawimages/param.bin $(OUTPUT_DIR) $(FLASH_PARTITION_XML)
	@if [ -e $(OUTPUT_DIR)/param.bin ];then cp -rf $(OUTPUT_DIR)/param.bin $(APP_INSTALL_DIR)/; fi
endef

install:
	@mkdir -p $(APP_INSTALL_DIR)
	@mkdir -p $(APP_INSTALL_DIR)/rawimages
	@cp -rf $(TARGET) $(APP_INSTALL_DIR)
# BITMAP
ifeq ($(CONFIG_RESOURCE_INSTALL_BITMAP), y)
	@cp -f $(APP_RESOURCE_DIR)/bitmap/tiger.bmp $(APP_INSTALL_DIR)
endif
# VDEC
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES), y)
	@cp -rpf $(APP_RESOURCE_DIR)/vdec/* $(APP_INSTALL_DIR)
endif
# AI
#ADD CV184X MODEL
# FILE  RECOVRY
ifeq ($(CONFIG_RESOURCE_INSTALL_H264_PCM_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/h264_pcm_template.bin $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_H265_PCM_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/h265_pcm_template.bin $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_MP4_AAC_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/mp4_aac_template.bin $(APP_INSTALL_DIR)
endif
# NETWORK
ifeq ($(CONFIG_RESOURCE_INSTALL_NETWORK_WWW),y)
	@cp -rf $(APP_RESOURCE_DIR)/www $(APP_INSTALL_DIR)
endif
# PARAM
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1842HP_WEVB_CV2003),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1842hp_wevb_cv2003.ini $(APP_INSTALL_DIR)/param_config.ini
	@$(call pack_param ,$(APP_RESOURCE_DIR)/parameter/fbm/cv1842hp_wevb_cv2003.ini)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1842HP_WEVB_CV2003_MIPI_SWITCH),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1842hp_wevb_cv2003_mipi_switch.ini $(APP_INSTALL_DIR)/param_config.ini
	@$(call pack_param ,$(APP_RESOURCE_DIR)/parameter/fbm/cv1842hp_wevb_cv2003_mipi_switch.ini)
endif
