#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <signal.h>
#include "cvi_math.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_stitch.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_buffer.h"
#include "cvi_gdc.h"
#include "cvi_vpss.h"
#include "cvi_stitch.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define GDC_STRIDE_ALIGN 32
#define STITCH_EXT_OP_CLR_VB 0
#define MaxQLength 20
#define STITCH_MAX_TOTAL_SRC_NUM STITCH_MAX_GRP_NUM * STITCH_MAX_SRC_NUM

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct Node {
    SIMPLEQ_ENTRY(Node) field;
    VIDEO_FRAME_INFO_S stVideoFrame;
} Node;

typedef SIMPLEQ_HEAD(Qhead, Node) Qhead;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_STITCH_CFG_S g_stStitchCfg;
APP_PARAM_STITCH_CFG_S *g_pstStitchCfg = &g_stStitchCfg;

static pthread_t g_Stitch_Src_Thread[STITCH_MAX_TOTAL_SRC_NUM] = {0};
static pthread_t g_Stitch_Thread = 0;
static CVI_BOOL g_Stitch_Running = CVI_FALSE;

Qhead *qHead[STITCH_MAX_TOTAL_SRC_NUM] = {NULL};
int qLength[STITCH_MAX_TOTAL_SRC_NUM] = {0};

pthread_cond_t conda[STITCH_MAX_TOTAL_SRC_NUM];
pthread_cond_t condaLength[STITCH_MAX_TOTAL_SRC_NUM];
pthread_mutex_t mutexd[STITCH_MAX_TOTAL_SRC_NUM];
pthread_mutex_t mutexs[STITCH_MAX_TOTAL_SRC_NUM];
pthread_mutex_t mutexqLength[STITCH_MAX_TOTAL_SRC_NUM];

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_STITCH_CFG_S *app_ipcam_Stitch_Param_Get(void)
{
    return g_pstStitchCfg;
}

