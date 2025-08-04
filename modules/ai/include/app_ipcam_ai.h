#ifndef __APP_IPCAM_AI_H__
#define __APP_IPCAM_AI_H__

#ifndef __CV184X__
#include "linux/cvi_type.h"
#else
#include "cvi_type.h"
#endif
#ifndef __CV184X__
#include "linux/cvi_comm_video.h"
#else
#include "cvi_comm_video.h"
#endif
#include "cvi_vpss.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_vpss.h"
#include "app_ipcam_venc.h"

#include "tdl_sdk.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MODEL_PATH_LEN  128


#define SMT_MUTEXAUTOLOCK_INIT(mutex) pthread_mutex_t AUTOLOCK_##mutex = PTHREAD_MUTEX_INITIALIZER;

#define SMT_MutexAutoLock(mutex, lock) __attribute__((cleanup(AutoUnLock))) \
       pthread_mutex_t *lock= &AUTOLOCK_##mutex;\
       pthread_mutex_lock(lock);

__attribute__ ((always_inline)) inline void AutoUnLock(void *mutex) {
       pthread_mutex_unlock( *(pthread_mutex_t**) mutex);
}

#define SEND_SIGNAL_DRAW_AI_OBJS_RECT(mutex, osdcCmprCond)  \
    do {                                                    \
        pthread_mutex_lock(&mutex);                         \
        pthread_cond_signal(&osdcCmprCond);                 \
        pthread_mutex_unlock(&mutex);                       \
    } while(0)

#define WAIT_SIGNAL_DRAW_AI_OBJS_RECT(mutex, osdcCmprCond, timeout)  \
    do {                                                             \
        pthread_mutex_lock(&mutex);                                  \
        pthread_cond_timedwait(&osdcCmprCond, &mutex, &timeout);     \
        pthread_mutex_unlock(&mutex);                                \
    } while(0)

typedef CVI_S32 (*pfpInferenceFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLObject *obj);
typedef CVI_S32 (*pfpFaceInferenceFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLFace *obj);
typedef CVI_S32 (*pfpRescaleFunc)(const TDLImage image_handle, TDLObject *obj);

typedef struct APP_PARAM_AI_CAPTURE_CFG_T
{
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    char config_file[128];
    char gallery_dir[128];
}APP_PARAM_AI_CAPTURE_CFG_S;

typedef struct APP_PARAM_AI_FD_CFG_T {
    CVI_BOOL FD_bEnable;
    CVI_BOOL FEA_bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    float threshold_fd;
    TDLModel model_id_fd;
    TDLModel model_id_fea;
    char model_path_fd[MODEL_PATH_LEN];
    char model_path_fea[MODEL_PATH_LEN];
    char gallery_dir_path[MODEL_PATH_LEN];
    char model_cfg_path[MODEL_PATH_LEN];
} APP_PARAM_AI_FD_CFG_S;

typedef struct APP_PARAM_AI_MD_CFG_T {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    CVI_U32 threshold;
    CVI_U32 u32BgUpPeriod;
    CVI_U32 miniArea;
} APP_PARAM_AI_MD_CFG_S;

typedef struct APP_PARAM_AI_PD_CFG_T {
    CVI_BOOL bEnable;
    // CVI_BOOL Intrusion_bEnable;
    // CVI_BOOL capture_enable;
    // CVI_S32 capture_frames;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    // CVI_U32 model_size_w;
    // CVI_U32 model_size_h;
    // CVI_U32 region_stRect_x1;
    // CVI_U32 region_stRect_y1;
    // CVI_U32 region_stRect_x2;
    // CVI_U32 region_stRect_y2;
    // CVI_U32 region_stRect_x3;
    // CVI_U32 region_stRect_y3;
    // CVI_U32 region_stRect_x4;
    // CVI_U32 region_stRect_y4;
    // CVI_U32 region_stRect_x5;
    // CVI_U32 region_stRect_y5;
    // CVI_U32 region_stRect_x6;
    // CVI_U32 region_stRect_y6;
    // CVI_BOOL bVpssPreProcSkip;
    float threshold;
    TDLModel model_id;
    char model_path[MODEL_PATH_LEN];
} APP_PARAM_AI_PD_CFG_S;

#ifdef TDL_CAPTURE_SUPPORT
APP_PARAM_AI_CAPTURE_CFG_S *app_ipcam_Ai_Capture_Param_Get(void);
CVI_BOOL app_ipcam_Ai_Capture_Pause_Get(void);
CVI_VOID app_ipcam_Ai_Capture_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Capture_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Capture_ProcStatus_Set(CVI_BOOL flag);
int app_ipcam_Ai_Capture_Start(void);
int app_ipcam_Ai_Capture_Stop(void);
CVI_VOID app_ipcam_Ai_Cap_ObjDrawInfo_Get(TDLObject *pstAiObj);
#endif

#ifdef TDL_FD_SUPPORT
/* Face detection function */
APP_PARAM_AI_FD_CFG_S *app_ipcam_Ai_FD_Param_Get(void);
CVI_VOID app_ipcam_Ai_FD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_FD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_FD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_FD_Pause_Get(void);
int app_ipcam_Ai_FD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_FD_Start(void);
int app_ipcam_Ai_FD_Stop(void);
int app_ipcam_Ai_FD_ObjDrawInfo_Get(TDLFace *pstAiObj);
CVI_U32 app_ipcam_Ai_FD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_FD_ProcTime_Get(void);
#endif

#ifdef TDL_MD_SUPPORT
/* motion detection function */
APP_PARAM_AI_MD_CFG_S *app_ipcam_Ai_MD_Param_Get(void);
CVI_VOID app_ipcam_Ai_MD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_MD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_MD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_MD_Pause_Get(void);
// int app_ipcam_Ai_MD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_MD_Start(void);
int app_ipcam_Ai_MD_Stop(void);
int app_ipcam_Ai_MD_ObjDrawInfo_Get(TDLObject *pstMdObj);
CVI_U32 app_ipcam_Ai_MD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_MD_ProcTime_Get(void);
CVI_VOID app_ipcam_Ai_MD_Thresold_Set(CVI_U32 value);
CVI_U32 app_ipcam_Ai_MD_Thresold_Get(void);
CVI_S32 app_ipcam_Ai_MD_StatusGet(void);
// CVI_VOID app_ipcam_Ai_MD_obj_Free(cvimd_object_t *pstMdObj);
#endif

#ifdef TDL_PD_SUPPORT
/* Personnel detection function */
APP_PARAM_AI_PD_CFG_S *app_ipcam_Ai_PD_Param_Get(void);
CVI_VOID app_ipcam_Ai_PD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_PD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_PD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_PD_Pause_Get(void);
int app_ipcam_Ai_PD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_PD_Start(void);
int app_ipcam_Ai_PD_Stop(void);
int app_ipcam_Ai_PD_ObjDrawInfo_Get(TDLObject *pstAiObj);
CVI_U32 app_ipcam_Ai_PD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_PD_ProcTime_Get(void);
CVI_S32 app_ipcam_Pd_threshold_Set(float threshold);
CVI_S32 app_ipcam_Ai_Pd_Intrusion_Init(void);
CVI_U32 app_ipcam_Ai_PD_ProcIntrusion_Num_Get(void);
CVI_S32 app_ipcam_Ai_PD_StatusGet(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
