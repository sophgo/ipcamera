install:
	@mkdir -p $(SRCTREE)/install
	@cp -rf $(TARGET) $(SRCTREE)/install
#tiger.bmp
ifeq ($(CONFIG_RESOURCE_INSTALL_BITMAP), y)
	@cp -f $(APP_RESOURCE_DIR)/bitmap/tiger.bmp $(APP_INSTALL_DIR)
endif
#CV186AH AI MODEL
ifeq ($(CONFIG_MODULE_AI),y)
	@cp -f $(TDL_PATH)/install/SOPHON/configs/model/model_factory.json $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/cls_attribute_gender_age_glass_emotion_112_112_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_FEATURE_CLIP_IMAGE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/feature_clip_image_224_224_W4BF16_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_FEATURE_CLIP_TEXT),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/feature_clip_text_1_77_W4BF16_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_FEATURE_CVIFACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/feature_cviface_112_112_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_KEYPOINT_FACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/keypoint_face_v2_64_64_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_SCRFD_DET_FACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/scrfd_det_face_432_768_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_TRACKING_FEARTRACK),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/tracking_feartrack_128_128_256_256_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_YOLOV8N_DET_MONITOR_PERSON),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/yolov8n_det_monitor_person_256_448_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_CV186X_YOLOV8N_DET_PERSON_VEHICLE),y)
	@mkdir -p $(APP_INSTALL_DIR)/cv186x/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/cv186x/yolov8n_det_person_vehicle_384_640_INT8_cv186x.bmodel $(APP_INSTALL_DIR)/cv186x/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/cls_attribute_gender_age_glass_emotion_112_112_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_FEATURE_CLIP_IMAGE),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/feature_clip_image_224_224_W4BF16_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_FEATURE_CLIP_TEXT),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/feature_clip_text_1_77_W4BF16_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_FEATURE_CVIFACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/feature_cviface_112_112_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_KEYPOINT_FACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/keypoint_face_v2_64_64_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_SCRFD_DET_FACE),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/scrfd_det_face_432_768_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_TRACKING_FEARTRACK),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/tracking_feartrack_128_128_256_256_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_YOLOV8N_DET_MONITOR_PERSON),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/yolov8n_det_monitor_person_256_448_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_AI_BM1688_YOLOV8N_DET_PERSON_VEHICLE),y)
	@mkdir -p $(APP_INSTALL_DIR)/bm1688/
	@cp -f $(APP_RESOURCE_DIR)/ai_models/bm1688/yolov8n_det_person_vehicle_384_640_INT8_bm1688.bmodel $(APP_INSTALL_DIR)/bm1688/
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
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_2OS04E10_2OV9282),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_2os04e10_2ov9282.ini $(APP_INSTALL_DIR)/param_config.ini
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
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_2OV9282_DPU),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_ov9282_ov9282_dpu.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_2OV9282_4OS05A20),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_2ov9282_4os05a20.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_2SC438AI_3SC1330),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_2sc438ai_3sc1330.ini $(APP_INSTALL_DIR)/param_config.ini
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FBM_CV186AH_WEVB_AUTOTEST),y)
	@cp -rf $(APP_RESOURCE_DIR)/parameter/fbm/cv186ah_wevb_autotest.ini $(APP_INSTALL_DIR)/param_config.ini
endif
#VDEC
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES_H264),y)
	@cp -rf $(APP_RESOURCE_DIR)/vdec/1080p.h264 $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES_H265),y)
	@cp -rf $(APP_RESOURCE_DIR)/vdec/enc-1-1.265 $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES_JPG),y)
	@cp -rf $(APP_RESOURCE_DIR)/vdec/1080p.jpg $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_VDEC_FILES_MJP),y)
	@cp -rf $(APP_RESOURCE_DIR)/vdec/1080p.mjp $(APP_INSTALL_DIR)
endif
#DPU
ifeq ($(CONFIG_RESOURCE_INSTALL_DPU_GRIDINFO_L),y)
	@cp -rf $(APP_RESOURCE_DIR)/dpu/gridinfoL.dat $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_DPU_GRIDINFO_R),y)
	@cp -rf $(APP_RESOURCE_DIR)/dpu/gridinfoR.dat $(APP_INSTALL_DIR)
endif
#FISHEYE
ifeq ($(CONFIG_RESOURCE_INSTALL_FISHEYE_GRIDINFO_L),y)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/L_grid_info_68_68_4624_70_70_dst_2240x2240_src_2240x2240.dat $(APP_INSTALL_DIR)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/Lgrid_info_bev_128_64_4078_128_64_dst_4096x2048_src_2240x2240.dat $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FISHEYE_GRIDINFO_R),y)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/R_grid_info_68_68_4624_70_70_dst_2240x2240_src_2240x2240.dat $(APP_INSTALL_DIR)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/Rgrid_info_bev_128_64_4726_128_64_dst_4096x2048_src_2240x2240.dat $(APP_INSTALL_DIR)
endif
ifeq ($(CONFIG_RESOURCE_INSTALL_FISHEYE_STITCH_BIN),y)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/alpha_4096_2048.bin $(APP_INSTALL_DIR)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/beta_4096_2048.bin $(APP_INSTALL_DIR)
	@cp -rf $(APP_RESOURCE_DIR)/fisheye/grid_info_bev_32_24_768_32_24_dst_1024x768_src_4096x2048.dat $(APP_INSTALL_DIR)
endif
