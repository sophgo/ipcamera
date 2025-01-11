#include <errno.h>
#include <fcntl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "cvi_isp.h"
#include "cvi_vpss.h"
#include "cvi_ae.h"

#include "app_ipcam_comm.h"
#include "cvi_mbuf.h"
#include "cvi_uvc_gadget.h"

#ifndef CLEAR
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(val, min, max)                   \
    ({                                         \
        typeof(val) __val = (val);             \
        typeof(min) __min = (min);             \
        typeof(max) __max = (max);             \
        (void)(&__val == &__min);              \
        (void)(&__val == &__max);              \
        __val = __val < __min ? __min : __val; \
        __val > __max ? __max : __val;         \
    })
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))
#endif

#ifndef pixfmtstr
#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff
#endif
/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */

#define UVC_STREAM_VENC_CHN 0

#define PU_ISP_MIN_VAL 0
#define PU_ISP_MAX_VAL 100
#define PU_ISP_STEP_SIZE 1
#define PU_ISP_DEFAULT_VAL 50

#define PU_FLICKER_MIN_VAL 50
#define PU_FLICKER_MAX_VAL 60

#define MJPG_PAYLOAD_SIZE 1024
#define H264_PAYLOAD_SIZE 512
#define H265_PAYLOAD_SIZE 512

#define UVC_INTF_CONTROL 0
#define UVC_INTF_STREAMING 1

#define UVC_EVENT_FIRST (V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_CONNECT (V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_DISCONNECT (V4L2_EVENT_PRIVATE_START + 1)
#define UVC_EVENT_STREAMON (V4L2_EVENT_PRIVATE_START + 2)
#define UVC_EVENT_STREAMOFF (V4L2_EVENT_PRIVATE_START + 3)
#define UVC_EVENT_SETUP (V4L2_EVENT_PRIVATE_START + 4)
#define UVC_EVENT_DATA (V4L2_EVENT_PRIVATE_START + 5)
#define UVC_EVENT_LAST (V4L2_EVENT_PRIVATE_START + 5)

#define UVCIOC_SEND_RESPONSE _IOW('U', 1, struct uvc_request_data)


/* ---------------------------------------------------------------------------
 * Generic stuff
 */

struct uvc_request_data {
    uint32_t length;
    uint8_t data[60];
};

struct uvc_event {
    union {
        struct usb_ctrlrequest req;
        struct uvc_request_data data;
    };
};


/* Buffer representing one video frame */
struct buffer {
    struct v4l2_buffer buf;
    void *start;
    size_t length;
};

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */

struct uvc_frame_info {
    uint32_t width;
    uint32_t height;
    uint32_t intervals[8];
};

