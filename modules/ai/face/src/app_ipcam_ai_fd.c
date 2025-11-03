#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <dirent.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define MAX_FD_NUM 100
SMT_MUTEXAUTOLOCK_INIT(g_FDMutex);
static pthread_mutex_t g_FDStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_FD_CFG_S g_stFDCfg;
static TDLFeatureInfo gallery_feature = {0};
// static APP_PARAM_AI_FD_CFG_S g_stFDCfg = {
//     .FD_bEnable = 0,
//     .VpssGrp = 0,
//     .VpssChn = 1,
//     .u32GrpWidth = 640,
//     .u32GrpHeight = 384,
//     .threshold_fd = 0.7,
//     .model_id_fd = TDL_MODEL_SCRFD_DET_FACE,
//     .model_path_fd = "/mnt/scrfd_det_face_432_768_INT8_mars3.bmodel",
// };
static APP_PARAM_AI_FD_CFG_S *g_pstFDCfg = &g_stFDCfg;

static volatile bool g_bFDRunning = CVI_FALSE;
static volatile bool g_bFDPause = CVI_FALSE;
static pthread_t g_FDThreadHandle;
static TDLHandle g_FDAiHandle = NULL;
static TDLFace g_stFDObjDraw = {};
static pfpFaceInferenceFunc g_pfpFDInference;

// #define IMAGE_DIR "/mnt/sd/picture/"
struct LinkList
{
    int index;
    char name[128];
    struct LinkList *next;
};
static CVI_U32 g_FDFps;
static CVI_U32 g_FDProc;


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_FD_CFG_S *app_ipcam_Ai_FD_Param_Get(void)
{
    return g_pstFDCfg;
}

CVI_VOID app_ipcam_Ai_FD_ProcStatus_Set(CVI_BOOL flag)
{
    g_bFDRunning = flag;
}

CVI_BOOL app_ipcam_Ai_FD_ProcStatus_Get(void)
{
    return g_bFDRunning;
}

CVI_VOID app_ipcam_Ai_FD_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_FDStatusMutex);
    g_bFDPause = flag;
    pthread_mutex_unlock(&g_FDStatusMutex);
}

CVI_BOOL app_ipcam_Ai_FD_Pause_Get(void)
{
    return g_bFDPause;
}

CVI_U32 app_ipcam_Ai_FD_ProcFps_Get(void)
{
    return g_FDFps;
}

CVI_S32 app_ipcam_Ai_FD_ProcTime_Get(void)
{
    return g_FDProc;
}

const char* emotion_to_text(int code) {
    switch (code) {
        case 0: return "anger";
        case 1: return "disgust";
        case 2: return "fear";
        case 3: return "happy";
        case 4: return "neutral";
        case 5: return "sad";
        case 6: return "surprise";
        default: return "unknown";
    }
}

static void app_ipcam_Ai_Fd_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "FD_bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d \n", \
        g_pstFDCfg->FD_bEnable, g_pstFDCfg->VpssGrp, g_pstFDCfg->VpssChn, g_pstFDCfg->u32GrpWidth, g_pstFDCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold_fd=%f \n",  g_pstFDCfg->threshold_fd);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_fd=%d model_path_fd=%s \n", g_pstFDCfg->model_id_fd, g_pstFDCfg->model_path_fd);
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_SCRFD_DET_FACE:
            g_pfpFDInference = TDL_FaceDetection;
        break;

        default:
            // g_pfpFDInference = TDL_FaceDetection;
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}


static CVI_S32 app_ipcam_Ai_FD_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Fd_Param_dump();

    /* 1.Creat Handle */
    if (g_FDAiHandle == NULL)
    {
        g_FDAiHandle = TDL_CreateHandle(0);
        if (g_FDAiHandle == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstFDCfg->model_id_fd);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", g_pstFDCfg->model_id_fd);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_FDAiHandle, g_pstFDCfg->model_id_fd, g_pstFDCfg->model_path_fd, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelPath failed with %#x!\n", g_pstFDCfg->model_path_fd, s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_SetModelThreshold(g_FDAiHandle,  g_pstFDCfg->model_id_fd, g_pstFDCfg->threshold_fd);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s CVI_TDL_SetModelThreshold failed with %#x!\n", g_pstFDCfg->model_path_fd, s32Ret);
        return s32Ret;
    }

    if (g_pstFDCfg->FEA_bEnable) {
        TDL_GetGalleryFeature(g_pstFDCfg->gallery_dir_path, &gallery_feature, 256);
        s32Ret = TDL_OpenModel(g_FDAiHandle, g_pstFDCfg->model_id_fea, g_pstFDCfg->model_path_fea, g_pstFDCfg->model_cfg_path);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelPath failed with %#x!\n", g_pstFDCfg->model_path_fea, s32Ret);
            return s32Ret;
        }

        s32Ret = TDL_OpenModel(g_FDAiHandle, g_pstFDCfg->model_id_landmark, g_pstFDCfg->model_path_landmark, g_pstFDCfg->model_cfg_path);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelPath failed with %#x!\n", g_pstFDCfg->model_path_landmark, s32Ret);
            return s32Ret;
        }
    }

    if (g_pstFDCfg->FD_ATTR_bEnable) {
        s32Ret = TDL_OpenModel(g_FDAiHandle, g_pstFDCfg->model_id_fd_attr, g_pstFDCfg->model_path_fd_attr, NULL);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelPath failed with %#x!\n", g_pstFDCfg->model_path_fd_attr, s32Ret);
            return s32Ret;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD init ------------------> done \n");
    return CVI_SUCCESS;
}

