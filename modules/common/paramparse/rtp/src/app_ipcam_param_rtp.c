#include <stdio.h>
#include <string.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_rtp.h"

static const char *rtp_role[APP_RTP_ROLE_BUTT] = {
    [APP_RTP_ROLE_NONE] = "APP_RTP_ROLE_NONE",
    [APP_RTP_ROLE_TX] = "APP_RTP_ROLE_TX",
    [APP_RTP_ROLE_RX] = "APP_RTP_ROLE_RX"
};

int Load_Param_Rtp(const char *file)
{
    APP_RTP_PARAM_S *Rtp = app_ipcam_Rtp_Param_Get();
    char role[32] = {0};
    int enum_num = APP_RTP_ROLE_NONE;
    int ret;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTP config ------------------> start \n");

    memset(Rtp, 0, sizeof(*Rtp));
    Rtp->s32VencChn = -1;
    Rtp->s32VdecChn = -1;
    Rtp->u32MaxAuSize = APP_RTP_DEFAULT_AU_SIZE;
    Rtp->bEnable = ini_getl("rtp_config", "enable", 0, file);
    if (!Rtp->bEnable)
        return CVI_SUCCESS;

    ini_gets("rtp_config", "role", "APP_RTP_ROLE_NONE", role, sizeof(role), file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(role, rtp_role,
        APP_RTP_ROLE_BUTT, &enum_num);
    if (ret != CVI_SUCCESS || enum_num == APP_RTP_ROLE_NONE) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[rtp_config][role] invalid: %s\n", role);
        return CVI_FAILURE;
    }
    Rtp->enRole = enum_num;

    Rtp->s32VencChn = ini_getl("rtp_config", "venc_chn", -1, file);
    Rtp->s32VdecChn = ini_getl("rtp_config", "vdec_chn", -1, file);
    ini_gets("rtp_config", "peer_ip", "", Rtp->szPeerIp, sizeof(Rtp->szPeerIp), file);
    Rtp->u16VideoPort = ini_getl("rtp_config", "video_port", 5600, file);
    Rtp->u16ControlPort = ini_getl("rtp_config", "control_port", 5601, file);
    Rtp->u32MaxAuSize = ini_getl("rtp_config", "max_au_size", APP_RTP_DEFAULT_AU_SIZE, file);
    if (Rtp->u32MaxAuSize == 0 || Rtp->szPeerIp[0] == '\0') {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "[rtp_config] peer_ip and max_au_size must be configured\n");
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "RTP role=%s venc_chn=%d vdec_chn=%d peer_ip=%s video_port=%u control_port=%u max_au_size=%u\n",
        role, Rtp->s32VencChn, Rtp->s32VdecChn, Rtp->szPeerIp,
        Rtp->u16VideoPort, Rtp->u16ControlPort, Rtp->u32MaxAuSize);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTP config ------------------> done \n\n");

    return CVI_SUCCESS;
}