struct uvc_format_info {
    uint32_t fcc;
    const struct uvc_frame_info *frames;
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
    {
        2560,
        1440,
        {400000, 0},
    },
    {
        1920,
        1080,
        {400000, 0},
    },
    {
        1280,
        720,
        {400000, 0},
    },
    {
        640,
        360,
        {400000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
    {
        2560,
        1440,
        {400000, 0},
    },
    {
        1920,
        1080,
        {400000, 0},
    },
    {
        1280,
        720,
        {400000, 0},
    },
    {
        640,
        360,
        {400000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h265[] = {
    {
        2560,
        1440,
        {400000, 0},
    },
    {
        1920,
        1080,
        {400000, 0},
    },
    {
        1280,
        720,
        {400000, 0},
    },
    {
        640,
        360,
        {400000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

// TODO, move into parameters
static const struct uvc_format_info uvc_formats[] = {
    {V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg},
    {V4L2_PIX_FMT_H264, uvc_frames_h264},
    {V4L2_PIX_FMT_HEVC, uvc_frames_h265},
};

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a UVC based video output device */
typedef struct tagUVC_DEVICE_CTX_S {
    /* uvc device specific */
    int uvc_fd;
    int is_streaming;

    /* uvc control request specific */

    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;
    int control;
    struct uvc_request_data request_error_code;
    uint8_t cur_cs;
    uint32_t cur_value;

    /* uvc buffer specific */
    struct buffer *dummy_buf;
    uint32_t nbufs;
    uint32_t fcc;
    uint32_t width;
    uint32_t height;
    uint32_t fps;

    uint32_t bulk;
    uint8_t color;
    uint32_t imgsize;

    /* USB speed specific */
    int mult;
    int burst;
    int maxpkt;

    CVI_MBUF_HANDLE readerid;
    CVI_MEDIA_FRAME_INFO_T stReadFrameInfo;
} UVC_DEVICE_CTX_S;
static UVC_DEVICE_CTX_S s_stUVCDevCtx;

static int uvc_video_qbuf(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    uint32_t i = 0;
    for (i = 0; i < dev->nbufs; ++i)
    {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.m.userptr = (unsigned long)dev->dummy_buf[i].start;
        buf.length = dev->dummy_buf[i].length;
        buf.index = i;
        buf.bytesused = dev->dummy_buf[i].length;

        int ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf);
        if (ret < 0)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: VIDIOC_QBUF failed : %s (%d).\n",
                strerror(errno), errno);
            return ret;
        }
    }

    return 0;
}

static int uvc_video_reqbufs(UVC_DEVICE_CTX_S *dev, int nbufs)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    int ret = 0;
    struct v4l2_requestbuffers rb;

    CLEAR(rb);
    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0)
    {
        if (ret == -EINVAL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: does not support user pointer i/o\n");
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: VIDIOC_REQBUFS11 error %s (%d).\n", strerror(errno), errno);
        }
        return ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "UVC: %u buffers allocated.\n", rb.count);
    if (0 == rb.count)
        return 0;

    return 0;
}

static int uvc_video_set_format(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    fmt.fmt.pix.sizeimage = dev->imgsize;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "imgsize: %u format: %c%c%c%c wh:%ux%u\n",
        dev->imgsize, pixfmtstr(dev->fcc), dev->width, dev->height);

    int ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: Unable to set format %s (%d).\n",
            strerror(errno), errno);
        return ret;
    }

    return 0;
}

static int uvc_video_set_stream(UVC_DEVICE_CTX_S *dev, int enable)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    int ret = 0;
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    if (0 == enable)
    {
        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: VIDIOC_STREAMOFF failed: %s (%d).\n",
                strerror(errno), errno);
            return ret;
        }
        if (dev->readerid)
        {
            app_ipcam_Mbuf_DestoryReader(dev->readerid);
            dev->readerid = NULL;
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "UVC: Stopping video stream.\n");
    }
    else
    {
        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
        if (ret < 0)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: Unable to start streaming %s (%d).\n",
                strerror(errno), errno);
            return ret;
        }
        dev->readerid = app_ipcam_Mbuf_CreateReader(UVC_STREAM_VENC_CHN, 1);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "UVC: Starting video stream.\n");
    }
    return 0;
}


/* ---------------------------------------------------------------------------
 * UVC streaming related
 */

static void uvc_video_get_stream(UVC_DEVICE_CTX_S *dev, struct v4l2_buffer *buf)
{
    if ((NULL == dev) || (NULL == buf))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    switch (dev->fcc)
    {
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_HEVC:
retry:
            dev->stReadFrameInfo.frameParam.frameLen = CVI_MBUF_STREAM_MAX_SIZE;
            if (app_ipcam_Mbuf_ReadFrame(dev->readerid, 0, &dev->stReadFrameInfo, 100) > 0)
            {
                if (dev->stReadFrameInfo.frameParam.frameType != CVI_MEDIA_AFRAME_A)
                {
                    memcpy(dev->dummy_buf[buf->index].start, dev->stReadFrameInfo.frameBuf, dev->stReadFrameInfo.frameParam.frameLen);
                    buf->length = buf->bytesused = dev->stReadFrameInfo.frameParam.frameLen;
                }
                else
                {
                    //return stream otherwise blur
                    goto retry;
                }
            }
            else
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Mbuf Read Failed!\n");
            }
            break;
        default:
        {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "unsupport fmt:%d\n", dev->fcc);
            break;
        }
    }
}

