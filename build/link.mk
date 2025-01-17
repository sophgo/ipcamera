LIBS += -L$(TARGET_OUT_DIR)/lib/

LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)                 += -lapp_media_audio
LIBS-$(CONFIG_MODULE_NETWORK)                     += -lapp_network
LIBS-$(CONFIG_MODULE_RTSP)                        += -lapp_rtsp

LIBS-$(CONFIG_MODULE_ADC)                         += -lapp_adc
LIBS-$(CONFIG_MODULE_IRCUT)                       += -lapp_ircut
LIBS-$(CONFIG_MODULE_GPIO)                        += -lapp_gpio
LIBS-$(CONFIG_MODULE_PWM)                         += -lapp_pwm

LIBS-$(CONFIG_MODULE_AI_MD)                       += -lapp_md
LIBS-$(CONFIG_MODULE_AI_PD)                       += -lapp_ai_pd
LIBS-$(CONFIG_MODULE_AI_HAND_DETECT)              += -lapp_ai_hand_detect
LIBS-$(CONFIG_MODULE_AI_FD_FACE)                  += -lapp_ai_fd_cace
LIBS-$(CONFIG_MODULE_AI_IRFAECE)                  += -lapp_ai_irface
LIBS-$(CONFIG_MODULE_AI_BABYCRY)                  += -lapp_ai_babycry
LIBS-$(CONFIG_MODULE_AI_CONSUMER_COUNTING)        += -lapp_ai_consumer_counting

LIBS-$(CONFIG_MODULE_PARAMPARSE)                  += -lapp_paramparse
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_display
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_panel

LIBS-$(CONFIG_MODULE_MEDIA_SYS)                   += -lapp_media_sys
LIBS-$(CONFIG_MODULE_MEDIA_VI)                    += -lapp_media_vi
LIBS-$(CONFIG_MODULE_MEDIA_VPSS)                  += -lapp_media_vpss
LIBS-$(CONFIG_MODULE_MEDIA_GDC)                   += -lapp_media_gdc
LIBS-$(CONFIG_MODULE_MEDIA_VENC)                  += -lapp_media_venc
LIBS-$(CONFIG_MODULE_MEDIA_OSD)                   += -lapp_media_osd
LIBS-$(CONFIG_MODULE_MEDIA_VO)                    += -lapp_media_vo
LIBS-$(CONFIG_MODULE_MEDIA_VDEC)                   += -lapp_media_vdec
LIBS-$(CONFIG_MODULE_MEDIA_VDECSOFT)               += -lapp_media_vdecsoft
LIBS-$(CONFIG_MODULE_CVIUAC)                      += -lapp_cvi_uac
LIBS-$(CONFIG_MODULE_CVIUVC)                      += -lapp_cvi_uvc
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)                += -lapp_media_stitch
LIBS-$(CONFIG_MODULE_MEDIA_DPU)                   += -lapp_media_dpu

ifeq ($(CONFIG_MODULE_PARAMPARSE), y)
LIBS-y += -Wl,--whole-archive
LIBS-$(CONFIG_MODULE_MEDIA_SYS)                   += -lapp_paramparse_sys
LIBS-$(CONFIG_MODULE_MEDIA_VI)                    += -lapp_paramparse_vi
LIBS-$(CONFIG_MODULE_MEDIA_VPSS)                  += -lapp_paramparse_vpss
LIBS-$(CONFIG_MODULE_MEDIA_GDC)                   += -lapp_paramparse_gdc
LIBS-$(CONFIG_MODULE_MEDIA_VENC)                  += -lapp_paramparse_venc
LIBS-$(CONFIG_MODULE_MEDIA_OSD)                   += -lapp_paramparse_osd
LIBS-$(CONFIG_MODULE_MEDIA_VDEC)                  += -lapp_paramparse_vdec
LIBS-$(CONFIG_MODULE_MEDIA_VDECSOFT)              += -lapp_paramparse_vdecsoft
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)                 += -lapp_paramparse_audio
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)                += -lapp_paramparse_stitch
LIBS-$(CONFIG_MODULE_MEDIA_DPU)                   += -lapp_paramparse_dpu
LIBS-$(CONFIG_MODULE_AI)                          += -lapp_paramparse_ai
LIBS-$(CONFIG_MODULE_AI_MD)                       += -lapp_paramparse_ai_md
LIBS-$(CONFIG_MODULE_AI_PD)                       += -lapp_paramparse_ai_pd
LIBS-$(CONFIG_MODULE_AI_HAND_DETECT)              += -lapp_paramparse_ai_hd
LIBS-$(CONFIG_MODULE_AI_FD_FACE)                  += -lapp_paramparse_ai_face
LIBS-$(CONFIG_MODULE_AI_IRFAECE)                  += -lapp_paramparse_ai_ir_face
LIBS-$(CONFIG_MODULE_AI_BABYCRY)                  += -lapp_paramparse_ai_babycry
LIBS-$(CONFIG_MODULE_AI_CONSUMER_COUNTING)        += -lapp_paramparse_ai_consumer_counting
LIBS-$(CONFIG_MODULE_DISPLAY)                     += -lapp_paramparse_display
LIBS-$(CONFIG_MODULE_FRMBUF)                      += -lapp_paramparse_frmbuf
LIBS-$(CONFIG_MODULE_GPIO)                        += -lapp_paramparse_gpio
LIBS-$(CONFIG_MODULE_RTSP)                        += -lapp_paramparse_rtsp
LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_paramparse_record
LIBS-y += -Wl,--no-whole-archive
endif


LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_recorder
LIBS-$(CONFIG_MODULE_RECORD)                      += -lapp_file_recover
LIBS-$(CONFIG_MODULE_RINGBUF)                     += -lapp_ringbuffer
LIBS-$(CONFIG_MODULE_COMMON)                      += -lapp_common
LIBS-$(CONFIG_MODULE_CJSON)                       += -lapp_cjson
LIBS-$(CONFIG_MODULE_MININI)                      += -lapp_minini

LIBS-$(CONFIG_MODULE_FRMBUF)                      += -lapp_frmbuf
LIBS-$(CONFIG_MODULE_DISP_FRMBUF)                 += -lapp_disp_frmbuf


LIBS += -Wl,-Bstatic

## module  link
LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR)
LIBS-$(CONFIG_MODULE_MEDIA_DECSOFT) += -L$(FFMPEG_6_0_LIB_DIR) -lavcodec -lavutil -lswresample -lswscale
LIBS-y += -L$(MW_PATH)/lib -lcvi_bin -lvenc -lvdec -lsns_full -lisp -lawb -lae -laf -lisp_algo
LIBS-y += -L$(MW_PATH)/lib -lvi -lvpss -lvo -lrgn -lgdc -lsys -lmipi_tx
LIBS-$(CONFIG_SUPPORT_ATOMIC) += -latomic
#for dual_os
LIBS-$(CONFIG_MODULE_MSG) += -lmsg -lcvilink
LIBS-y += -L$(MW_PATH)/lib/3rd

LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO) += -lcvi_dnvqe -lcvi_ssp2
LIBS-$(CONFIG_MODULE_MEDIA_AUDIO)  += -laacdec2 -laacenc2 -laacsbrdec2 -laacsbrenc2 -laaccomm2
LIBS-$(CONFIG_MODULE_AUDIO_MP3)  += -lcvi_mp3 -lmad
LIBS-$(CONFIG_MODULE_CVIUAC)  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine
LIBS-$(CONFIG_MODULE_CVIUAC) += -lcvi_dnvqe -lcvi_ssp2
LIBS-$(CONFIG_MODULE_CVIUAC)  += -laacdec2 -laacenc2 -laacsbrdec2 -laacsbrenc2 -laaccomm2
LIBS-$(CONFIG_MODULE_MEDIA_STITCH)  += -lstitch
LIBS-$(CONFIG_MODULE_MEDIA_DPU)  += -ldpu -lcvi_ive
LIBS-$(CONFIG_MODULE_AI_MD) += -L$(TDL_PATH)/install/lib/
LIBS-$(CONFIG_MODULE_AI_MD) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/

ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y) # MD libs static link
  MDSDK := -lcvi_md
  IVE := -lcvi_ive
  LIBS-$(CONFIG_MODULE_AI_MD) += -Wl,--start-group $(MDSDK) $(IVE) $(TPU) -Wl,--end-group
else # MD libs dynamic link
  MDSDK := -lcvi_md
  IVE := -lcvi_ive
  LIBS-$(CONFIG_MODULE_AI_MD) += $(MDSDK) $(TPU) $(IVE)
endif # MD libs static link end

ifneq ($(findstring $(TARGET_MACHINE), riscv64-unknown-linux-musl),)
  LIBS-$(CONFIG_MODULE_AI) += -L$(APP_PREBUILT_DIR)/jpegturbo/musl_lib
else ifneq ($(findstring $(TARGET_MACHINE), riscv64-unknown-linux-gnu),)
  LIBS-$(CONFIG_MODULE_AI) += -L$(APP_PREBUILT_DIR)/jpegturbo/glibc_lib
