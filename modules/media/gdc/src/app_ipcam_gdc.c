#include "app_ipcam_comm.h"
#include "app_ipcam_gdc.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define DUMP_MESH_FOLDER "/mnt/nfs"
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_GDC_CFG_S g_stGDCAttrCfg, *g_pstGDCAttrCfg = &g_stGDCAttrCfg;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
APP_PARAM_GDC_CFG_S *app_ipcam_GDC_Param_Get(void)
{
    return g_pstGDCAttrCfg;
}

int app_ipcam_GDC_DumpMesh(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    MESH_DUMP_ATTR_S stMeshDumpAttr = {0};

    stMeshDumpAttr.enModId = g_pstGDCAttrCfg->enModId;
    switch (g_pstGDCAttrCfg->enModId) {
        case CVI_ID_VI:
            for (int vi_idx = 0; vi_idx < g_pstGDCAttrCfg->viCnt; vi_idx++) {
                snprintf(stMeshDumpAttr.binFileName
                    , sizeof(stMeshDumpAttr.binFileName)
                    , "%s/dump_mesh_vi_pipe%d_chn%d_idx%d.bin"
                    , DUMP_MESH_FOLDER
                    , g_pstGDCAttrCfg->stGDCViCfg[vi_idx].ViPipe
                    , g_pstGDCAttrCfg->stGDCViCfg[vi_idx].ViChn
                    , vi_idx);
                stMeshDumpAttr.viMeshAttr.chn =
                    g_pstGDCAttrCfg->stGDCViCfg[vi_idx].ViChn;
                s32Ret = CVI_GDC_DumpMesh(&stMeshDumpAttr);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "CVI_GDC_DumpMesh failed with s32Ret:%#x. \n", s32Ret);
                    return s32Ret;
                }
            }
            break;
        case CVI_ID_VPSS:
            for (int vpss_idx = 0; vpss_idx < g_pstGDCAttrCfg->vpssCnt; vpss_idx++) {
                snprintf(stMeshDumpAttr.binFileName
                    , sizeof(stMeshDumpAttr.binFileName)
                    , "%s/dump_mesh_vpss_grp%d_chn%d_idx%d.bin"
                    , DUMP_MESH_FOLDER
                    , g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx].VpssGrp
                    , g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx].VpssChn
                    , vpss_idx);
                stMeshDumpAttr.vpssMeshAttr.grp =
                    g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx].VpssGrp;
                stMeshDumpAttr.vpssMeshAttr.chn =
                    g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx].VpssChn;
                s32Ret = CVI_GDC_DumpMesh(&stMeshDumpAttr);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "CVI_GDC_DumpMesh failed with s32Ret:%#x. \n", s32Ret);
                    return s32Ret;
                }
            }
            break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support. modId:%d.\n"
                , g_pstGDCAttrCfg->enModId);
            return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

static int app_ipcam_GDC_Config_Vi(APP_PARAM_GDC_VI_CFG_S *pstGDCViCfg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VI_PIPE ViPipe = pstGDCViCfg->ViPipe;
    VI_CHN  ViChn  = pstGDCViCfg->ViChn;
    VI_LDC_ATTR_S stLDCViAttr = {0};
    MESH_DUMP_ATTR_S *pMeshDumpAttr = &pstGDCViCfg->stMeshAttr;
    LDC_ATTR_S       *pstLDCAttr    = &pstGDCViCfg->stLDCAttr;

    if (pstGDCViCfg == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR
            , "app_ipcam_GDC_Config_Vi param == NULL. \n");
        return CVI_FAILURE;
    }

    if (pstGDCViCfg->enType == GDC_GRIDINFO) {
        stLDCViAttr.bEnable = pstGDCViCfg->bEnable;
        memcpy(&stLDCViAttr.stAttr, pstLDCAttr, sizeof(LDC_ATTR_S));
        memcpy(stLDCViAttr.stAttr.stGridInfoAttr.gridFileName
            , pstLDCAttr->stGridInfoAttr.gridFileName
            , 128);
        memcpy(stLDCViAttr.stAttr.stGridInfoAttr.gridBindName
            , pstLDCAttr->stGridInfoAttr.gridBindName
            , 128);
        s32Ret = CVI_VI_SetChnLDCAttr(ViPipe, ViChn, &stLDCViAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VPSS_SetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }
    } else if (pstGDCViCfg->enType == GDC_MESH) {
        s32Ret = CVI_VI_GetChnLDCAttr(ViPipe, ViChn, &stLDCViAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VI_GetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }

        stLDCViAttr.bEnable = CVI_FALSE;
        s32Ret = CVI_VI_SetChnLDCAttr(ViPipe, ViChn, &stLDCViAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VI_SetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }

        if (pstGDCViCfg->bEnable == CVI_TRUE) {
            s32Ret = CVI_GDC_LoadMesh(pMeshDumpAttr, pstLDCAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR
                    , "CVI_GDC_LoadMesh failed with s32Ret:%#x. \n", s32Ret);
                return s32Ret;
            }
        }
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support. enType:%d.\n"
            , pstGDCViCfg->enType);
        return CVI_FAILURE;
    }

    return s32Ret;
}