static int uvc_video_process(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    int ret  = 0;
    struct v4l2_buffer ubuf;

    /* Prepare a v4l2 buffer to be dequeued from UVC domain. */
    CLEAR(ubuf);
    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ubuf.memory = V4L2_MEMORY_USERPTR;

    /* UVC stanalone setup. */
    ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
    if (ret < 0)
        return 0;

    uvc_video_get_stream(dev, &ubuf);

    ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &ubuf);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: Unable to queue buffer: %s (%d).\n",
            strerror(errno), errno);
        return ret;
    }
    return 0;
}

static void uvc_events_process_control(UVC_DEVICE_CTX_S *dev, uint8_t req, uint8_t cs, uint8_t entity_id,
                                       uint8_t len, struct uvc_request_data *resp)
{
    if ((NULL == dev) || (NULL == resp))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }

    int cur_value = 0;
    switch (entity_id)
    {
        case UVC_VC_DESCRIPTOR_UNDEFINED:
            switch (cs)
            {
                case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
                    /* Send the request error code last prepared. */
                    resp->data[0] = dev->request_error_code.data[0];
                    resp->length = dev->request_error_code.length;
                    break;

                default:
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare an error code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

        case UVC_VC_HEADER:
            switch (cs)
            {
                /*
                 * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
                 * terminal, as our bmControls[0] = 2 for CT. Also we
                 * support only auto exposure.
                 */
                case UVC_CT_AE_MODE_CONTROL:
                    switch (req)
                    {
                        case UVC_SET_CUR:
                            /* Incase of auto exposure, attempts to
                             * programmatically set the auto-adjusted
                             * controls are ignored.
                             */
                            resp->data[0] = 0x01;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_INFO:
                            /*
                             * TODO: We support Set and Get requests, but
                             * don't support async updates on an video
                             * status (interrupt) endpoint as of
                             * now.
                             */
                            resp->data[0] = 0x03;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_CUR:
                        case UVC_GET_DEF:
                        case UVC_GET_RES:
                            /* Auto Mode â€“ auto Exposure Time, auto Iris. */
                            resp->data[0] = 0x02;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        default:
                            /*
                             * We don't support this control, so STALL the
                             * control ep.
                             */
                            resp->length = -EL2HLT;
                            /*
                             * For every unsupported control request
                             * set the request error code to appropriate
                             * value.
                             */
                            dev->request_error_code.data[0] = 0x07;
                            dev->request_error_code.length = 1;
                            break;
                    }
                    break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

        case UVC_VC_INPUT_TERMINAL:
            dev->cur_cs = cs;
            switch (cs)
            {
                    case UVC_PU_BRIGHTNESS_CONTROL:
                    case UVC_PU_CONTRAST_CONTROL:
                    case UVC_PU_HUE_CONTROL:
                    case UVC_PU_SATURATION_CONTROL:
                    case UVC_PU_SHARPNESS_CONTROL:
                    case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
                        switch (req)
                        {
                            case UVC_SET_CUR:
                                resp->data[0] = 0x0;
                                resp->length = len;
                                /*
                                 * For every successfully handled control
                                 * request set the request error code to no
                                 * error
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_MIN:
                                resp->data[0] = (cs == UVC_PU_POWER_LINE_FREQUENCY_CONTROL) ? PU_FLICKER_MIN_VAL : PU_ISP_MIN_VAL;
                                resp->length = 2;
                                /*
                                 * For every successfully handled control
                                 * request set the request error code to no
                                 * error
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_MAX:
                                resp->data[0] = (cs == UVC_PU_POWER_LINE_FREQUENCY_CONTROL) ? PU_FLICKER_MAX_VAL : PU_ISP_MAX_VAL;
                                resp->length = 2;
                                /*
                                 * For every successfully handled control
                                 * request set the request error code to no
                                 * error
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_CUR:
                                resp->length = 2;
                                if (cs == UVC_PU_BRIGHTNESS_CONTROL)
                                {
                                    CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, &cur_value);
                                }
                                else if (cs == UVC_PU_CONTRAST_CONTROL)
                                {
                                    CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_CONTRAST, &cur_value);
                                }
                                else if (cs == UVC_PU_HUE_CONTROL)
                                {
                                    CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_HUE, &cur_value);
                                }
                                else if (cs == UVC_PU_SATURATION_CONTROL)
                                {
                                    CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_SATURATION, &cur_value);
                                }
                                else if (cs == UVC_PU_SHARPNESS_CONTROL)
                                {
                                    ISP_SHARPEN_ATTR_S stDRCAttr;
                                    CVI_ISP_GetSharpenAttr(0, &stDRCAttr);
                                    if (stDRCAttr.Enable && (stDRCAttr.enOpType == OP_TYPE_MANUAL)) {
                                        cur_value = stDRCAttr.stManual.GlobalGain;
                                    }
                                }
                                else if (cs == UVC_PU_POWER_LINE_FREQUENCY_CONTROL)
                                {
                                    CVI_BOOL bEnable = 0;
                                    CVI_U8 value = 0;
                                    CVI_ISP_GetAntiFlicker(0, &bEnable, &value);
                                    if (bEnable)
                                    {
                                        cur_value = value;
                                    }
                                }
                                memcpy(&resp->data[0], &cur_value, resp->length);
                                /*
                                 * For every successfully handled control
                                 * request set the request error code to no
                                 * error
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_INFO:
                                /*
                                 * We support Set and Get requests and don't
                                 * support async updates on an interrupt endpt
                                 */
                                resp->data[0] = 0x03;
                                resp->length = 1;
                                /*
                                 * For every successfully handled control
                                 * request, set the request error code to no
                                 * error.
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_DEF:
                                
                                resp->data[0] = PU_ISP_DEFAULT_VAL;
                                resp->length = 2;
                                /*
                                 * For every successfully handled control
                                 * request, set the request error code to no
                                 * error.
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            case UVC_GET_RES:
                                resp->data[0] = PU_ISP_STEP_SIZE;
                                resp->length = 2;
                                /*
                                 * For every successfully handled control
                                 * request, set the request error code to no
                                 * error.
                                 */
                                dev->request_error_code.data[0] = 0x00;
                                dev->request_error_code.length = 1;
                                break;
                            default:
                                /*
                                 * We don't support this control, so STALL the
                                 * default control ep.
                                 */
                                resp->length = -EL2HLT;
                                /*
                                 * For every unsupported control request
                                 * set the request error code to appropriate
                                 * code.
                                 */
                                dev->request_error_code.data[0] = 0x07;
                                dev->request_error_code.length = 1;
                                break;
                        }
                        break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }

            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
    }

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "control request (req %02x cs %02x)\n", req, cs);
}