static CVI_S32 app_stitch_Cfg_Wgt_Image(SIZE_S size, enum stitch_wgt_mode wgtmode, char *name,
                      CVI_U64 *u64PhyAddr, CVI_VOID **pVirAddr, CVI_S32 value)
{
    CVI_U32 wgt_len;

    if (!name || strlen(name) == 0) {
        APP_PROF_LOG_PRINT(
            LEVEL_INFO,
            "wgt image file is null, use average value for wgt map.\n");
        wgt_len = size.u32Width * size.u32Height;
        if (wgtmode == STITCH_WGT_UV_SHARE) wgt_len = wgt_len << 1;
        if (CVI_SYS_IonAlloc(u64PhyAddr, pVirAddr, "stitch_wgt_image",
                             wgt_len) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_IonAlloc NG.\n");
            return CVI_FAILURE;
        }
        if (*u64PhyAddr == 0 || *pVirAddr == 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "CVI_SYS_IonAlloc NG. zero phy/vir address\n");
            return CVI_FAILURE;
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "wgt_len[%d]\n", wgt_len);
        memset(*pVirAddr, value, wgt_len);
    } else {
        FILE *fp;
        int len;
        fp = fopen(name, "rb");
        if (fp == CVI_NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "open data file [%s] error\n",
                               name);
            return CVI_FAILURE;
        }

        wgt_len = size.u32Width * size.u32Height;
        if (wgtmode == STITCH_WGT_UV_SHARE) wgt_len = wgt_len << 1;
        if (CVI_SYS_IonAlloc(u64PhyAddr, pVirAddr, "stitch_wgt_image",
                             wgt_len) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_IonAlloc NG.\n");
            return CVI_FAILURE;
        }
        if (*u64PhyAddr == 0 || *pVirAddr == 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "CVI_SYS_IonAlloc NG. zero phy/vir address\n");
            return CVI_FAILURE;
        }

        APP_PROF_LOG_PRINT(LEVEL_INFO, "wgt_len[%d]\n", wgt_len);

        len = fread(*pVirAddr, wgt_len, 1, fp);
        if (len <= 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "stitch read wgt image fread error, ret(%d)\n",
                               len);
            fclose(fp);
            return CVI_FAILURE;
        }
        fflush(fp);
        fclose(fp);
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_stitch_Set_Wgt_Param(APP_PARAM_STITCH_GRP_CFG_S *pstStitchGrpCfg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    // Set wgtAttr Width & Height
    pstStitchGrpCfg->wgtAttr.size_wgt[0].u32Width = ALIGN(pstStitchGrpCfg->srcAttr.ovlap_attr.ovlp_rx[0] - pstStitchGrpCfg->srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
    pstStitchGrpCfg->wgtAttr.size_wgt[0].u32Height = pstStitchGrpCfg->srcAttr.size[0].u32Height;

    // Set wgtAttr Width & Height
    if (pstStitchGrpCfg->wgtAttr.size_wgt[0].u32Width && pstStitchGrpCfg->wgtAttr.size_wgt[0].u32Height) {
        s32Ret |= app_stitch_Cfg_Wgt_Image(
                    pstStitchGrpCfg->wgtAttr.size_wgt[0],
                    pstStitchGrpCfg->opAttr.wgt_mode,
                    pstStitchGrpCfg->wgt_alpha_name[0],
                    &pstStitchGrpCfg->u64PhyAddr_alpha[0],
                    &pstStitchGrpCfg->VirAddr_alpha[0],
                    pstStitchGrpCfg->wgt_value_alpha[0]);

        s32Ret |= app_stitch_Cfg_Wgt_Image(
                    pstStitchGrpCfg->wgtAttr.size_wgt[0],
                    pstStitchGrpCfg->opAttr.wgt_mode,
                    pstStitchGrpCfg->wgt_beta_name[0],
                    &pstStitchGrpCfg->u64PhyAddr_beta[0],
                    &pstStitchGrpCfg->VirAddr_beta[0],
                    pstStitchGrpCfg->wgt_value_beta[0]);
    }

    // Set wgtAttr Physical Address
    pstStitchGrpCfg->wgtAttr.phy_addr_wgt[0][0] = (__u64) pstStitchGrpCfg->u64PhyAddr_alpha[0];
    pstStitchGrpCfg->wgtAttr.phy_addr_wgt[0][1] = (__u64) pstStitchGrpCfg->u64PhyAddr_beta[0];

    return s32Ret;
}

static CVI_S32 app_stitch_Src_Start(APP_STITCH_SRC_CFG_S *param)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VB_BLK blk;

    /*Frame source from CVI_ID_VPSS*/
    if (param->astChn[0].enModId == CVI_ID_VPSS) {
        VPSS_GRP VpssGrp = param->astChn[0].s32DevId;
        VPSS_CHN VpssChn = param->astChn[0].s32ChnId;
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &param->stImgIn, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_GetChnFrame fail! VpssGrp=%d, VpssChn=%d\n", VpssGrp, VpssChn);
            goto exit1;
        }
    }

    /*Frame source from CVI_ID_BASE*/
    if (param->astChn[0].enModId == CVI_ID_BASE) {
        (CVI_VOID) param->astChn[0].s32DevId;
        (CVI_VOID) param->astChn[0].s32ChnId;
    }

    /*Frame source from CVI_ID_STITCH*/
    if (param->astChn[0].enModId == CVI_ID_STITCH) {
        STITCH_GRP StitchGrp = param->astChn[0].s32DevId;
        /* Stitch chn id is not used */
        (CVI_VOID) param->astChn[0].s32ChnId;
        s32Ret = CVI_STITCH_GetChnFrame(StitchGrp, &param->stImgIn, 30000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_GetChnFrame fail! StitchGrp=%d\n", StitchGrp);
            /*Increase semv to wake up CVI_STITCH_SendFrame*/
            sem_post(param->semv);
            goto exit1;
        }
    }
    /*Increase semv to wake up CVI_STITCH_SendFrame*/
    sem_post(param->semv);

    if(qLength[param->src_idx] >= MaxQLength) {
        //APP_PROF_LOG_PRINT(LEVEL_INFO, "qLengthL: %d full\n", qLengthL);
        goto exit1;
    }

    /*Insert iNode to tail of the queue*/
    pthread_mutex_lock(&mutexd[param->src_idx]);
    Node *iNode = (Node *)malloc(sizeof(Node));
    iNode->stVideoFrame = param->stImgIn;
    SIMPLEQ_INSERT_TAIL(qHead[param->src_idx], iNode, field);
    pthread_cond_signal(&conda[param->src_idx]);
    pthread_mutex_unlock(&mutexd[param->src_idx]);

    pthread_mutex_lock(&mutexqLength[param->src_idx]);
    qLength[param->src_idx]++;
    pthread_mutex_unlock(&mutexqLength[param->src_idx]);
    return s32Ret;

exit1:
    if (param->stImgIn.stVFrame.u64PhyAddr[0]) {
        blk = CVI_VB_PhysAddr2Handle(param->stImgIn.stVFrame.u64PhyAddr[0]);
        if (blk != VB_INVALID_HANDLE) {
            s32Ret = CVI_VB_ReleaseBlock(blk);
        }
    }

    return s32Ret;
}

