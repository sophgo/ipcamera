install:
	@mkdir -p $(SRCTREE)/install
	@cp -rf $(TARGET) $(SRCTREE)/install
#BITMAP
ifeq ($(CONFIG_RESOURCE_INSTALL_BITMAP), y)
	@cp -f $(APP_RESOURCE_DIR)/bitmap/tiger.bmp $(APP_INSTALL_DIR)
endif
#VDEC
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES), y)
	@cp -rpf $(APP_RESOURCE_DIR)/vdec/* $(APP_INSTALL_DIR)
endif
#AI
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV180X_MOBILEDETV2_PEDESTRIAN_D0_LS_384),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/180x/mobiledetv2-pedestrian-d0-ls-384.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV180X_MOBILEDETV2_PEDESTRIAN_D0_LS_448),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/180x/mobiledetv2-pedestrian-d0-ls-448.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_CVIFACE_V5),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/cviface-v5-s.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_HAND_DET_QAT_640X360),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/hand_det_qat_640x384.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_HAND_KPT_128X128),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/hand_kpt_128x128.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_HAND_KPT_CLS9),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/hand_kpt_cls9.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_HARDHAT_720_1280),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/hardhat_720_1280.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_IR_RECOGITION),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/ir_recogition.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_LIVENESS),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/liveness.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_MOBILEDETV2_PEDESTRIAN_D0_LS_384),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/mobiledetv2-pedestrian-d0-ls-384.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_MOBILEDETV2_PEDESTRIAN_D0_LS_448),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/mobiledetv2-pedestrian-d0-ls-448.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_MOBILEDETV2_PEDESTRIAN_D1_LS),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/mobiledetv2-pedestrian-d1-ls.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_PIPNET_MBV3_64_V5_2),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/pipnet_mbv3_64_v5_cv181x_2.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_RETINAFACE_MASK),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/retinaface_mask.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_SCRFD_320X256),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/scrfd_320_256.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_SCRFD_500M_BNKPS_432X768),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/scrfd_500m_bnkps_432_768.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_SOUND_IPC_SR8K_V14),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/sound_ipc_sr8k_v14_cv181x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_SOUND_IPC_SR8K_V17),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/sound_ipc_sr8k_v17_cv181x.cvimodel $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV181X_YOLOV8N_HEADPERSON_KPT),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/yolov8n_headperson_kpt_cv181x.cvimodel $(APP_INSTALL_DIR)
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
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV180ZB_WEVB_GC2053),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv180zb_wevb_gc2053.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV180ZB_WEVB_SC2336_AUTOTEST),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv180zb_wevb_sc2336_autotest.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1810C_WEVB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1801c_wevb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1810C_WEVB_GC1084_GC1084),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1810c_wevb_gc1084_gc1084.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811C_WDMB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811c_wdmb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811C_WDMB_OV5647),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811c_wdmb_ov5647.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811C_WEVB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811c_wevb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811C_WEVB_SC3336_SC3336),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811c_wevb_sc3336_sc3336 $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811H_WEVB_GC2053_GC2053),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811h_wevb_gc2053_gc2053.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811H_WEVB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811h_wevb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811H_WEVB_GC4653_PANEL),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811h_wevb_gc4653_panel.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1812CP_WEVB_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1812cp_wevb_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1812H_WEVB_GC4653_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1812h_wevb_gc4653_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV1811H_WEVB_GC2093_GC2053_AUTOTEST),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv1811h_wevb_gc2093_gc2053_autotest.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV180ZB_GC2064_PANEL),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv180zb_gc2063_panel.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV180ZB_GC2063),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv180zb_gc2063.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV180ZB_SC_3336),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv180zb_sc3336.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_cb1800B_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1800b_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1801C_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1801c_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1810C_GC4653_PANEL),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1810c_gc4653_panel.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1810C_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1810c_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1811C_GC4653_AUDIO_ORDER),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1811c_gc4653_audio_order.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1811C_GC4653_CONSUMER_COUNTING),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1811c_gc4653_consumer_counting.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1811C_GC4653_HD),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1811c_gc4653_hd.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1811C_GC4653_PANEL),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1811c_gc4653_panel.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1811C_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1811c_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_SBM_CV1812H_GC4653),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/sbm/cv1812h_gc4653.ini $(APP_INSTALL_DIR)/param_config.ini
endif