/* ---------------------------------------------------------------------------
 * UVC Request processing
 */

static void uvc_events_process_steaming_def(UVC_DEVICE_CTX_S *dev, struct uvc_streaming_control *ctrl, int iframe,
                                       int iformat)
{
    if ((NULL == ctrl) || (NULL == dev))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    uint32_t nframes = 0;

    if (iformat < 0)
        iformat = ARRAY_SIZE(uvc_formats) + iformat;
    if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats))
        return;
    format = &uvc_formats[iformat];

    while (format->frames[nframes].width != 0)
        ++nframes;

    if (iframe < 0)
        iframe = nframes + iframe;
    if (iframe < 0 || iframe >= (int)nframes)
        return;
    frame = &format->frames[iframe];

    memset(ctrl, 0, sizeof *ctrl);
    ctrl->bmHint = 1;
    ctrl->bFormatIndex = iformat + 1;
    ctrl->bFrameIndex = iframe + 1;
    ctrl->dwFrameInterval = frame->intervals[0];
    ctrl->dwMaxVideoFrameSize = CVI_MBUF_STREAM_MAX_SIZE;
    ctrl->dwMaxPayloadTransferSize = (dev->maxpkt) * (dev->mult + 1) * (dev->burst + 1);
    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;
}

static void uvc_events_process_streaming(UVC_DEVICE_CTX_S *dev, uint8_t req, uint8_t cs,
                                         struct uvc_request_data *resp)
{
    if ((NULL == dev) || (NULL == resp))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    struct uvc_streaming_control *ctrl;

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "streaming request (req %02x cs %02x)\n", req, cs);

    if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
        return;

    ctrl = (struct uvc_streaming_control *)&resp->data;
    resp->length = sizeof *ctrl;

    switch (req)
    {
        case UVC_SET_CUR:
            dev->control = cs;
            resp->length = 34;
            break;

        case UVC_GET_CUR:
            if (cs == UVC_VS_PROBE_CONTROL)
                memcpy(ctrl, &dev->probe, sizeof *ctrl);
            else
                memcpy(ctrl, &dev->commit, sizeof *ctrl);
            break;

        case UVC_GET_MIN:
        case UVC_GET_MAX:
        case UVC_GET_DEF:
            uvc_events_process_steaming_def(dev, ctrl, req == UVC_GET_MAX ? -1 : 0, req == UVC_GET_MAX ? -1 : 0);
            break;

        case UVC_GET_RES:
            CLEAR(ctrl);
            break;

        case UVC_GET_LEN:
            resp->data[0] = 0x00;
            resp->data[1] = 0x22;
            resp->length = 2;
            break;

        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
    }
}

