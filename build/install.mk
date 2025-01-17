install:
	@mkdir -p $(SRCTREE)/install
	@cp -rf $(TARGET) $(SRCTREE)/install
#tiger.bmp
ifeq ($(CONFIG_RESOURCE_INSTALL_BITMAP), y)
	@cp -f $(APP_RESOURCE_DIR)/bitmap/tiger.bmp $(APP_INSTALL_DIR)
endif
#CV186AH AI MODEL
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_MOBILEDETV2_PEDESTRIAN_D0_384),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/mobiledetv2-pedestrian-d0-384_cv186x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_MOBILEDETV2_PEDESTRIAN_D0_448),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/mobiledetv2-pedestrian-d0-448_cv186x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_MOBILEDETV2_PEDESTRIAN_D1_896),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/mobiledetv2-pedestrian-d1-896_cv186x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_MOBILEDETV2_PEDESTRIAN_D1_LS_896),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/mobiledetv2-pedestrian-d1-ls-896_cv186x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_CVIFACE_V6_S_MIX),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/cviface-v6-s_mix_cv186x.bmodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_HAND_DET_QAT_640X384_INT8),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/hand_det_qat_640x384_int8_cv186x.bmodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_HAND_KPT_128X128_INT8),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/hand_kpt_128x128_int8_cv186x.bmodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_HAND_KPT_CLS9_INT8),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/hand_kpt_cls9_int8_cv186x.bmodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_SCRFD_500M_BNKPS_432X768_INT8),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/186x/scrfd_500m_bnkps_432_768_int8_cv186x.bmodel $(APP_INSTALL_DIR)
endif
#FILE  RECOVRY
ifeq ($(CONFIG_RESOURCE_INSTALL_H264_PCM_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/h264_pcm_template.bin $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_H265_PCM_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/h265_pcm_template.bin $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_MP4_AAC_TEMPLATE),y)
	@cp -f $(APP_RESOURCE_DIR)/file_recover/mp4_aac_template.bin $(APP_INSTALL_DIR)
endif
#NETWORK
ifeq ($(CONFIG_RESOURCE_INSTALL_NETWORK_WWW),y)
	@cp -rf $(APP_RESOURCE_DIR)/www $(APP_INSTALL_DIR)
endif
#PARAM
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_SC4336P_SC4336P),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_sc4336p_sc4336p.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_OS04E10_OS04E10),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_os04e10_os04e10.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_2OS04E10_2SC4336P),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_2os04e10_2sc4336p.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_GC4653_MULITRECORD), y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_gc4653_mulitrecord.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_GC4653_AI), y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_gc4653_ai.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_6IMX307),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_6imx307.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_AUTOTEST),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_autotest.ini $(APP_INSTALL_DIR)/param_config.ini
endif