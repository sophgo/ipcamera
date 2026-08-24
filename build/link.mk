LIBS += -L$(TARGET_OUT_DIR)/lib/

ifeq ($(CONFIG_MODULE_PARAMPARSE), y)
LIBS-y += -Wl,--whole-archive
LIBS-$(CONFIG_MODULE_MEDIA_SYS)                   += -lapp_paramparse_sys
LIBS-$(CONFIG_MODULE_MEDIA_VI)                    += -lapp_paramparse_vi
LIBS-$(CONFIG_MODULE_MEDIA_VPSS)                  += -lapp_paramparse_vpss
LIBS-$(CONFIG_MODULE_MEDIA_VENC)                  += -lapp_paramparse_venc
LIBS-$(CONFIG_MODULE_MEDIA_OSD)                   += -lapp_paramparse_osd
LIBS-$(CONFIG_MODULE_MEDIA_DEC)                   += -lapp_paramparse_vdec
LIBS-$(CONFIG_MODULE_MEDIA_DECSOFT)               += -lapp_paramparse_vdecsoft
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)                 += -lapp_paramparse_audio
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)                += -lapp_paramparse_stitch
LIBS-$(CONFIG_MODULE_MEDIA_GDC)                   += -lapp_paramparse_gdc
LIBS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT)            += -lapp_paramparse_blacklight
LIBS-$(CONFIG_MODULE_TDL_OBJECT_TRACK)            += -lapp_paramparse_ai_object_track
LIBS-$(CONFIG_MODULE_TDL_HUMAN_KEYPOINT)          += -lapp_paramparse_ai_human_keypoint_detect
LIBS-$(CONFIG_MODULE_TDL_MD)                      += -lapp_paramparse_ai_md
LIBS-$(CONFIG_MODULE_TDL_PD)                      += -lapp_paramparse_ai_pd
LIBS-$(CONFIG_MODULE_TDL_CAPTURE)                 += -lapp_paramparse_ai_capture
LIBS-$(CONFIG_MODULE_TDL_FD_FACE)                 += -lapp_paramparse_ai_face
LIBS-$(CONFIG_MODULE_TDL_MOTION)                  += -lapp_paramparse_ai_motion
LIBS-$(CONFIG_MODULE_TDL_SOUND_CLS)               += -lapp_paramparse_ai_babycry
LIBS-$(CONFIG_MODULE_TDL_AEROEAR_DETECT)          += -lapp_paramparse_ai_aeroear
LIBS-$(CONFIG_MODULE_TDL)                         += -lapp_paramparse_ai
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_paramparse_display
LIBS-$(CONFIG_MODULE_FRMBUF)                      += -lapp_paramparse_frmbuf
LIBS-$(CONFIG_MODULE_GPIO)                        += -lapp_paramparse_gpio
LIBS-$(CONFIG_MODULE_PWM)                         += -lapp_paramparse_pwm
LIBS-$(CONFIG_MODULE_RTSP)                        += -lapp_paramparse_rtsp
LIBS-$(CONFIG_MODULE_RTP)                         += -lapp_paramparse_rtp
LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_paramparse_record
LIBS-y += -Wl,--no-whole-archive
endif

LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)                 += -lapp_media_audio
LIBS-$(CONFIG_MODULE_NETWORK)                     += -lapp_network
LIBS-$(CONFIG_MODULE_RTSP)                        += -lapp_rtsp

LIBS-$(CONFIG_MODULE_ADC)                         += -lapp_adc
LIBS-$(CONFIG_MODULE_IRCUT)                       += -lapp_ircut
LIBS-$(CONFIG_MODULE_GPIO)                        += -lapp_gpio
LIBS-$(CONFIG_MODULE_PWM)                         += -lapp_pwm
LIBS-$(CONFIG_MODULE_SDCARD)                      += -lapp_sdcard

LIBS-$(CONFIG_MODULE_TDL_MD)                      += -lapp_tdl_md
LIBS-$(CONFIG_MODULE_TDL_PD)                      += -lapp_tdl_pd
LIBS-$(CONFIG_MODULE_TDL_CAPTURE)                 += -lapp_tdl_capture
LIBS-$(CONFIG_MODULE_TDL_FD_FACE)                 += -lapp_tdl_fd_face
LIBS-$(CONFIG_MODULE_TDL_HUMAN_KEYPOINT)          += -lapp_tdl_human_keypoint_detect
LIBS-$(CONFIG_MODULE_TDL_MOTION)                  += -lapp_tdl_motion
LIBS-$(CONFIG_MODULE_TDL_SOUND_CLS)               += -lapp_tdl_sound_cls
LIBS-$(CONFIG_MODULE_TDL_AEROEAR_DETECT)           += -lapp_tdl_aeroear_detect
LIBS-$(CONFIG_MODULE_TDL_OBJECT_TRACK)            += -lapp_tdl_object_track