static void uvc_events_process_class(UVC_DEVICE_CTX_S *dev, struct usb_ctrlrequest *ctrl,
                                     struct uvc_request_data *resp)
{
    if ((NULL == ctrl) || (NULL == dev) || (NULL == resp))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return;

    switch (ctrl->wIndex & 0xff)
    {
        case UVC_INTF_CONTROL:
            uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, ctrl->wIndex >> 8,
                                       ctrl->wLength, resp);
            break;

        case UVC_INTF_STREAMING:
            uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
            break;

        default:
            break;
    }
}
static void uvc_events_process_setup(UVC_DEVICE_CTX_S *dev, struct usb_ctrlrequest *ctrl,
                                     struct uvc_request_data *resp)
{
    if ((NULL == ctrl) || (NULL == dev) || (NULL == resp))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    dev->control = 0;

    switch (ctrl->bRequestType & USB_TYPE_MASK)
    {
        case USB_TYPE_CLASS:
            uvc_events_process_class(dev, ctrl, resp);
            break;

        default:
            break;
    }
}

static int uvc_events_process_control_data(UVC_DEVICE_CTX_S *dev, uint8_t cs, struct uvc_request_data *data)
{
    if ((NULL == dev) || (NULL == data))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }

    int ret = 0;
    int vpssGrp = 0;
    int viPipe = 0;
    memcpy(&dev->cur_value, data->data, data->length);
    switch (cs)
    {
        case UVC_PU_BRIGHTNESS_CONTROL:
            ret = CVI_VPSS_SetGrpProcAmp(vpssGrp, PROC_AMP_BRIGHTNESS, dev->cur_value);
            break;

        case UVC_PU_CONTRAST_CONTROL:
            ret = CVI_VPSS_SetGrpProcAmp(vpssGrp, PROC_AMP_CONTRAST, dev->cur_value);
            break;
        case UVC_PU_HUE_CONTROL:
            ret = CVI_VPSS_SetGrpProcAmp(vpssGrp, PROC_AMP_HUE, dev->cur_value);
            break;
        case UVC_PU_SATURATION_CONTROL:
            ret = CVI_VPSS_SetGrpProcAmp(vpssGrp, PROC_AMP_SATURATION, dev->cur_value);
            break;
        case UVC_PU_SHARPNESS_CONTROL:
            memcpy(&dev->cur_value, data->data, data->length);
            ISP_SHARPEN_ATTR_S stDRCAttr;
            CVI_ISP_GetSharpenAttr(viPipe, &stDRCAttr);
            stDRCAttr.Enable = CVI_TRUE;
            stDRCAttr.enOpType = OP_TYPE_MANUAL;
            stDRCAttr.stManual.GlobalGain = (CVI_U8)dev->cur_value;
            ret = CVI_ISP_SetSharpenAttr(viPipe, &stDRCAttr);
            break;
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            ret = CVI_ISP_SetAntiFlicker(viPipe, 1, dev->cur_value);
            break;

        default:
            break;
    }

    if (ret != 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "set %d NG!\n", cs);
    }

    return ret;
}