static int app_ipcam_GDC_Config_Vpss(APP_PARAM_GDC_VPSS_CFG_S *pstGDCVpssCfg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_GRP VpssGrp = pstGDCVpssCfg->VpssGrp;
    VPSS_CHN VpssChn = pstGDCVpssCfg->VpssChn;
    VPSS_LDC_ATTR_S  stLDCVpssAttr  = {0};
    MESH_DUMP_ATTR_S *pMeshDumpAttr = &pstGDCVpssCfg->stMeshAttr;
    LDC_ATTR_S *pstLDCAttr = &pstGDCVpssCfg->stLDCAttr;

    if (pstGDCVpssCfg == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR
            , "app_ipcam_GDC_Config_Vpss param == NULL. \n");
        return CVI_FAILURE;
    }

    if (pstGDCVpssCfg->enType == GDC_GRIDINFO) {
        stLDCVpssAttr.bEnable = pstGDCVpssCfg->bEnable;
        memcpy(&stLDCVpssAttr.stAttr, pstLDCAttr, sizeof(LDC_ATTR_S));
        memcpy(stLDCVpssAttr.stAttr.stGridInfoAttr.gridFileName
            , pstLDCAttr->stGridInfoAttr.gridFileName
            , 128);
        memcpy(stLDCVpssAttr.stAttr.stGridInfoAttr.gridBindName
            , pstLDCAttr->stGridInfoAttr.gridBindName
            , 128);
        s32Ret = CVI_VPSS_SetChnLDCAttr(VpssGrp, VpssChn, &stLDCVpssAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VPSS_SetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }
    } else if (pstGDCVpssCfg->enType == GDC_MESH) {
        s32Ret = CVI_VPSS_GetChnLDCAttr(VpssGrp, VpssChn, &stLDCVpssAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VPSS_GetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }
        stLDCVpssAttr.bEnable = CVI_FALSE;
        s32Ret = CVI_VPSS_SetChnLDCAttr(VpssGrp, VpssChn, &stLDCVpssAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR
                , "CVI_VPSS_SetChnLDCAttr failed with s32Ret:%#x. \n", s32Ret);
            return s32Ret;
        }

        if ((pstGDCVpssCfg->bEnable == CVI_TRUE)
            && strlen(pstGDCVpssCfg->stMeshAttr.binFileName)) {
            s32Ret = CVI_GDC_LoadMesh(pMeshDumpAttr, pstLDCAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR
                    , "CVI_GDC_LoadMesh failed with s32Ret:%#x. \n", s32Ret);
                return s32Ret;
            }
        }
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support. enType:%d.\n"
            , pstGDCVpssCfg->enType);
        return CVI_FAILURE;
    }

    return s32Ret;
}

int app_ipcam_GDC_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PARAM_GDC_VI_CFG_S   *pstGDCViCfg   = g_pstGDCAttrCfg->stGDCViCfg;
    APP_PARAM_GDC_VPSS_CFG_S *pstGDCVpssCfg = g_pstGDCAttrCfg->stGDCVpssCfg;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_GDC_Init start. \n");

    switch(g_pstGDCAttrCfg->enModId){
        case CVI_ID_VI:
            for (int vi_idx = 0; vi_idx < g_pstGDCAttrCfg->viCnt; vi_idx++) {
                pstGDCViCfg = &g_pstGDCAttrCfg->stGDCViCfg[vi_idx];
                pstGDCViCfg->bEnable = CVI_TRUE;
                s32Ret = app_ipcam_GDC_Config_Vi(pstGDCViCfg);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "app_ipcam_GDC_Config_Vi failed with s32Ret:%#x.\n", s32Ret);
                }
                pstGDCViCfg = NULL;
            }
            break;
        case CVI_ID_VPSS:
            for (int vpss_idx = 0; vpss_idx < g_pstGDCAttrCfg->vpssCnt; vpss_idx++) {
                pstGDCVpssCfg = &g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx];
                pstGDCVpssCfg->bEnable = CVI_TRUE;
                s32Ret = app_ipcam_GDC_Config_Vpss(pstGDCVpssCfg);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "app_ipcam_GDC_Config_Vpss failed with s32Ret:%#x.\n", s32Ret);
                }
                pstGDCVpssCfg = NULL;
            }
            break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support. modId:%d.\n"
                , g_pstGDCAttrCfg->enModId);
            break;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_GDC_Init done. \n");

    return s32Ret;
}

int app_ipcam_GDC_DeInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PARAM_GDC_VI_CFG_S   *pstGDCViCfg   = g_pstGDCAttrCfg->stGDCViCfg;
    APP_PARAM_GDC_VPSS_CFG_S *pstGDCVpssCfg = g_pstGDCAttrCfg->stGDCVpssCfg;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_GDC_DeInit start. \n");

    switch(g_pstGDCAttrCfg->enModId){
        case CVI_ID_VI:
            for (int vi_idx = 0; vi_idx < g_pstGDCAttrCfg->viCnt; vi_idx++) {
                pstGDCViCfg = &g_pstGDCAttrCfg->stGDCViCfg[vi_idx];
                pstGDCViCfg->bEnable = CVI_FALSE;
                s32Ret = app_ipcam_GDC_Config_Vi(pstGDCViCfg);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "app_ipcam_GDC_Config_Vi failed with s32Ret:%#x.\n", s32Ret);
                }
                pstGDCViCfg = NULL;
            }
            break;
        case CVI_ID_VPSS:
            for (int vpss_idx = 0; vpss_idx < g_pstGDCAttrCfg->vpssCnt; vpss_idx++) {
                pstGDCVpssCfg = &g_pstGDCAttrCfg->stGDCVpssCfg[vpss_idx];
                pstGDCVpssCfg->bEnable = CVI_FALSE;
                s32Ret = app_ipcam_GDC_Config_Vpss(pstGDCVpssCfg);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR
                        , "app_ipcam_GDC_Config_Vpss failed with s32Ret:%#x.\n", s32Ret);
                }
                pstGDCVpssCfg = NULL;
            }
            break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support. modId:%d.\n"
                , g_pstGDCAttrCfg->enModId);
            break;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_GDC_DeInit done. \n");

    return s32Ret;
}