/**
 * 深度拷贝 TDLFace 结构体
 * @param dst 目标对象（需预先分配内存）
 * @param src 源对象
 * @return 0成功，-1失败
 */
static int DeepCopy_TDLFace(TDLFace* dst, const TDLFace* src) {
    // 1. 参数检查
    if (dst == NULL || src == NULL) {
        return -1;
    }

    // 2. 拷贝基本成员
    dst->size = src->size;
    dst->width = src->width;
    dst->height = src->height;

    // 3. 处理 TDLFaceInfo 数组
    if (src->info != NULL && src->size > 0) {
        // 分配新数组内存
        if (dst->info == NULL) {
            dst->info = (TDLFaceInfo*)malloc(MAX_FD_NUM * sizeof(TDLFaceInfo));
        }
        if (dst->info && src->size > MAX_FD_NUM) {
            TDL_ReleaseFaceMeta(dst);
            dst->info = (TDLFaceInfo*)malloc(src->size * sizeof(TDLFaceInfo));
        }
        if (dst->info == NULL) {
            return -1;
        }

        // 逐个拷贝人脸信息
        for (uint32_t i = 0; i < src->size; i++) {
            // 3.1 拷贝简单类型成员
            memcpy(dst->info[i].name, src->info[i].name, sizeof(src->info[i].name));
            dst->info[i].score = src->info[i].score;
            dst->info[i].box.x1 = src->info[i].box.x1;
            dst->info[i].box.x2 = src->info[i].box.x2;
            dst->info[i].box.y1 = src->info[i].box.y1;
            dst->info[i].box.y2 = src->info[i].box.y2;

            // 3.2 深度拷贝landmarks
            dst->info[i].landmarks.size = src->info[i].landmarks.size;
            dst->info[i].landmarks.score = src->info[i].landmarks.score;
            
            if(src->info[i].landmarks.x != NULL) {
                // if(dst->info[i].landmarks.x == NULL) {
                    dst->info[i].landmarks.x = (float*)malloc(sizeof(float));
                    if (dst->info[i].landmarks.x == NULL) {
                        fprintf(stderr, "Error: Failed to allocate memory for landmarks x\n");
                        return -1;
                    }
                // }
                memcpy(dst->info[i].landmarks.x, src->info[i].landmarks.x, sizeof(float));
            }
            
            if(src->info[i].landmarks.y != NULL) {
                // if(dst->info[i].landmarks.y == NULL) {
                    dst->info[i].landmarks.y = (float*)malloc(sizeof(float));
                    if (dst->info[i].landmarks.y == NULL) {
                        fprintf(stderr, "Error: Failed to allocate memory for landmarks y\n");
                        return -1;
                    }
                // }
                memcpy(dst->info[i].landmarks.y, src->info[i].landmarks.y, sizeof(float));
            }
            // 拷贝坐标数据
            // memcpy(dst->info[i].landmarks.x, src->info[i].landmarks.x, sizeof(float));
            // memcpy(dst->info[i].landmarks.y, src->info[i].landmarks.y, sizeof(float));
        }
    } else {
        dst->info = NULL;
        dst->size = 0;
    }

    return 0;
}