static int uvc_events_process_data(UVC_DEVICE_CTX_S *dev, struct uvc_request_data *data)
{
    if ((NULL == data) || (NULL == dev))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    struct uvc_streaming_control *target;
    struct uvc_streaming_control *ctrl;
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    const uint32_t *interval;
    uint32_t iformat, iframe;
    uint32_t nframes;

    switch (dev->control)
    {
        case UVC_VS_PROBE_CONTROL:
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "setting probe control, length = %d\n", data->length);
            target = &dev->probe;
            break;

        case UVC_VS_COMMIT_CONTROL:
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "setting commit control, length = %d\n", data->length);
            target = &dev->commit;
            break;

        default:
            return uvc_events_process_control_data(dev, dev->cur_cs, data);;
    }

    ctrl = (struct uvc_streaming_control *)&data->data;
    iformat = clamp((uint32_t)ctrl->bFormatIndex, 1U, (uint32_t)ARRAY_SIZE(uvc_formats));
    format = &uvc_formats[iformat - 1];

    nframes = 0;
    while (format->frames[nframes].width != 0)
        ++nframes;

    iframe = clamp((uint32_t)ctrl->bFrameIndex, 1U, nframes);
    frame = &format->frames[iframe - 1];
    interval = frame->intervals;

    while (interval[0] < ctrl->dwFrameInterval && interval[1])
        ++interval;

    dev->width = frame->width;
    dev->height = frame->height;

    target->bFormatIndex = iformat;
    target->bFrameIndex = iframe;
    switch (format->fcc)
    {
        case V4L2_PIX_FMT_MJPEG:
            dev->imgsize = MJPG_PAYLOAD_SIZE;
            break;
        case V4L2_PIX_FMT_H264:
            dev->imgsize = H264_PAYLOAD_SIZE;
            break;
        case V4L2_PIX_FMT_HEVC:
            dev->imgsize = H265_PAYLOAD_SIZE;
            break;
        default:
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupport fmt:%d\n", format->fcc);
            return 0;
        }
    }
    target->dwMaxVideoFrameSize = CVI_MBUF_STREAM_MAX_SIZE;
    target->dwFrameInterval = *interval;

    if (dev->control == UVC_VS_COMMIT_CONTROL)
    {
        dev->fcc = format->fcc;
        dev->width = frame->width;
        dev->height = frame->height;
        dev->fps = 10000000 / target->dwFrameInterval;
        uvc_video_set_format(dev);
    }

    return 0;
}

static int uvc_events_process_streamon(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid\n");
        return -1;
    }
    int ret = uvc_video_reqbufs(dev, dev->nbufs);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "after uvc_video_reqbufs\n");
        return ret;
    }

    /* Queue buffers to UVC domain and start streaming. */
    ret = uvc_video_qbuf(dev);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "after uvc_video_qbuf\n");
        return ret;
    }

    uvc_video_set_stream(dev, 1);

    dev->is_streaming = 1;

    return 0;
}