LIBS-$(CONFIG_MODULE_PARAMPARSE)                  += -lapp_paramparse
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_display
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_panel

LIBS-$(CONFIG_MODULE_CLOUD)                       += -lapp_hal_plat
LIBS-$(CONFIG_MODULE_AKYCLOUD)                    += -lapp_akysmart

LIBS-$(CONFIG_MODULE_MEDIA_SYS)                   += -lapp_media_sys
LIBS-$(CONFIG_MODULE_MEDIA_VI)                    += -lapp_media_vi
LIBS-$(CONFIG_MODULE_MEDIA_VPSS)                  += -lapp_media_vpss
LIBS-$(CONFIG_MODULE_MEDIA_VENC)                  += -lapp_media_venc
LIBS-$(CONFIG_MODULE_MEDIA_OSD)                   += -lapp_media_osd
LIBS-$(CONFIG_MODULE_MEDIA_VO)                    += -lapp_media_vo
LIBS-$(CONFIG_MODULE_MEDIA_DEC)                   += -lapp_media_vdec
LIBS-$(CONFIG_MODULE_MEDIA_DECSOFT)               += -lapp_media_vdecsoft
LIBS-$(CONFIG_MODULE_CVIUAC)                      += -lapp_cvi_uac
LIBS-$(CONFIG_MODULE_CVIUVC)                      += -lapp_cvi_uvc
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)                += -lapp_media_stitch
LIBS-$(CONFIG_MODULE_MEDIA_GDC)                   += -lapp_media_gdc
LIBS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT)            += -lapp_media_blacklight
LIBS-$(CONFIG_MODULE_RTP)                         += -lapp_rtp

LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_recorder
LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_file_recover
LIBS-$(CONFIG_MODULE_COMMON)                      += -lapp_common
LIBS-$(CONFIG_MODULE_CJSON)                       += -lapp_cjson
LIBS-$(CONFIG_MODULE_MININI)                      += -lapp_minini
LIBS-$(CONFIG_MODULE_MBUF)                        += -lapp_mbuf

LIBS-$(CONFIG_MODULE_FRMBUF)                      += -lapp_frmbuf
LIBS-$(CONFIG_MODULE_FRMBUF_DISP)                 += -lapp_frmbuf_disp
LIBS-$(CONFIG_MODULE_FRMBUF_LVGL)                 += -lapp_frmbuf_lvgl


ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y)
  LIBS += -Wl,-Bstatic
else
  LIBS += -Wl,-Bdynamic
endif

## module link
LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR)
LIBS-$(CONFIG_MODULE_PQTOOL) += -Wl,-Bstatic -lcvi_ispd2 -lisp -lraw_dump -lcvi_json-c
LIBS-$(CONFIG_MODULE_MEDIA_DECSOFT) += -L$(FFMPEG_6_0_LIB_DIR) -lavcodec -lavutil -lswresample -lswscale
LIBS-y += -L$(MW_PATH)/lib -lcvi_bin -lcvi_bin_isp -lvenc -lvdec -lsns_full -lisp -lawb -lae -laf -lisp_algo
LIBS-y += -L$(MW_PATH)/lib -lvi -lvo -lvpss -lrgn -lgdc -lsys -lrt
LIBS-$(CONFIG_SUPPORT_ATOMIC) += -latomic
LIBS-y += -L$(MW_PATH)/lib/3rd

ifneq ($(SOC_SEGMENT), CV180X)
  LIBS-$(CONFIG_MODULE_DISPLAY) += -L$(MW_PATH)/lib -lmipi_tx
endif
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine -lsbc
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO) += -lcvi_dnvqe -lcvi_ssp2
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)  += -laacdec2 -laacenc2 -laacsbrdec2 -laacsbrenc2 -laaccomm2
LIBS-$(CONFIG_MODULE_AUDIO_MP3)  += -lcvi_mp3 -lmad
LIBS-$(CONFIG_MODULE_CVIUAC)  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine
LIBS-$(CONFIG_MODULE_CVIUAC) += -lcvi_dnvqe -lcvi_ssp2
LIBS-$(CONFIG_MODULE_CVIUAC)  += -laacdec2 -laacenc2 -laacsbrdec2 -laacsbrenc2 -laaccomm2
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)  += -lstitch

ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  LIBS-$(CONFIG_MODULE_TDL) += -L$(APP_PREBUILT_DIR)/jpegturbo/musl_riscv64_lib
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  LIBS-$(CONFIG_MODULE_TDL) += -L$(APP_PREBUILT_DIR)/jpegturbo/glibc_riscv64_lib
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  LIBS-$(CONFIG_MODULE_TDL) += -L$(APP_PREBUILT_DIR)/jpegturbo/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
  LIBS-$(CONFIG_MODULE_TDL) += -L$(APP_PREBUILT_DIR)/jpegturbo/musl_arm32_lib
else ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
  LIBS-$(CONFIG_MODULE_TDL) += -L$(APP_PREBUILT_DIR)/jpegturbo/glibc_arm64_lib
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif
JPEG-TUBRO = -lturbojpeg

LIBS-$(CONFIG_MODULE_TDL) += -L$(TDL_PATH)/install/$(SOC_SEGMENT)/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/libsophon-0.4.9/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_ive_sdk/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/opencv-src/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/opencv-src/lib/opencv4/3rdparty
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/zlib-src/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/curl-src/lib
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/kaldi-native-fbank-build
LIBS-$(CONFIG_MODULE_TDL) += -L$(TOP_DIR)/tdl_sdk/build/$(SOC_SEGMENT)/_deps/kissfft-build

ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y) # AI libs static link
  AISDK := -ltdl_core-static
  TPU := -lcvikernel-static -lcviruntime-static -lcnpy -lcvimath-static -lz
  IVE := -lcvi_ive
  TEAISP := -lteaisp
  ifeq ($(TARGET_MACHINE),$(filter $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu arm-none-linux-musleabihf))
    OPENCV += -ltegra_hal -littnotify
  endif
  OPENCV += -lkaldi-native-fbank-core -lkissfft-float -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -llibwebp -llibpng -llibtiff -llibopenjp2 -lzlib -latomic
  LIBS-$(CONFIG_MODULE_TDL) += -Wl,--start-group -Wl,--allow-multiple-definition $(AISDK) $(IVE) $(TPU) $(OPENCV) $(JPEG-TUBRO) $(TEAISP) -Wl,--end-group
else
  AISDK := -ltdl_core
  TPU := -lcnpy  -lcvikernel  -lcvimath  -lcviruntime  -lz
  IVE := -lcvi_ive
  TEAISP := -lteaisp
  LIBS-$(CONFIG_MODULE_TDL) += $(AISDK) $(TPU) $(IVE) $(JPEG-TUBRO) $(TEAISP)
endif

# MULTI_PROCESS_SUPPORT
LIBS-$(CONFIG_MULTI_PROCESS_SUPPORT) += -lnanomsg

# RTSP
LIBS-$(CONFIG_MODULE_RTSP) += -L$(CVI_RTSP_DIR)/lib -lcomp_rtsp
LIBS-$(CONFIG_MODULE_RTSP) += -L$(RINGBUFFER_DIR)/lib -lcomp_ringbuffer
LIBS-$(CONFIG_MODULE_RTSP) += -L$(CVI_OSAL_DIR)/lib -lcomp_osal

#RECORD
LIBS-$(CONFIG_MODULE_RECORD) += -L$(RECORD_LIB_DIR)
LIBS-$(CONFIG_MODULE_RECORD) += -L$(FFMPEG_LIB_DIR) -lavformat -lavcodec -lavutil -lswresample
LIBS-$(CONFIG_MODULE_RECORD) += -L$(RINGBUFFER_DIR)/lib -lcomp_ringbuffer

#NETWORK
LIBS-$(CONFIG_MODULE_NETWORK) += -L$(THTTPD_LIB_DIR) -lthttpd
LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR) -L$(OPENSSL_LIB_DIR) -lwebsockets -lssl -lcrypto

LIBS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT) += -L$(MW_PATH)/lib -lcvi_ive
LIBS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_ive_sdk/ex_lib -lcvi_ive_tpu_ex
LIBS-$(CONFIG_MODULE_MEDIA_BLACKLIGHT) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib -lcvimath-static -lcviruntime-static -lcvikernel-static

# FRMBUF_LVGL
LIBS-$(CONFIG_MODULE_FRMBUF_LVGL) += -L$(LVGL_LIB_DIR) -llvgl -llvgl_demos -llvgl_examples -llvgl_thorvg

# CLOUD
ifeq ($(CONFIG_MODULE_CLOUD), y)
  ifeq ($(CONFIG_MODULE_AKYCLOUD), y)
    LIBS-$(CONFIG_MODULE_AKYCLOUD)  += -L$(CLOUD_LIB_DIR)   \
    -Wl,--start-group                                       \
    -lCloudServ -lUdsCoreSrv -lmcjson -lhv -lrtspserver     \
    -lmbedtls -lmbedcrypto -lmbedx509 -ltransclient -lztapi \
    -Wl,--end-group
  else
    $(info "Must select cloud paltform if open CONFIG_MODULE_CLOUD")
  endif
endif

LIBS += $(LIBS-y)

ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y)
ifeq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu riscv64-unknown-linux-gnu),)
  LIBS += -static
endif
endif
ifneq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu riscv64-unknown-linux-gnu),)
  LIBS += -Wl,-Bdynamic -ldl -pthread
endif
