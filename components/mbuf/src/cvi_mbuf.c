#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_mbuf.h"
#include "zfifo.h"
#include "cvi_log.h"

static ZFIFO *s_media_fifo[CVI_MBUF_MAX_NUM] = {0};

static CVI_MBUF_HANDLE s_hVideoWriterId[CVI_MBUF_MAX_NUM];

static int cvi_mbuf_check(int fifo_id)
{
    if (fifo_id < 0 || fifo_id >= CVI_MBUF_MAX_NUM)
    {
        CVI_LOGE("param error. %d\n", fifo_id);
        return -1;
    }

    return 0;
}

static int cvi_mbuf_create(int fifo_id, int size)
{
    if ((0 != cvi_mbuf_check(fifo_id)) || (size <= 0))
    {
        CVI_LOGE("param error. %d, %d\n", fifo_id, size);
        return -1;
    }

    if (s_media_fifo[fifo_id] != NULL)
    {
        CVI_LOGE("Mbuf %d has been initialized.\n", fifo_id);
        return 0;
    }

    char name[10] = {0};
    sprintf(name, "stream%d", fifo_id);
    s_media_fifo[fifo_id] = zfifo_init(name, size);
    if (s_media_fifo[fifo_id] == NULL)
    {
        CVI_LOGE("zfifo_init %s failed.\n", name);
        return -1;
    }

    return 0;
}

static int cvi_mbuf_Destroy(int fifo_id)
{
    if (0 != cvi_mbuf_check(fifo_id))
    {
        return -1;
    }

    if (s_media_fifo[fifo_id] == NULL)
    {
        return 0;
    }

    zfifo_uninit(s_media_fifo[fifo_id]);
    s_media_fifo[fifo_id] = NULL;

    return 0;
}

static CVI_MBUF_HANDLE * cvi_mbuf_add_writer(int fifo_id)
{
    if (0 != cvi_mbuf_check(fifo_id))
    {
        return NULL;
    }
    ZFIFO *zfifo = s_media_fifo[fifo_id];
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL)
    {
        CVI_LOGE("zfifo_open stream%d failed.\n", fifo_id);
        return NULL;
    }

    return (CVI_MBUF_HANDLE *)id;
}

static void cvi_mbuf_del_writer(CVI_MBUF_HANDLE * writerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)writerid;
    if (writerid == NULL)
    {
        CVI_LOGE("param error.\n");
        return;
    }

    zfifo_close(zdesc);
}

static ZFIFO_DESC * cvi_mbuf_add_reader(int fifo_id, int bLastTime)
{
    if (0 != cvi_mbuf_check(fifo_id))
    {
        return NULL;
    }

    ZFIFO *zfifo = s_media_fifo[fifo_id];
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL)
    {
        CVI_LOGE("zfifo_open stream%d failed.\n", fifo_id);
        return NULL;
    }
    if(bLastTime)
    {
        zfifo_set_newest_frame(id);
    }
    else
    {
        zfifo_set_oldest_frame(id);
    }

    return (ZFIFO_DESC *)id;
}

static void cvi_mbuf_del_reader(ZFIFO_DESC * readerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)readerid;
    if (readerid == NULL)
    {
        CVI_LOGE("param error.\n");
        return;
    }
    zfifo_close(zdesc);
}

static int cvi_mbuf_write_frame(ZFIFO_DESC * writerid, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;

    if (pFrameInfo == NULL || writerid == NULL)
    {
        CVI_LOGE("param error\n");
        return -1;
    }
    CVI_MBUF_HEADER_T stMFrameHeader;
    ZFIFO_NODE node[CVI_MBUF_MAX];
    node[CVI_MBUF_HEADER].base = &stMFrameHeader;
    node[CVI_MBUF_HEADER].len = sizeof(stMFrameHeader);

    memset(&stMFrameHeader, 0, sizeof(stMFrameHeader));
    stMFrameHeader.magicStart = MEDIA_MAGIC_START;
    stMFrameHeader.magicEnd = MEDIA_MAGIC_END;
    memcpy(&stMFrameHeader.frameParam, &pFrameInfo->frameParam, sizeof(stMFrameHeader.frameParam));

    node[CVI_MBUF_DATA].base = pFrameInfo->frameBuf;
    node[CVI_MBUF_DATA].len = pFrameInfo->frameParam.frameLen;

    iRet = zfifo_writev_plus((ZFIFO_DESC *)writerid, node, CVI_MBUF_MAX, (pFrameInfo->frameParam.frameType == CVI_MEDIA_VFRAME_I));
    if (iRet <= 0)
    {
        CVI_LOGE("zfifo_writev error. %d\n", iRet);
    }

    return iRet;
}