static CVI_S32 app_stitch_src_thread(void *arg)
{
    APP_STITCH_SRC_CFG_S *param = (APP_STITCH_SRC_CFG_S *)arg;

    while (g_Stitch_Running) {
        if (0 != app_stitch_Src_Start(param)) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_stitch_Src_Start failed!\n");
        }
    }
    return 0;
}

static CVI_S32 app_ipcam_stitch_proc()
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VB_BLK blk;
    CVI_U64 u64PhyAddr;
    Node *node;
    int i = 0, j = 0, index = 0;

    while (g_Stitch_Running) {

        /*Send frame to Stitch if ready
        * Only in stitch unbind mode, need to wait for semaphore and enter this block
        */
        for (i = 0; i < g_pstStitchCfg->s32GrpCnt; i++){
            for (j = 0; j < g_pstStitchCfg->astStitchGrpCfg[i].srcNum; j++) {
                if (g_pstStitchCfg->astStitchGrpCfg[i].srcParam[j].bBindEn == 0) {
                    index = g_pstStitchCfg->astStitchGrpCfg[i].srcParam[j].src_idx;

                    sem_wait(g_pstStitchCfg->astStitchGrpCfg[i].srcParam[j].semv);

                    pthread_mutex_lock(&mutexs[index]);
                    if (SIMPLEQ_FIRST(qHead[index]) == NULL) {
                        pthread_cond_wait(&conda[index], &mutexs[index]);
                    }
                    s32Ret = CVI_STITCH_SendFrame(g_pstStitchCfg->astStitchGrpCfg[i].grpId,
                        (STITCH_SRC_IDX) j,
                        &SIMPLEQ_FIRST(qHead[index])->stVideoFrame,
                        3000);
                    u64PhyAddr = SIMPLEQ_FIRST(qHead[index])->stVideoFrame.stVFrame.u64PhyAddr[0];
                    node = SIMPLEQ_FIRST(qHead[index]);
                    SIMPLEQ_REMOVE_HEAD(qHead[index], field);
                    pthread_mutex_unlock(&mutexs[index]);
                    free(node);

                    pthread_mutex_lock(&mutexqLength[index]);
                    qLength[index]--;
                    pthread_mutex_unlock(&mutexqLength[index]);

                    if (u64PhyAddr) {
                        blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
                        if (blk != VB_INVALID_HANDLE) {
                            s32Ret = CVI_VB_ReleaseBlock(blk);
                        }
                    }
                }
            }
        }
    }
    return s32Ret;
}