static void uvc_events_process(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid\n");
        return ;
    }
    struct v4l2_event v4l2_event;
    struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
    struct uvc_request_data resp;

    int ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "VIDIOC_DQEVENT failed: %s (%d)\n",
            strerror(errno), errno);
        return;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type)
    {
        case UVC_EVENT_CONNECT:
            APP_PROF_LOG_PRINT(LEVEL_INFO, "UVC_EVENT_CONNECT\n");
            return;

        case UVC_EVENT_DISCONNECT:
            APP_PROF_LOG_PRINT(LEVEL_INFO, "UVC_EVENT_DISCONNECT\n");
            return;

        case UVC_EVENT_SETUP:
            uvc_events_process_setup(dev, &uvc_event->req, &resp);
            break;

        case UVC_EVENT_DATA:
            ret = uvc_events_process_data(dev, &uvc_event->data);
            if (ret < 0)
                break;
            return;

        case UVC_EVENT_STREAMON:
            uvc_events_process_streamon(dev);
            return;

        case UVC_EVENT_STREAMOFF:
            /* ... and now UVC streaming.. */
            if (dev->is_streaming)
            {
                uvc_video_set_stream(dev, 0);
                uvc_video_reqbufs(dev, 0);
                dev->is_streaming = 0;
            }

            return;
    }

    ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVCIOC_S_EVENT failed: %s (%d)\n",
            strerror(errno), errno);
        return;
    }
}

static void uvc_events_init(UVC_DEVICE_CTX_S *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid\n");
        return ;
    }
    uvc_events_process_steaming_def(dev, &dev->probe, 0, 0);
    uvc_events_process_steaming_def(dev, &dev->commit, 0, 0);

    struct v4l2_event_subscription sub;
    memset(&sub, 0, sizeof sub);
    sub.type = UVC_EVENT_SETUP;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_CONNECT;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DISCONNECT;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

static void uvc_close(UVC_DEVICE_CTX_S *dev)
{
    close(dev->uvc_fd);
    dev->uvc_fd = -1;
}

static int32_t uvc_device_open(const char *devname, UVC_DEVICE_CTX_S *dev)
{
    if ((NULL == devname) || (NULL == dev))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return -1;
    }
    int ret = -EINVAL;
    struct v4l2_capability cap;
    CLEAR(cap);

    int fd = open(devname, O_RDWR | O_NONBLOCK);
    if (fd == -1)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: device open failed: %s (%d).\n",
            strerror(errno), errno);
        return ret;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: unable to query uvc device: %s (%d)\n",
            strerror(errno), errno);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: %s is no video output device\n", devname);
        goto err;
    }

    dev->uvc_fd = fd;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "uvc device is %s on bus %s file descriptor = %d\n",
        cap.card, cap.bus_info, dev->uvc_fd);

    return 0;

err:
    if (fd)
    {
        close(fd);
    }
    return ret;
}

int32_t UVC_GADGET_DeviceCheck(void)
{
    fd_set fdsu;

    FD_ZERO(&fdsu);

    /* We want both setup and data events on UVC interface.. */
    FD_SET(s_stUVCDevCtx.uvc_fd, &fdsu);

    fd_set efds = fdsu;
    fd_set dfds = fdsu;

    int ret = select(s_stUVCDevCtx.uvc_fd + 1, NULL, &dfds, &efds, NULL);
    if (-1 == ret)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "select error %s, %d\n", strerror(errno), errno);
        if (EINTR == errno)
            return ret;
    }

    if (0 == ret)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "select timeout\n");
        return ret;
    }

    if (FD_ISSET(s_stUVCDevCtx.uvc_fd, &efds))
    {
        uvc_events_process(&s_stUVCDevCtx);
    }
    if (FD_ISSET(s_stUVCDevCtx.uvc_fd, &dfds))
    {
        if (0 == s_stUVCDevCtx.is_streaming)
        {
            usleep(1000);
        }
        else
        {
            uvc_video_process(&s_stUVCDevCtx);
        }
    }

    return ret;
}