int CVI_MBUF_ReadFrame(CVI_MBUF_HANDLE readerid, int bKeyFrame, CVI_MEDIA_FRAME_INFO_T *pFrameInfo, int iTimeouts)
{
    int iRet = -1;
    if (readerid == NULL || pFrameInfo == NULL)
    {
        CVI_LOGE("param error.\n");
        return -1;
    }
    CVI_MBUF_HEADER_T stMFrameHeader;
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
    ZFIFO_NODE node[CVI_MBUF_MAX];
    node[CVI_MBUF_HEADER].base = &stMFrameHeader;
    node[CVI_MBUF_HEADER].len = sizeof(stMFrameHeader);

    if ((pFrameInfo->frameBuf == NULL) || (pFrameInfo->frameBufLen <= 0))
    {
        CVI_LOGE("param error. buf:%p, len:%d. \n"
            , pFrameInfo->frameBuf, pFrameInfo->frameBufLen);
        return -1;
    }
    else
    {
        node[CVI_MBUF_DATA].base = pFrameInfo->frameBuf;
        node[CVI_MBUF_DATA].len = pFrameInfo->frameBufLen;
    }
    memset(&stMFrameHeader, 0, sizeof(stMFrameHeader));

    if (bKeyFrame)
    {
        iRet = zfifo_readv_flag_plus(zfifo_desc, node, CVI_MBUF_MAX, iTimeouts);
    }
    else
    {
        iRet = zfifo_readv_plus(zfifo_desc, node, CVI_MBUF_MAX, iTimeouts);
    }
    if (iRet < 0)
    {
        CVI_LOGE("zfifo_readv error. %d\n", iRet);
        return iRet;
    }
    else if (iRet == 0)
    {
        return iRet;
    }
    else
    {
        if ((stMFrameHeader.magicStart != MEDIA_MAGIC_START)
            || (stMFrameHeader.magicEnd != MEDIA_MAGIC_END))
        {
            CVI_LOGE("zfifo read magic error.%x, %x\n", stMFrameHeader.magicStart, stMFrameHeader.magicEnd);
            return -1;
        }

        memcpy(&pFrameInfo->frameParam, &stMFrameHeader.frameParam, sizeof(pFrameInfo->frameParam));
    }

    return iRet;
}

int CVI_MBUF_Video_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;
    if (s_hVideoWriterId[mbufId] && (0 == cvi_mbuf_check(mbufId)))
    {
        iRet = cvi_mbuf_write_frame((ZFIFO_DESC *)s_hVideoWriterId[mbufId], pFrameInfo);
    }
    else
    {
        CVI_LOGE("Invalid Write Id:%d\n", mbufId);
    }
    return iRet;
}

int CVI_MBUF_Audio_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;
    if (s_hVideoWriterId[mbufId] && (0 == cvi_mbuf_check(mbufId)))
    {
        iRet = cvi_mbuf_write_frame((ZFIFO_DESC *)s_hVideoWriterId[mbufId], pFrameInfo);
    }
    else
    {
        CVI_LOGE("Invalid Write Id:%d\n", mbufId);
    }
    return iRet;
}

CVI_MBUF_HANDLE* CVI_MBUF_CreateReader(int mbufId, int bLastTime)
{
    if (0 != cvi_mbuf_check(mbufId))
    {
        return NULL;
    }
    return (CVI_MBUF_HANDLE*)cvi_mbuf_add_reader(mbufId, bLastTime);
}

int CVI_MBUF_DestoryReader(CVI_MBUF_HANDLE* readerid)
{
    cvi_mbuf_del_reader((ZFIFO_DESC *)readerid);
    return 0;
}

int CVI_MBUF_Init(int bufid, int bufsize)
{
    int iRet = -1;

    if (0 != cvi_mbuf_check(bufid)) {
        return iRet;
    }

    iRet = cvi_mbuf_create(bufid, bufsize);
    if (iRet) {
        CVI_LOGE("Create vmbuf %d failed.\n", bufid);
        iRet = -1;
        goto endFunc;
    }

    if (s_hVideoWriterId[bufid] == NULL) {
        s_hVideoWriterId[bufid] = cvi_mbuf_add_writer(bufid);
        if (s_hVideoWriterId[bufid] == NULL)
        {
            CVI_LOGE("Create vmbuf write %d failed.\n", bufid);
            iRet = -1;
            goto endFunc;
        }
    } else {
        CVI_LOGE("s_hVideoWriterId %d has been initialized.\n", bufid);
    }

    iRet = 0;
endFunc:
    if (iRet)
    {
        CVI_MBUF_UnInit(bufid);
    }

    return iRet;
}


int CVI_MBUF_UnInit(int bufid)
{
    if (s_hVideoWriterId[bufid])
    {
        cvi_mbuf_del_writer(s_hVideoWriterId[bufid]);
        cvi_mbuf_Destroy(bufid);
        s_hVideoWriterId[bufid] = NULL;
    }
    return 0;
}


