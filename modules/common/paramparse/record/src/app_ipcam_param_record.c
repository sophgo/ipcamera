#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_record.h"

#define REC_DEFAULT_SPLITIMEMSEC 5 * 60 * 1000//MS 

int Load_Param_Record(const char * file)
{
    CVI_S32 i = 0;
    char tmp_section[32] = {0};
    APP_IPCAM_RECORD_S * pstRecordParam = app_ipcam_Record_Param_Get();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading record config ------------------> start \n");

    pstRecordParam->s32ChnNumber = ini_getl("record", "chnnumber", 0, file);
    pstRecordParam->s32SplitTimeSec = ini_getl("record", "splittimesec", REC_DEFAULT_SPLITIMEMSEC, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "record chnnumber %d \n", pstRecordParam->s32ChnNumber);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "record s32SplitTimeSec %d \n", pstRecordParam->s32SplitTimeSec);
    for (i = 0 ; i < pstRecordParam->s32ChnNumber; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "record%d_video", i);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32Enable = ini_getl(tmp_section, "enable", 0, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32BindChn = ini_getl(tmp_section, "bindchn", -1, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32Type = ini_getl(tmp_section, "type", 96, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32Width = ini_getl(tmp_section, "width", 1920, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32Height = ini_getl(tmp_section, "height", 1080, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32BitRate = ini_getl(tmp_section, "bitrate", 1024, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32FrameRate = ini_getl(tmp_section, "framerate", 15, file);
        pstRecordParam->stChnHandle[i].stVideoAttribute.s32Gop = ini_getl(tmp_section, "gop", 15, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32Enable %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32Enable);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32BindChn %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32BindChn);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32Type %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32Type);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32Width %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32Width);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32Height %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32Height);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32BitRate %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32BitRate);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32FrameRate %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32FrameRate);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] video s32Gop %d \n", i, pstRecordParam->stChnHandle[i].stVideoAttribute.s32Gop);
        snprintf(tmp_section, sizeof(tmp_section), "record%d_audio", i);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32Enable = ini_getl(tmp_section, "enable", 0, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32BindChn = ini_getl(tmp_section, "bindchn", -1, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32Type = ini_getl(tmp_section, "type", 104, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32SampleBitWidth = ini_getl(tmp_section, "samplebitwidth", 16, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32ChnCnt = ini_getl(tmp_section, "chncnt", 2, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32SamplePerFrame = ini_getl(tmp_section, "sampleperframe", 320, file);
        pstRecordParam->stChnHandle[i].stAudioAttribute.s32SampleRate = ini_getl(tmp_section, "sampleperate", 8000, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32Enable %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32Enable);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32BindChn %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32BindChn);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32Type %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32Type);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32SampleBitWidth %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32SampleBitWidth);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32ChnCnt %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32ChnCnt);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32SamplePerFrame %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32SamplePerFrame);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "record[%d] audio s32SampleRate %d \n", i, pstRecordParam->stChnHandle[i].stAudioAttribute.s32SampleRate);
    }
    return 0;
}