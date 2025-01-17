#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "app_ipcam_comm.h"
#include "app_ipcam_websocket.h"
#include "libwebsockets.h"
#include "cvi_mbuf.h"

#ifdef MD_SUPPORT
#include "app_ipcam_md.h"
#endif

#ifdef AI_SUPPORT
#include "app_ipcam_ai.h"
#endif
#define MAX_PAYLOAD_SIZE (1024 * 1024)
#define MAX_BUFFER_SIZE (256*1024)

struct lws *g_wsi = NULL;
static unsigned char *s_aiData = NULL;
static int s_aiSize = 0;
static int g_CurVencChn = 1;    /* default set to sub-streaming */
static volatile int s_terminal = 0;
pthread_mutex_t g_aiMutexLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_websocketThread;
CVI_MBUF_HANDLE* g_WebReaderid = NULL;
CVI_MEDIA_FRAME_INFO_T g_WebReadFrameInfo;

struct sessionData_s
{
    int msg_count;
    unsigned char buf[MAX_PAYLOAD_SIZE];
    int len;
    int bin;
    int fin;
};

int app_ipcam_WebSocketChn_Get()
{
    return g_CurVencChn;
}

int app_ipcam_WebSocketChn_Set(int chn)
{
    g_CurVencChn = chn;

    return 0;
}

static void CVI_IPC_WebsocketRequestIDR(void)
{
    CVI_VENC_RequestIDR(g_CurVencChn, 1);
}

static int ProtocolMyCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct sessionData_s *data = (struct sessionData_s *)user;
    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        printf("Client LWS_CALLBACK_ESTABLISHED!\n");
        g_wsi = wsi;
        CVI_IPC_WebsocketRequestIDR();
        if (NULL == g_WebReaderid)
        {
            g_WebReaderid = CVI_MBUF_CreateReader(g_CurVencChn, 1);
        }

        break;
    case LWS_CALLBACK_RECEIVE:
        lws_rx_flow_control(wsi, 0);
        char *tmp_data = (char *)malloc(len + 1);
        memset(tmp_data, 0, len + 1);
        memcpy(tmp_data, in, len);
        data->len = len;
        free(tmp_data);
        // lws_callback_on_writable(wsi);
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        if (g_WebReaderid && g_WebReadFrameInfo.frameBuf)
        {
            lws_callback_on_writable(g_wsi);
            memset(g_WebReadFrameInfo.frameBuf, 0, CVI_MBUF_STREAM_MAX_SIZE);
            g_WebReadFrameInfo.frameParam.frameLen = CVI_MBUF_STREAM_MAX_SIZE;
            if (0 < CVI_MBUF_ReadFrame(g_WebReaderid, 0, &g_WebReadFrameInfo, 100))
            {
                lws_write(wsi, g_WebReadFrameInfo.frameBuf, g_WebReadFrameInfo.frameParam.frameLen + 1, LWS_WRITE_BINARY); // LWS_WRITE_BINARY LWS_WRITE_TEXT
            }
        }

        pthread_mutex_lock(&g_aiMutexLock);

        if (s_aiSize != 0 && NULL != s_aiData) {
            lws_write(wsi, s_aiData + LWS_PRE, s_aiSize, LWS_WRITE_BINARY); // LWS_WRITE_BINARY LWS_WRITE_TEXT
        } else {
            //printf("----- websocket filesize is 0\n");
        }
        if (s_aiData != NULL) {
            free(s_aiData);
            s_aiData = NULL;
        }
        pthread_mutex_unlock(&g_aiMutexLock);

        lws_rx_flow_control(wsi, 1);
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        printf("client LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
        break;
    case LWS_CALLBACK_WSI_DESTROY:
    case LWS_CALLBACK_CLOSED:
        printf("client close WebSocket\n");
        if (g_WebReaderid)
        {
            CVI_MBUF_DestoryReader(g_WebReaderid);
            g_WebReaderid = NULL;
        }
        g_wsi = NULL;
        // CVI_IPC_CleanHumanTraffic();

		pthread_mutex_lock(&g_aiMutexLock);
		if (s_aiData) {
			free(s_aiData);
			s_aiData = NULL;
		}
        s_aiSize = 0;
		pthread_mutex_unlock(&g_aiMutexLock);
        break;
    default:
        break;
    }
    return 0;
}

struct lws_protocols protocols[] = {
    {"ws",                          /*协议名*/
     ProtocolMyCallback,            /*回调函数*/
     sizeof(struct sessionData_s),  /*定义新连接分配的内存,连接断开后释放*/
     MAX_PAYLOAD_SIZE,              /*定义rx 缓冲区大小*/
     0,
     NULL,
     0},
    {NULL, NULL, 0, 0, 0, 0, 0} /*结束标志*/
};

