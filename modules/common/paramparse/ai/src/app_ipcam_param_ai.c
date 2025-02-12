#include "app_ipcam_paramparse.h"
#define NAME2STR(NAME)              (#NAME)
#define MODEL_NAME(MODEL_NAME)      NAME2STR(MODEL_NAME)
//ai common attribute
const char *ai_supported_model[CVI_TDL_SUPPORTED_MODEL_END] = {
    [CVI_TDL_SUPPORTED_MODEL_RETINAFACE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_RETINAFACE),
    [CVI_TDL_SUPPORTED_MODEL_SCRFDFACE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_SCRFDFACE),
    [CVI_TDL_SUPPORTED_MODEL_RETINAFACE_IR] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_RETINAFACE_IR),
    [CVI_TDL_SUPPORTED_MODEL_RETINAFACE_HARDHAT] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_RETINAFACE_HARDHAT),
    [CVI_TDL_SUPPORTED_MODEL_THERMALFACE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_THERMALFACE),
    [CVI_TDL_SUPPORTED_MODEL_THERMALPERSON] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_THERMALPERSON),
    [CVI_TDL_SUPPORTED_MODEL_FACEATTRIBUTE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_FACEATTRIBUTE),
    [CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION),
    [CVI_TDL_SUPPORTED_MODEL_MASKFACERECOGNITION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MASKFACERECOGNITION),
    [CVI_TDL_SUPPORTED_MODEL_FACEQUALITY] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_FACEQUALITY),
    [CVI_TDL_SUPPORTED_MODEL_LIVENESS] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_LIVENESS),
    [CVI_TDL_SUPPORTED_MODEL_MASKCLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MASKCLASSIFICATION),
    [CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE),
    [CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE),
    [CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN),
    [CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PERSON_PETS] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PERSON_PETS),
    [CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_COCO80] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_COCO80),
    [CVI_TDL_SUPPORTED_MODEL_YOLOV3] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_YOLOV3),
    [CVI_TDL_SUPPORTED_MODEL_YOLOX] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_YOLOX),
    [CVI_TDL_SUPPORTED_MODEL_OSNET] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_OSNET),
    [CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION),
    [CVI_TDL_SUPPORTED_MODEL_WPODNET] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_WPODNET),
    [CVI_TDL_SUPPORTED_MODEL_LPRNET_TW] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_LPRNET_TW),
    [CVI_TDL_SUPPORTED_MODEL_LPRNET_CN] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_LPRNET_CN),
    [CVI_TDL_SUPPORTED_MODEL_DEEPLABV3] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_DEEPLABV3),
    [CVI_TDL_SUPPORTED_MODEL_EYECLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_EYECLASSIFICATION),
    [CVI_TDL_SUPPORTED_MODEL_YAWNCLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_YAWNCLASSIFICATION),
    [CVI_TDL_SUPPORTED_MODEL_FACELANDMARKER] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_FACELANDMARKER),
    [CVI_TDL_SUPPORTED_MODEL_INCAROBJECTDETECTION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_INCAROBJECTDETECTION),
    [CVI_TDL_SUPPORTED_MODEL_SMOKECLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_SMOKECLASSIFICATION),
    [CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION) ,
    [CVI_TDL_SUPPORTED_MODEL_PERSON_PETS_DETECTION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_PERSON_PETS_DETECTION) ,
    [CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_YOLOV8POSE) ,
    [CVI_TDL_SUPPORTED_MODEL_HAND_DETECTION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_HAND_DETECTION),
    [CVI_TDL_SUPPORTED_MODEL_HAND_KEYPOINT] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_HAND_KEYPOINT),
    [CVI_TDL_SUPPORTED_MODEL_HAND_KEYPOINT_CLASSIFICATION] = MODEL_NAME(CVI_TDL_SUPPORTED_MODEL_HAND_KEYPOINT_CLASSIFICATION)
};

const char ** app_ipcam_Param_get_ai_supported_model()
{
    return ai_supported_model;

}
