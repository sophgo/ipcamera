#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

//private attribute
const char *vdec_demode[VIDEO_MODE_BUTT] = {
    [VIDEO_MODE_STREAM] = "VIDEO_MODE_STREAM",
    [VIDEO_MODE_FRAME]  = "VIDEO_MODE_FRAME",
    [VIDEO_MODE_COMPAT] = "VIDEO_MODE_COMPAT"
};

const char *vdec_input_type[APP_VDEC_INPUT_BUTT] = {
    [APP_VDEC_INPUT_NONE] = "APP_VDEC_INPUT_NONE",
    [APP_VDEC_INPUT_FILE] = "APP_VDEC_INPUT_FILE",
    [APP_VDEC_INPUT_RTSP] = "APP_VDEC_INPUT_RTSP",
    [APP_VDEC_INPUT_RTP]  = "APP_VDEC_INPUT_RTP"
};

const char *vdec_dec_mode[VIDEO_DEC_MODE_BUTT] = {
    [VIDEO_DEC_MODE_IPB] = "VIDEO_DEC_MODE_IPB",
    [VIDEO_DEC_MODE_IP]  = "VIDEO_DEC_MODE_IP",
    [VIDEO_DEC_MODE_I]   = "VIDEO_DEC_MODE_I"
};

const char *vdec_output_order[VIDEO_OUTPUT_ORDER_BUTT] = {
    [VIDEO_OUTPUT_ORDER_DISP] = "VIDEO_OUTPUT_ORDER_DISP",
    [VIDEO_OUTPUT_ORDER_DEC]  = "VIDEO_OUTPUT_ORDER_DEC"
};

#ifdef RTSP_SUPPORT
const char *vdec_rtsp_transport[APP_RTSP_TRANS_BUTT] = {
    [APP_RTSP_TRANS_UDP] = "APP_RTSP_TRANS_UDP",
    [APP_RTSP_TRANS_TCP] = "APP_RTSP_TRANS_TCP"
};
#endif

int Load_Param_Vdec(const char *file)
{
    int ret = 0;
    int i = 0;
    int enum_num = 0;
    long int vdec_chn_num = 0;
    char tmp_section[32] = {0};
    char tmp_buff[PARAM_STRING_LEN] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_VDEC_CTX_S *Vdec = app_ipcam_Vdec_Param_Get();
    const char ** payload_type = app_ipcam_Param_get_payload_type();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vdec config ------------------> start \n");

    vdec_chn_num = ini_getl("vdec_config", "chn_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vdec_chn_num: %ld\n", vdec_chn_num);

    for (i = 0; i < vdec_chn_num; i++)
    {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vdecchn%d", i);
        Vdec->astVdecChnCfg.VdecChn = i;
        Vdec->astVdecChnCfg.bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        Vdec->astVdecChnCfg.input_type = APP_VDEC_INPUT_NONE;
        memset(Vdec->astVdecChnCfg.decode_file_name, 0, sizeof(Vdec->astVdecChnCfg.decode_file_name));
        memset(Vdec->astVdecChnCfg.rtsp_url, 0, sizeof(Vdec->astVdecChnCfg.rtsp_url));
        if (!Vdec->astVdecChnCfg.bEnable)
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec_chn[%d] not enable!\n", i);
            continue;
        }

        Vdec->astVdecChnCfg.u32Width  = ini_getl(tmp_section, "width", 0, file);
        Vdec->astVdecChnCfg.u32Height = ini_getl(tmp_section, "height", 0, file);

        ini_gets(tmp_section, "input_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_input_type,
            APP_VDEC_INPUT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS || enum_num == APP_VDEC_INPUT_NONE) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][input_type] invalid: %s\n", tmp_section, str_name);
            Vdec->astVdecChnCfg.bEnable = CVI_FALSE;
            Vdec->astVdecChnCfg.input_type = APP_VDEC_INPUT_NONE;
            continue;
        }
        Vdec->astVdecChnCfg.input_type = enum_num;
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][input_type] Convert string name [%s] to enum number [%d].\n",
            tmp_section, str_name, enum_num);

        if (Vdec->astVdecChnCfg.input_type == APP_VDEC_INPUT_FILE) {
            ini_gets(tmp_section, "decode_filename", " ", tmp_buff, PARAM_STRING_LEN, file);
            strncpy(Vdec->astVdecChnCfg.decode_file_name, tmp_buff, PARAM_STRING_LEN);
            if (Vdec->astVdecChnCfg.decode_file_name[0] == '\0') {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][decode_filename] empty, disable vdec.\n", tmp_section);
                Vdec->astVdecChnCfg.bEnable = CVI_FALSE;
                Vdec->astVdecChnCfg.input_type = APP_VDEC_INPUT_NONE;
                continue;
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s] vdec input file: %s\n",
                tmp_section, Vdec->astVdecChnCfg.decode_file_name);
        }
        else if (Vdec->astVdecChnCfg.input_type == APP_VDEC_INPUT_RTSP) {
#ifdef RTSP_SUPPORT
            ini_gets(tmp_section, "rtsp_url", " ", tmp_buff, PARAM_STRING_LEN, file);
            strncpy(Vdec->astVdecChnCfg.rtsp_url, tmp_buff, PARAM_STRING_LEN);
            if (Vdec->astVdecChnCfg.rtsp_url[0] == '\0') {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][rtsp_url] empty, disable vdec.\n", tmp_section);
                Vdec->astVdecChnCfg.bEnable = CVI_FALSE;
                Vdec->astVdecChnCfg.input_type = APP_VDEC_INPUT_NONE;
                continue;
            }
            ini_gets(tmp_section, "rtsp_transport", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_rtsp_transport,
                APP_RTSP_TRANS_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "[%s][rtsp_transport] invalid: %s, use tcp.\n",
                    tmp_section, str_name);
                enum_num = APP_RTSP_TRANS_TCP;
            }
            Vdec->astVdecChnCfg.rtsp_transport = enum_num;
            if (ret == CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rtsp_transport] Convert string name [%s] to enum number [%d].\n",
                    tmp_section, str_name, enum_num);
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s] vdec input rtsp: %s transport:%s(%d)\n",
                tmp_section, Vdec->astVdecChnCfg.rtsp_url,
                (Vdec->astVdecChnCfg.rtsp_transport == APP_RTSP_TRANS_UDP) ? "udp" : "tcp",
                Vdec->astVdecChnCfg.rtsp_transport);