static CVI_VOID *Thread_FD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD start running!\n");
    prctl(PR_SET_NAME, "Thread_FD_PROC");

    VPSS_GRP VpssGrp = g_pstFDCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstFDCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    // TDLFace face;
    TDLImage image_handle;
    
    while (app_ipcam_Ai_FD_ProcStatus_Get()) {
        pthread_mutex_lock(&g_FDStatusMutex);
        s32Ret = app_ipcam_Ai_FD_Pause_Get();

        if (s32Ret) {
            pthread_mutex_unlock(&g_FDStatusMutex);
            usleep(1000*1000);
            continue;
        }
        /* 1.Get frame */
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_FDStatusMutex);
            usleep(100*1000);
            continue;
        }

        // 计算总的图像大小
        size_t image_size = stfdFrame.stVFrame.u32Length[0] +
                            stfdFrame.stVFrame.u32Length[1] +
                            stfdFrame.stVFrame.u32Length[2];
        bool isMapped = false;
        // 如果虚拟地址为空，进行内存映射
        if (stfdFrame.stVFrame.pu8VirAddr[0] == NULL) {
            stfdFrame.stVFrame.pu8VirAddr[0] =
                (CVI_U8 *)CVI_SYS_Mmap(stfdFrame.stVFrame.u64PhyAddr[0], image_size);
            isMapped = true;
        }

        image_handle = TDL_WrapFrame((void*)&stfdFrame, false);
        if(image_handle == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, " image_handle is NULL\n");
            CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
            continue;
        }

        pthread_mutex_unlock(&g_FDStatusMutex);
        // iTime_proc = GetCurTimeInMsec();
        /* 2. Face Detect*/
        TDLFace face;
        memset(&face, 0, sizeof(TDLFace));
        g_pfpFDInference(g_FDAiHandle, g_pstFDCfg->model_id_fd, image_handle, &face);

        if (g_pstFDCfg->FEA_bEnable && face.size > 0) {
            TDLImage crop_image;
            s32Ret = TDL_FaceLandmark(g_FDAiHandle, g_pstFDCfg->model_id_landmark, image_handle, &crop_image, &face);
            if (s32Ret == 0) {
                TDLFeature face_feature = {0};
                s32Ret = TDL_FeatureExtraction(g_FDAiHandle, g_pstFDCfg->model_id_fea, crop_image, &face_feature);
                if (s32Ret == 0) {
                    for (uint32_t j = 0; j < gallery_feature.size; j ++) {
                        float similarity = 0.0;
                        s32Ret = TDL_CaculateSimilarity(face_feature, gallery_feature.feature[j], &similarity);
                        if (s32Ret == 0 && similarity > 0.6) {
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "Get Feature similarity to gallery, j = %d, similarity = %f\n", j, similarity); 
                        }
                    }
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "------------------------------------------------------------\n");
                    TDL_ReleaseFeatureMeta(&face_feature);
                }
            }
            TDL_DestroyImage(crop_image);
        }

        if (g_pstFDCfg->FD_ATTR_bEnable && face.size > 0) {
            s32Ret = TDL_FaceAttribute(g_FDAiHandle, g_pstFDCfg->model_id_fd_attr, image_handle, &face);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_FaceAttribute failed with %#x!\n", s32Ret);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "gender score:%f,age score:%f,glass score:%f,emotion score:%f\n",
                                   face.info->gender_score, face.info->age,
                                   face.info->glass_score, face.info->emotion_score);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "Gender:%s\n",
                                   face.info->gender_score > 0.5 ? "Male" : "Female");
                APP_PROF_LOG_PRINT(LEVEL_INFO, "Age:%d\n", (int)round(face.info->age * 100.0));
                APP_PROF_LOG_PRINT(LEVEL_INFO, "Glass:%s\n", face.info->glass_score > 0.5 ? "Yes" : "No");
                APP_PROF_LOG_PRINT(LEVEL_INFO, "Emotion:%s\n", emotion_to_text(face.info->emotion_score));
            }
        }

        if (isMapped) {
            CVI_SYS_Munmap((void *)stfdFrame.stVFrame.u64PhyAddr[0], image_size);
            isMapped = false;
        }

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);

        g_stFDObjDraw.size = 0;
        if (face.size == 0) {
            continue;
        }

        {
            SMT_MutexAutoLock(g_FDMutex, lock);
            DeepCopy_TDLFace(&g_stFDObjDraw, &face);
        }

        TDL_ReleaseFaceMeta(&face);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_FD_ObjDrawInfo_Get(TDLFace *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    pstAiObj->size = 0;
    SMT_MutexAutoLock(g_FDMutex, lock);
    if (g_stFDObjDraw.size == 0){
        return CVI_SUCCESS;
    }
    else
    {
        DeepCopy_TDLFace(pstAiObj, &g_stFDObjDraw);
    }
    return CVI_SUCCESS;
}

int app_ipcam_Ai_FD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstFDCfg->FD_bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_FD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_FD_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_FDThreadHandle)
    {
        pthread_join(g_FDThreadHandle, NULL);
        g_FDThreadHandle = 0;
    }

    if (g_pstFDCfg->FEA_bEnable) {
        for (uint32_t i = 0; i < gallery_feature.size; i++) {
            TDL_ReleaseFeatureMeta(&gallery_feature.feature[i]);
        }
        TDL_CloseModel(g_FDAiHandle, g_pstFDCfg->model_id_fea);
        TDL_CloseModel(g_FDAiHandle, g_pstFDCfg->model_id_landmark);
    }

    if (g_pstFDCfg->FD_ATTR_bEnable) {
        TDL_CloseModel(g_FDAiHandle, g_pstFDCfg->model_id_fd_attr);
    }

    TDL_CloseModel(g_FDAiHandle, g_pstFDCfg->model_id_fd);
    s32Ret = TDL_DestroyHandle(g_FDAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_FD_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_FDAiHandle = NULL;
    }
    TDL_ReleaseFaceMeta(&g_stFDObjDraw);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}


int app_ipcam_Ai_FD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstFDCfg->FD_bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bFDRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_FD_Proc_Init();
	if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Fd_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_FD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_FDThreadHandle, NULL, Thread_FD_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI Fd_pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}
