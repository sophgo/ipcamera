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

ifeq ($(CONFIG_MODULE_TDL),y)
	@cp -f $(TDL_PATH)/install/$(SOC_SEGMENT)/configs/model/model_factory.json $(APP_INSTALL_DIR)
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV180X_YOLOV8N_DET_MONITOR_PERSON),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv180x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/180x/yolov8n_det_monitor_person_256_448_INT8_cv180x.cvimodel $(APP_INSTALL_DIR)/cv180x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_APP_CJSON),y)
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/face_pet_cap_app.json $(APP_INSTALL_DIR)
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_FEATURE_CVIFACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/feature_cviface_112_112_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_KEYPOINT_FACE_V2),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/keypoint_face_v2_64_64_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_KEYPOINT_YOLOV8POSE_PERSON),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/keypoint_yolov8pose_person17_384_640_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_YOLOV8N_DET_FACE_HAND_PERSON_PET),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/yolov8n_det_face_head_person_pet_384_640_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/cls_attribute_gender_age_glass_emotion_112_112_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_YOLOV8N_DET_MONITOR_PERSON),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/yolov8n_det_monitor_person_256_448_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_SCRFD_DET_FACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/scrfd_det_face_432_768_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_FEATURE_CVIFACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/feature_cviface_112_112_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_SOUND_BABAY_CRY),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/cls_sound_babay_cry_188_40_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_TRACKING_FEARTACK),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/tracking_feartrack_128_128_256_256_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
endif

ifeq ($(CONFIG_RESOURCE_INSTALL_TDL_CV181X_YOLOV8N_DET_PERSON_VEHICLE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv181x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/181x/yolov8n_det_person_vehicle_384_640_INT8_cv181x.cvimodel $(APP_INSTALL_DIR)/cv181x/
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