int32_t UVC_GADGET_Init(void)
{
    int ret = 0;
    s_stUVCDevCtx.uvc_fd = -1;
    s_stUVCDevCtx.nbufs = 3;
    /* USB speed related params */
    s_stUVCDevCtx.maxpkt = 1024;
    s_stUVCDevCtx.mult = 2;
    s_stUVCDevCtx.burst = 0;
    /* Allocate buffers to hold dummy data pattern. */
    s_stUVCDevCtx.dummy_buf = calloc(s_stUVCDevCtx.nbufs, sizeof s_stUVCDevCtx.dummy_buf[0]);
    if (!s_stUVCDevCtx.dummy_buf)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (uint32_t i = 0; i < s_stUVCDevCtx.nbufs; ++i)
    {
        s_stUVCDevCtx.dummy_buf[i].length = CVI_MBUF_STREAM_MAX_SIZE;
        s_stUVCDevCtx.dummy_buf[i].start = malloc(CVI_MBUF_STREAM_MAX_SIZE);
        if (!s_stUVCDevCtx.dummy_buf[i].start)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "UVC: Out of memory\n");
            ret = -ENOMEM;
            goto err;
        }
    }

    memset(&s_stUVCDevCtx.stReadFrameInfo.frameParam, 0, sizeof(s_stUVCDevCtx.stReadFrameInfo.frameParam));
    s_stUVCDevCtx.stReadFrameInfo.frameBuf = malloc(CVI_MBUF_STREAM_MAX_SIZE);
    if (NULL == s_stUVCDevCtx.stReadFrameInfo.frameBuf)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "frameBuf malloc fail\n");
        ret = -ENOMEM;
        goto err;
    }
    s_stUVCDevCtx.stReadFrameInfo.frameParam.frameLen = CVI_MBUF_STREAM_MAX_SIZE;

    return 0;
err:
    UVC_GADGET_UnInit((void *)&s_stUVCDevCtx);
    return ret;
}

int32_t UVC_GADGET_UnInit(void *dev)
{
    if (NULL == dev)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid\n");
    }
    UVC_DEVICE_CTX_S *pstUVCDevCtx = (UVC_DEVICE_CTX_S *)dev;
    uint32_t i = 0;

    for (i = 0; i < pstUVCDevCtx->nbufs; ++i)
    {
        free(pstUVCDevCtx->dummy_buf[i].start);
        pstUVCDevCtx->dummy_buf[i].start = NULL;
    }

    free(pstUVCDevCtx->dummy_buf);
    pstUVCDevCtx->dummy_buf = NULL;

    if (pstUVCDevCtx->stReadFrameInfo.frameBuf)
    {
        free(pstUVCDevCtx->stReadFrameInfo.frameBuf);
        pstUVCDevCtx->stReadFrameInfo.frameBuf = NULL;
    }

    return 0;
}

int32_t UVC_GADGET_DeviceOpen(const char *pDevPath)
{
    if (NULL == pDevPath)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid\n");
    }
    if (s_stUVCDevCtx.uvc_fd == -1)
    {
        if (uvc_device_open(pDevPath, &s_stUVCDevCtx))
        {
            return -1;
        }
    }

    uvc_events_init(&s_stUVCDevCtx);

    return 0;
}

int32_t UVC_GADGET_DeviceClose(void)
{
    if (s_stUVCDevCtx.is_streaming)
    {
        uvc_video_set_stream(&s_stUVCDevCtx, 0);
        uvc_video_reqbufs(&s_stUVCDevCtx, 0);
        s_stUVCDevCtx.is_streaming = 0;
    }

    uvc_close(&s_stUVCDevCtx);
    UVC_GADGET_UnInit((void *)&s_stUVCDevCtx);

    return 0;
}