CVI_S32 app_ipcam_Stitch_UnInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    int i = 0, j = 0, k = 0, m = 0, index = 0;
    CVI_U64 u64PhyAddr;
    VB_BLK blk;
    Node *node;
    CVI_BOOL Enable = g_pstStitchCfg->Enable;

    g_pstStitchCfg->Enable = CVI_FALSE;

    if (Enable) {
        g_Stitch_Running = CVI_FALSE;

        /*
        * Uninit Stitch Src Thread in unbind mode
        */
        for (j = 0; j < g_pstStitchCfg->s32GrpCnt; j++) {
            for (k = 0; k < g_pstStitchCfg->astStitchGrpCfg[j].srcNum; k++) {
                if (g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].bBindEn == 0) {
                    index = g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].src_idx;
                    if (g_Stitch_Src_Thread[index] != 0) {
                        pthread_kill(g_Stitch_Src_Thread[index], SIGUSR1);
                        pthread_join(g_Stitch_Src_Thread[index], NULL);
                        g_Stitch_Src_Thread[index] = 0;
                    }
                } else {
                    s32Ret = CVI_SYS_UnBind(&g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].astChn[0],
                                            &g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].astChn[1]);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_SYS_UnBind failed with %#x\n", s32Ret);
                        return s32Ret;
                    }
                }
            }
        }

        /*Uninit Stitch Proc Thread*/
        if (g_Stitch_Thread != 0) {
            pthread_kill(g_Stitch_Thread, SIGUSR1);
            pthread_join(g_Stitch_Thread, NULL);
            g_Stitch_Thread = 0;
        }

        /*Uninit semaphore*/
        for (k = 0; k < g_pstStitchCfg->s32GrpCnt; k++) {
            for (m = 0; m < g_pstStitchCfg->astStitchGrpCfg[k].srcNum; m++) {
                if (g_pstStitchCfg->astStitchGrpCfg[k].srcParam[m].bBindEn == 0) {
                    pthread_cond_destroy(&conda[m]);
                    pthread_mutex_destroy(&mutexd[m]);
                    pthread_mutex_destroy(&mutexs[m]);
                    pthread_mutex_destroy(&mutexqLength[m]);

                    sem_destroy(g_pstStitchCfg->astStitchGrpCfg[k].srcParam[m].semv);
                    free(g_pstStitchCfg->astStitchGrpCfg[k].srcParam[m].semv);
                }
            }
        }

        /*Uninit qHead[]*/
        for (k = 0; k < g_pstStitchCfg->s32GrpCnt; k++){
            for (m = 0; m < g_pstStitchCfg->astStitchGrpCfg[k].srcNum; m++){
                if (g_pstStitchCfg->astStitchGrpCfg[k].srcParam[m].bBindEn == 0) {
                    index = g_pstStitchCfg->astStitchGrpCfg[k].srcParam[m].src_idx;
                    while(SIMPLEQ_FIRST(qHead[index]) != NULL) {
                        u64PhyAddr = SIMPLEQ_FIRST(qHead[index])->stVideoFrame.stVFrame.u64PhyAddr[0];
                        node = SIMPLEQ_FIRST(qHead[index]);
                        SIMPLEQ_REMOVE_HEAD(qHead[index], field);
                        if (node) {
                            free(node);
                        }

                        if (u64PhyAddr) {
                            blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
                            if (blk != VB_INVALID_HANDLE) {
                                s32Ret = CVI_VB_ReleaseBlock(blk);
                            }
                        }
                    }
                }
            }
        }

        /*Free memory region of Wgt Image */
        for (i = 0; i < g_pstStitchCfg->s32GrpCnt; i++){
            for (j = 0; j < (g_pstStitchCfg->astStitchGrpCfg[i].srcNum - 1); j++) {
                if (g_pstStitchCfg->astStitchGrpCfg[i].u64PhyAddr_alpha[j]
                    && g_pstStitchCfg->astStitchGrpCfg[i].VirAddr_alpha[j])
                    CVI_SYS_IonFree(g_pstStitchCfg->astStitchGrpCfg[i].u64PhyAddr_alpha[j]
                                    , g_pstStitchCfg->astStitchGrpCfg[i].VirAddr_alpha[j]);

                if (g_pstStitchCfg->astStitchGrpCfg[i].u64PhyAddr_beta[j]
                    && g_pstStitchCfg->astStitchGrpCfg[i].VirAddr_beta[j])
                    CVI_SYS_IonFree(g_pstStitchCfg->astStitchGrpCfg[i].u64PhyAddr_beta[j]
                                    , g_pstStitchCfg->astStitchGrpCfg[i].VirAddr_beta[j]);
            }
        }

        for (i = 0; i < g_pstStitchCfg->s32GrpCnt; i++) {
            /*Disable Stitch Group*/
            s32Ret = CVI_STITCH_DisableGrp(i);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_DisableGrp failed!\n");
                return CVI_FAILURE;
            }

            /*DeInit Stitch Group*/
            s32Ret = CVI_STITCH_DeInitGrp(i);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_DeInitGrp failed!\n");
                return CVI_FAILURE;
            }
        }

        /*DeInit Stitch*/
        s32Ret = CVI_STITCH_DeInit();
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_DeInit failed!\n");
            return CVI_FAILURE;
        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_Stitch_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_BOOL Enable = g_pstStitchCfg->Enable;
    int i = 0, j = 0, k = 0, index = 0;

    if (Enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "stitch init ------------------> start \n");
        g_Stitch_Running = CVI_TRUE;

        /*
        * Loop through each stitch group.
        * determines the stitch source mode
        * srcNum = 2 corresponds to STITCH_2_WAY,
        * srcNum = 4 corresponds to STITCH_4_WAY.
        * Only sources with bind mode disabled (bBindEn == false) will enter this block.
        * If the image source is in bind mode, this block will be skipped.
        */
        for (j = 0; j < g_pstStitchCfg->s32GrpCnt; j++) {
            for (k = 0; k < g_pstStitchCfg->astStitchGrpCfg[j].srcNum; k++) {
                if (g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].bBindEn == 0) {
                    index = g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].src_idx;
                    qLength[index] = 0;
                    qHead[index] = (Qhead *)malloc(sizeof(Qhead));

                    SIMPLEQ_INIT(qHead[index]);
                    pthread_cond_init(&conda[index], NULL);
                    pthread_mutex_init(&mutexd[index], NULL);
                    pthread_mutex_init(&mutexs[index], NULL);
                    pthread_mutex_init(&mutexqLength[index], NULL);

                    g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].semv = (sem_t *)malloc(sizeof(sem_t));
                    sem_init(g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].semv, 0, 1);
                }
            }
        }

        s32Ret = CVI_STITCH_Init();
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH INIT failed!\n");
            return s32Ret;
        }

        s32Ret = CVI_STITCH_Reset();
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_Reset failed!\n");
            goto STITCH_EXIT1;
        }

        for (i = 0; i < g_pstStitchCfg->s32GrpCnt; i++) {
            /*Init Stitch Group*/
            s32Ret = CVI_STITCH_InitGrp(i);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_InitGrp failed!\n");
                goto STITCH_EXIT2;
            }

            /*Alloc memory region for wgt image*/
            s32Ret = app_stitch_Set_Wgt_Param(&g_pstStitchCfg->astStitchGrpCfg[i]);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_stitch_Set_Wgt_Param failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- SetSrcAttr*/
            s32Ret = CVI_STITCH_SetSrcAttr(i, &g_pstStitchCfg->astStitchGrpCfg[i].srcAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_SetSrcAttr failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- SetChnAttr*/
            s32Ret = CVI_STITCH_SetChnAttr(i, &g_pstStitchCfg->astStitchGrpCfg[i].chnAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_SetChnAttr failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- SetOptAttr*/
            s32Ret = CVI_STITCH_SetOpAttr(i, &g_pstStitchCfg->astStitchGrpCfg[i].opAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_SetOpAttr failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- SetWgtAttr*/
            s32Ret = CVI_STITCH_SetWgtAttr(i, &g_pstStitchCfg->astStitchGrpCfg[i].wgtAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_SetWgtAttr failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- SetRegX*/
            s32Ret = CVI_STITCH_SetRegX(16);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_SetRegX failed!\n");
                goto STITCH_EXIT2;
            }

            /*Set Stitch Param -- Attach VB Pool*/
            if (g_pstStitchCfg->astStitchGrpCfg[i].bAttachEn) {
                s32Ret = CVI_STITCH_AttachVbPool(i, (VB_POOL)g_pstStitchCfg->astStitchGrpCfg[i].u32AttachVbPool);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_AttachVbPool failed!\n");
                    goto STITCH_EXIT2;
                }
            }
        }

        for (i = 0; i < g_pstStitchCfg->s32GrpCnt; i++) {
            /*After setting param, Enable Device*/
            s32Ret = CVI_STITCH_EnableGrp(i);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_STITCH_EnableGrp failed!\n");
                goto STITCH_EXIT3;
            }
        }

        /*
        * Only in stitch unbind mode, need to create thread to get frame
        * In bind mode, will bind two sources directly
        */
        for (j = 0; j < g_pstStitchCfg->s32GrpCnt; j++) {
            for (k = 0; k < g_pstStitchCfg->astStitchGrpCfg[j].srcNum; k++) {
                /*UnBind mode will create thread to get frame*/
                if (g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].bBindEn == 0) {
                    index = g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].src_idx;
                    s32Ret = pthread_create(&g_Stitch_Src_Thread[index], NULL, (void *)app_stitch_src_thread, &g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k]);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create src[%d] failed!\n", index);
                        goto STITCH_EXIT3;
                    }
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "pthread_create app_stitch_src_thread src[%d] success!\n", index);
                } else {
                    /*Bind mode will skip create thread and bind two sources*/
                    s32Ret = CVI_SYS_Bind(&g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].astChn[0],
                                          &g_pstStitchCfg->astStitchGrpCfg[j].srcParam[k].astChn[1]);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_Bind failed with %#x\n", s32Ret);
                        goto STITCH_EXIT3;
                    }
                }
            }
        }

        /*Init Stitch Proc Thread*/
        s32Ret = pthread_create(&g_Stitch_Thread, NULL, (void *)app_ipcam_stitch_proc, g_pstStitchCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create stitch failed!\n");
            goto STITCH_EXIT3;
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "pthread_create app_ipcam_stitch_proc success!\n");

        APP_PROF_LOG_PRINT(LEVEL_INFO, "stitch init ------------------> end \n");
    }

    return s32Ret;

STITCH_EXIT3:
    for(j = 0; j < g_pstStitchCfg->s32GrpCnt; j++){
        CVI_STITCH_DisableGrp(j);
    }
STITCH_EXIT2:
    for(j = 0; j < g_pstStitchCfg->s32GrpCnt; j++){
        CVI_STITCH_DeInitGrp(j);
    }
STITCH_EXIT1:
    CVI_STITCH_DeInit();

    return s32Ret;
}