else ifneq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf),)
  LIBS-$(CONFIG_MODULE_AI) += -L$(APP_PREBUILT_DIR)/jpegturbo/lib32bit
else ifneq ($(findstring $(TARGET_MACHINE), aarch64-linux-gnu aarch64-linux aarch64-none-linux-gnu),)
  LIBS-$(CONFIG_MODULE_AI) += -L$(APP_PREBUILT_DIR)/jpegturbo/lib64bit
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif
JPEG-TUBRO = -lturbojpeg

LIBS-$(CONFIG_MODULE_AI) += -L$(TDL_PATH)/install/lib
LIBS-$(CONFIG_MODULE_AI) += -L$(TDL_PATH)/install/sample/3rd/opencv/lib
LIBS-$(CONFIG_MODULE_AI) += -L$(MW_PATH)/3rdparty/opencv/lib_64
LIBS-$(CONFIG_MODULE_AI) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib
LIBS-$(CONFIG_MODULE_AI) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/libsophon-0.4.9/lib
LIBS-$(CONFIG_MODULE_AI) += -L$(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_ive_sdk/lib
ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y) # AI libs static link
    AISDK += -lgdc -lcvi_tdl -lcvi_tdl_app
    IVE = -lcvi_ive
    ifneq ($(wildcard $(OUTPUT_DIR)/tpu_$(SDK_VER)/cvitek_tpu_sdk/libsophon-0.4.9/lib/libbmrt.a),)
      TPU := -lbmrt -lbmlib -lbmodel
    else
      TPU := -Wl,-Bdynamic -lbmrt -lbmlib -Wl,-Bstatic
    endif
    OPENCV += -lz -ltegra_hal -lopencv_core -lopencv_imgproc
  LIBS-$(CONFIG_MODULE_AI) += -Wl,--start-group $(AISDK) $(IVE) $(TPU) $(OPENCV) $(JPEG-TUBRO) -Wl,--end-group
else
    AISDK += -lcvi_tdl -lcvi_tdl_app
    IVE = -lcvi_ive
    TPU := -lbmrt -lbmlib
  LIBS-$(CONFIG_MODULE_AI) += $(AISDK) $(TPU) $(IVE) $(JPEG-TUBRO)
endif

LIBS-$(CONFIG_MODULE_PQTOOL) += -Wl,-Bstatic -lcvi_ispd2 -lvo -lisp -lraw_dump -lcvi_json-c

# MULTI_PROCESS_SUPPORT
LIBS-$(CONFIG_MULTI_PROCESS_SUPPORT) += -lnanomsg

# RTSP
LIBS-$(CONFIG_MODULE_RTSP) += -L$(CVI_RTSP_DIR)/lib -lcvi_comp_rtsp
LIBS-$(CONFIG_MODULE_RTSP) += -L$(RINGBUFFER_DIR)/lib -lcvi_comp_ringbuffer
LIBS-$(CONFIG_MODULE_RTSP) += -L$(CVI_OSAL_DIR)/lib -lcvi_comp_osal

#MBUF
LIBS-$(CONFIG_MODULE_MBUF) += -L$(CVI_MBUF_DIR)/lib -lcvi_comp_mbuf

#RECORD
LIBS-$(CONFIG_MODULE_RECORD) += -L$(FFMPEG_LIB_DIR) -lavformat -lavcodec -lavutil -lswresample
#NETWORK
LIBS-$(CONFIG_MODULE_NETWORK) += -L$(THTTPD_LIB_DIR) -lthttpd
ifneq ($(findstring $(TARGET_MACHINE), riscv64-unknown-linux-gnu),)
  LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR) -L$(OPENSSL_LIB_DIR) -lwebsockets -lssl -lcrypto
else ifneq ($(findstring $(TARGET_MACHINE), aarch64-linux-gnu aarch64-linux aarch64-none-linux-gnu),)
  LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR) -L$(OPENSSL_LIB_DIR) -lwebsockets -lssl -lcrypto
else
  LIBS-$(CONFIG_MODULE_NETWORK) += -L$(WEB_SOCKET_LIB_DIR) -lwebsockets
endif

LIBS += $(LIBS-y)

ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y)
ifeq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu aarch64-linux aarch64-none-linux-gnu riscv64-unknown-linux-gnu),)
  LIBS += -static
endif
endif
ifneq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-linux-gnu aarch64-linux aarch64-none-linux-gnu riscv64-unknown-linux-gnu),)
  LIBS += -Wl,-Bdynamic -ldl -lpthread
endif