#endif
        }

        ini_gets(tmp_section, "de_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, payload_type, PT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][de_type] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][de_type] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnAttr.enType = enum_num;
        }

        ini_gets(tmp_section, "de_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_demode, VIDEO_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][de_mode] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][de_mode] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnAttr.enMode = enum_num;
        }

        Vdec->astVdecChnCfg.astChnAttr.u32PicWidth      = ini_getl(tmp_section, "pic_width", 0, file);
        Vdec->astVdecChnCfg.astChnAttr.u32PicHeight     = ini_getl(tmp_section, "pic_height", 0, file);
        Vdec->astVdecChnCfg.astChnAttr.u32FrameBufCnt   = ini_getl(tmp_section, "frm_buf_cnt", 0, file);

        ini_gets(tmp_section, "vdec_pixfmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][vdec_pixfmt] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_pixfmt] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnParam.enPixelFormat = enum_num;
        }
        
        Vdec->astVdecChnCfg.astChnParam.u32DisplayFrameNum = ini_getl(tmp_section, "vdec_disp_frm_num", 0, file);
        Vdec->PicVbPool = ini_getl(tmp_section, "PicVbPool", 0, file);
        Vdec->astVdecChnCfg.u32MiniBufMode = ini_getl(tmp_section, "vdec_mini_buf_mode", 0, file);
        Vdec->astVdecChnCfg.u32RefFrameNum = ini_getl(tmp_section, "vdec_ref_frame_num", 16, file);

        Vdec->astVdecChnCfg.enDecMode = VIDEO_DEC_MODE_IPB;
        ini_gets(tmp_section, "vdec_dec_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_dec_mode, VIDEO_DEC_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_dec_mode] not set, use default IPB.\n", tmp_section);
        } else {
            Vdec->astVdecChnCfg.enDecMode = enum_num;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_dec_mode] Convert string name [%s] to enum number [%d].\n",
                tmp_section, str_name, enum_num);
        }

        Vdec->astVdecChnCfg.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;
        ini_gets(tmp_section, "vdec_output_order", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_output_order, VIDEO_OUTPUT_ORDER_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_output_order] not set, use default DISP.\n", tmp_section);
        } else {
            Vdec->astVdecChnCfg.enOutputOrder = enum_num;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_output_order] Convert string name [%s] to enum number [%d].\n",
                tmp_section, str_name, enum_num);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vdec config ------------------> done \n\n");

    return CVI_SUCCESS;
}
