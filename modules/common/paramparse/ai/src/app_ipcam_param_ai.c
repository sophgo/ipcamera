#include "app_ipcam_paramparse.h"
#define NAME2STR(NAME)              (#NAME)
#define MODEL_NAME(MODEL_NAME)      NAME2STR(MODEL_NAME)
//ai common attribute
const char *ai_supported_model[TDL_MODEL_MAX] = {
    [TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON] = MODEL_NAME(TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON),
    [TDL_MODEL_SCRFD_DET_FACE] = MODEL_NAME(TDL_MODEL_SCRFD_DET_FACE),
    [TDL_MODEL_KEYPOINT_YOLOV8POSE_PERSON17] = MODEL_NAME(TDL_MODEL_KEYPOINT_YOLOV8POSE_PERSON17)
};

const char ** app_ipcam_Param_get_ai_supported_model()
{
    return ai_supported_model;
}