static void *ThreadWebsocket(void *arg)
{
    struct lws_context_creation_info ctx_info = {0};
    ctx_info.port = 8000;
    ctx_info.iface = NULL;  // 在所有网络接口上监听
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;
    ctx_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

    // ctx_info.ssl_ca_filepath = "../ca/ca-cert.pem";
    // ctx_info.ssl_cert_filepath = "./server-cert.pem";
    // ctx_info.ssl_private_key_filepath = "./server-key.pem";
    // ctx_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    // ctx_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;

    struct lws_context *context = lws_create_context(&ctx_info);
    while (!s_terminal) {
        lws_service(context, 1000);
    }
    lws_context_destroy(context);

    return NULL;
}

int app_ipcam_WebSocket_Init()
{
    int s32Ret = 0;

    memset(&g_WebReadFrameInfo.frameParam, 0, sizeof(g_WebReadFrameInfo.frameParam));
    g_WebReadFrameInfo.frameBuf = malloc(CVI_MBUF_STREAM_MAX_SIZE);
    if (NULL == g_WebReadFrameInfo.frameBuf)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "frameBuf malloc fail\n");
        return -1;
    }

    s32Ret = pthread_create(&g_websocketThread, NULL, ThreadWebsocket, NULL);
    return s32Ret;
}

int app_ipcam_WebSocket_DeInit()
{
    s_terminal = 1;
    if (g_websocketThread) {
        pthread_join(g_websocketThread, NULL);
        g_websocketThread = 0;
    }

    if (g_WebReadFrameInfo.frameBuf)
    {
        free(g_WebReadFrameInfo.frameBuf);
        g_WebReadFrameInfo.frameBuf = NULL;
    }

    return 0;
}

int app_ipcam_WebSocket_Stream_Send(void *pData, void *pArgs)
{

    return 0;
}

int app_ipcam_WebSocket_AiFps_Send(void)
{
    if (g_wsi == NULL) {
        return 0;
    }
    char AiFps[10] = {0};
    #if defined AI_SUPPORT && defined MD_SUPPORT && defined PD_SUPPORT
    snprintf(AiFps, sizeof(AiFps), "%d %d %d", app_ipcam_Ai_PD_ProcFps_Get(), app_ipcam_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcIntrusion_Num_Get());
    #else
    snprintf(AiFps, sizeof(AiFps), "N N N");
    #endif
    int len = strlen(AiFps);

    pthread_mutex_lock(&g_aiMutexLock);
    if (s_aiData != NULL) {
        free(s_aiData);
        s_aiData = NULL;
    }

    s_aiData = malloc(LWS_PRE + 1 + len); // one for type, one for reserve
    if (s_aiData == NULL) {
        printf("error, malloc img buff failed\n");
        pthread_mutex_unlock(&g_aiMutexLock);
        return -1;
    }
    memset(s_aiData, 0, LWS_PRE + 1 + len);

    s_aiData[LWS_PRE] = 3 & 0xff;

    memcpy(s_aiData + LWS_PRE + 1, AiFps, len);

    s_aiSize = len + 1;

    if (g_wsi != NULL) {
        lws_callback_on_writable(g_wsi);
    } else {
        free(s_aiData);
        s_aiData = NULL;
    }
    pthread_mutex_unlock(&g_aiMutexLock);
   
    return CVI_SUCCESS;
}
// static void FrameUnmmap(VIDEO_FRAME_INFO_S *frame)
// {
//     size_t length = 0;
//     for (int i = 0; i < 3; ++i)
//     {
//         length += frame->stVFrame.u32Length[i];
//     }
//     CVI_SYS_Munmap((void *)frame->stVFrame.pu8VirAddr[0], length);
// }

// static int FrameMmap(VIDEO_FRAME_INFO_S *frame)
// {
//     size_t length = 0;
//     for (int i = 0; i < 3; ++i)
//     {
//         length += frame->stVFrame.u32Length[i];
//     }
//     frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[0],
//                                                                 length);
//     if (!frame->stVFrame.pu8VirAddr[0])
//     {
//         printf("mmap frame virtual addr failed\n");
//         return -1;
//     }
//     frame->stVFrame.pu8VirAddr[1] = frame->stVFrame.pu8VirAddr[0] + frame->stVFrame.u32Length[0];
//     frame->stVFrame.pu8VirAddr[2] = frame->stVFrame.pu8VirAddr[1] + frame->stVFrame.u32Length[1];
//     return 0;
// }
