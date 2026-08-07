#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "cvi_venc.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_rtp.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

#define APP_RTP_HEADER_LEN 12
#define APP_RTP_PAYLOAD_TYPE 96
#define APP_RTP_MAX_PACKET 1500
#define APP_RTP_MAX_PAYLOAD (APP_RTP_MAX_PACKET - APP_RTP_HEADER_LEN)
#define APP_RTP_CONTROL_MSG "IDR"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct APP_RTP_CTX_S {
	APP_RTP_PARAM_S stParam;
	int s32VideoFd;
	int s32ControlFd;
	struct sockaddr_in stPeerAddr;
	pthread_t control_thread;
	CVI_BOOL bControlThreadRun;
	CVI_U16 u16Sequence;
	CVI_U16 u16ExpectedSequence;
	CVI_BOOL bHaveSequence;
	CVI_BOOL bWaitIdr;
	CVI_U32 u32SeqGapCount;
	CVI_U32 u32IdrReqCount;
	CVI_U32 u32IdrReqFailCount;
	CVI_U32 u32IdrCtrlCount;
	CVI_U32 u32VdecDropCount;
	CVI_U32 u32TxSendFailCount;
	CVI_U8 *pu8AuBuf;
	CVI_U32 u32AuLen;
	CVI_U64 u64LastIdrRequestMs;
} APP_RTP_CTX_S;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

static APP_RTP_CTX_S g_stRtpCtx = {
	.s32VideoFd = -1,
	.s32ControlFd = -1,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static CVI_U64 app_ipcam_Rtp_MonotonicMs(void)
{
	struct timespec stTime;

	clock_gettime(CLOCK_MONOTONIC, &stTime);
	return (CVI_U64)stTime.tv_sec * 1000 + stTime.tv_nsec / 1000000;
}

/**
 * @brief Reset the incomplete H.264 access-unit reassembly state.
 *
 * @param wait_idr CVI_TRUE makes the receiver reject non-IDR access units.
 * @return None.
 * @note This is used after a sequence gap, AU overflow, or VDEC backpressure so
 *       that decoding resumes only from a complete IDR access unit.
 */
static void app_ipcam_Rtp_ResetAu(CVI_BOOL wait_idr)
{
	g_stRtpCtx.u32AuLen = 0;
	g_stRtpCtx.bWaitIdr = wait_idr;
}

static CVI_BOOL app_ipcam_Rtp_IsIdr(const CVI_U8 *data, CVI_U32 data_len)
{
	CVI_U32 i;

	for (i = 0; i + 4 < data_len; i++) {
		if (data[i] == 0 && data[i + 1] == 0 &&
			((data[i + 2] == 1) || (data[i + 2] == 0 && data[i + 3] == 1))) {
			CVI_U32 nal_offset = data[i + 2] == 1 ? i + 3 : i + 4;

			if (nal_offset < data_len && (data[nal_offset] & 0x1f) == 5)
				return CVI_TRUE;
		}
	}

	return CVI_FALSE;
}

static CVI_S32 app_ipcam_Rtp_Append(const CVI_U8 *data, CVI_U32 data_len)
{
	if (data_len > g_stRtpCtx.stParam.u32MaxAuSize - g_stRtpCtx.u32AuLen) {
		app_ipcam_Rtp_ResetAu(CVI_TRUE);
		return CVI_FAILURE;
	}

	memcpy(g_stRtpCtx.pu8AuBuf + g_stRtpCtx.u32AuLen, data, data_len);
	g_stRtpCtx.u32AuLen += data_len;
	return CVI_SUCCESS;
}

/**
 * @brief Request an IDR access unit from the TX endpoint after RX loss recovery.
 *
 * @param None.
 * @return None.
 * @note The request is rate-limited to one request per 100 ms and is sent only
 *       in RX role. "idr_req" means the RX detected an unusable stream state:
 *       a sequence gap, a VDEC-dropped AU, or a non-IDR AU while resynchronizing.
 *       Debug params: request is the total IDR requests, fail is send failure
 *       count, gap is RTP sequence-gap count, drop is VDEC-drop count, and sent
 *       is the sendto result. The log is emitted on failure and at counts 1, 33,
 *       65, and so on to limit log traffic.
 */
static void app_ipcam_Rtp_RequestIdr(void)
{
	CVI_U64 now = app_ipcam_Rtp_MonotonicMs();
	ssize_t sent;

	if (g_stRtpCtx.stParam.enRole != APP_RTP_ROLE_RX ||
		now - g_stRtpCtx.u64LastIdrRequestMs < 100) {
		return;
	}

	sent = sendto(g_stRtpCtx.s32VideoFd, APP_RTP_CONTROL_MSG,
		strlen(APP_RTP_CONTROL_MSG), MSG_DONTWAIT,
		(const struct sockaddr *)&g_stRtpCtx.stPeerAddr,
		sizeof(g_stRtpCtx.stPeerAddr));
	g_stRtpCtx.u32IdrReqCount++;
	if (sent < 0)
		g_stRtpCtx.u32IdrReqFailCount++;
	if (sent < 0 || (g_stRtpCtx.u32IdrReqCount & 0x1f) == 1) {
		APP_PROF_LOG_PRINT(LEVEL_WARN,
			"idr_req request=%u fail=%u gap=%u drop=%u sent=%zd\n",
			g_stRtpCtx.u32IdrReqCount, g_stRtpCtx.u32IdrReqFailCount,
			g_stRtpCtx.u32SeqGapCount, g_stRtpCtx.u32VdecDropCount, sent);
	}
	g_stRtpCtx.u64LastIdrRequestMs = now;
}

static CVI_S32 app_ipcam_Rtp_SetNonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return CVI_FAILURE;
	return CVI_SUCCESS;
}

/**
 * @brief Configure and report the TX UDP socket send buffer.
 *
 * @param fd RTP video UDP socket descriptor.
 * @return None.
 * @note "tx_sndbuf_set_fail" or "tx_sndbuf_get_fail" means the requested socket
 *       buffer could not be configured or queried. "tx_sndbuf" reports the
 *       requested and kernel-accepted sizes for diagnosing TX queue pressure.
 */
static void app_ipcam_Rtp_LogTxSendBuffer(int fd)
{
	int requested = 512 * 1024;
	int actual = 0;
	socklen_t actual_len = sizeof(actual);

	// What changed: Request a 512 KiB TX UDP send buffer before RTP transmission.
	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &requested, sizeof(requested)) < 0) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR,
			"tx_sndbuf_set_fail requested=%d errno=%d\n", requested, errno);
		return;
	}
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &actual, &actual_len) < 0) {
		APP_PROF_LOG_PRINT(LEVEL_ERROR, "tx_sndbuf_get_fail errno=%d\n", errno);
		return;
	}
	APP_PROF_LOG_PRINT(LEVEL_INFO, "tx_sndbuf requested=%d actual=%d\n", requested, actual);
}

/**
 * @brief Receive RX recovery requests and ask VENC to generate an IDR frame.
 *
 * @param arg Unused thread argument.
 * @return NULL when the control thread exits.
 * @note "idr_ctrl" means the TX received the "IDR" control message from RX and
 *       called CVI_VENC_RequestIDR(). control is the total control requests and
 *       ret is the VENC API result; it is logged on failure and periodically.
 */
static void *app_ipcam_Rtp_ControlThread(void *arg)
{
	char buffer[16];

	(void)arg;
	while (g_stRtpCtx.bControlThreadRun) {
		ssize_t len = recv(g_stRtpCtx.s32ControlFd, buffer, sizeof(buffer), 0);

		if (len >= 3 && !memcmp(buffer, APP_RTP_CONTROL_MSG, 3)) {

			CVI_S32 ret = CVI_VENC_RequestIDR(g_stRtpCtx.stParam.s32VencChn, CVI_TRUE);

			g_stRtpCtx.u32IdrCtrlCount++;

			if (ret != CVI_SUCCESS || (g_stRtpCtx.u32IdrCtrlCount & 0x1f) == 1) {
				APP_PROF_LOG_PRINT(LEVEL_WARN, "idr_ctrl control=%u ret=%#x\n",
					g_stRtpCtx.u32IdrCtrlCount, ret);
			}
			continue;
		}
		if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			break;
		usleep(1000);
	}

	return NULL;
}

/**
 * @brief Get the global RTP runtime parameter storage.
 *
 * @param None.
 * @return Pointer to the mutable RTP parameter structure.
 * @note The parameter parser initializes this storage before app_ipcam_Rtp_Init()
 *       creates sockets or starts the TX control thread.
 */
APP_RTP_PARAM_S *app_ipcam_Rtp_Param_Get(void)
{
	return &g_stRtpCtx.stParam;
}

/**
 * @brief Initialize RTP/UDP transport for the configured TX or RX role.
 *
 * @param None.
 * @return CVI_SUCCESS when disabled or initialized successfully; CVI_FAILURE on
 *         socket, address, bind, memory, or control-thread setup failure.
 * @note Step 1 creates a nonblocking video UDP socket and records the peer.
 *       Step 2 in RX role binds the video port, allocates the AU reassembly
 *       buffer, and waits for an IDR.
 *       Step 3 in TX role binds the control port and starts the IDR-control thread.
 *       Step 4 This separation keeps AU transport on the video socket while recovery requests use the control socket.
 */
int app_ipcam_Rtp_Init(void)
{
	int reuse = 1;
	struct sockaddr_in bind_addr;

	if (!g_stRtpCtx.stParam.bEnable)
		return CVI_SUCCESS;

	g_stRtpCtx.s32VideoFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_stRtpCtx.s32VideoFd < 0)
		goto fail;
	if (app_ipcam_Rtp_SetNonblock(g_stRtpCtx.s32VideoFd) != CVI_SUCCESS)
		goto fail;
	if (g_stRtpCtx.stParam.enRole == APP_RTP_ROLE_TX)
		app_ipcam_Rtp_LogTxSendBuffer(g_stRtpCtx.s32VideoFd);
	memset(&g_stRtpCtx.stPeerAddr, 0, sizeof(g_stRtpCtx.stPeerAddr));
	g_stRtpCtx.stPeerAddr.sin_family = AF_INET;
	g_stRtpCtx.stPeerAddr.sin_port = htons(g_stRtpCtx.stParam.enRole == APP_RTP_ROLE_TX ?
		g_stRtpCtx.stParam.u16VideoPort : g_stRtpCtx.stParam.u16ControlPort);
	if (inet_pton(AF_INET, g_stRtpCtx.stParam.szPeerIp,
		&g_stRtpCtx.stPeerAddr.sin_addr) != 1)
		goto fail;

	if (g_stRtpCtx.stParam.enRole == APP_RTP_ROLE_RX) {
		memset(&bind_addr, 0, sizeof(bind_addr));
		bind_addr.sin_family = AF_INET;
		bind_addr.sin_port = htons(g_stRtpCtx.stParam.u16VideoPort);
		bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		setsockopt(g_stRtpCtx.s32VideoFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
		if (bind(g_stRtpCtx.s32VideoFd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0)
			goto fail;
		g_stRtpCtx.pu8AuBuf = malloc(g_stRtpCtx.stParam.u32MaxAuSize);
		if (g_stRtpCtx.pu8AuBuf == NULL)
			goto fail;
		g_stRtpCtx.bWaitIdr = CVI_TRUE;
	} else {
		g_stRtpCtx.s32ControlFd = socket(AF_INET, SOCK_DGRAM, 0);
		if (g_stRtpCtx.s32ControlFd < 0 ||
			app_ipcam_Rtp_SetNonblock(g_stRtpCtx.s32ControlFd) != CVI_SUCCESS)
			goto fail;
		memset(&bind_addr, 0, sizeof(bind_addr));
		bind_addr.sin_family = AF_INET;
		bind_addr.sin_port = htons(g_stRtpCtx.stParam.u16ControlPort);
		bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(g_stRtpCtx.s32ControlFd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0)
			goto fail;
		g_stRtpCtx.bControlThreadRun = CVI_TRUE;
		if (pthread_create(&g_stRtpCtx.control_thread, NULL,
			app_ipcam_Rtp_ControlThread, NULL) != 0) {
			g_stRtpCtx.bControlThreadRun = CVI_FALSE;
			goto fail;
		}
	}

	return CVI_SUCCESS;

fail:
	app_ipcam_Rtp_DeInit();
	return CVI_FAILURE;
}

/**
 * @brief Stop RTP transport and release all role-specific resources.
 *
 * @param None.
 * @return CVI_SUCCESS after the control thread, sockets, and AU buffer are released.
 * @note The control thread is stopped before closing its socket so it cannot use
 *       a recycled descriptor during shutdown.
 */
int app_ipcam_Rtp_DeInit(void)
{
	if (g_stRtpCtx.bControlThreadRun) {
		g_stRtpCtx.bControlThreadRun = CVI_FALSE;
		pthread_join(g_stRtpCtx.control_thread, NULL);
	}
	if (g_stRtpCtx.s32ControlFd >= 0) {
		close(g_stRtpCtx.s32ControlFd);
		g_stRtpCtx.s32ControlFd = -1;
	}
	if (g_stRtpCtx.s32VideoFd >= 0) {
		close(g_stRtpCtx.s32VideoFd);
		g_stRtpCtx.s32VideoFd = -1;
	}
	free(g_stRtpCtx.pu8AuBuf);
	g_stRtpCtx.pu8AuBuf = NULL;
	return CVI_SUCCESS;
}

/**
 * @brief Detect an Annex-B H.264 start-code length at one byte offset.
 *
 * @param data Encoded H.264 byte stream.
 * @param data_len Total byte count in data.
 * @param offset Byte offset to inspect.
 * @return 3 or 4 for a valid start code; 0 when no start code begins at offset.
 * @note This helper keeps the TX path independent from VENC pack boundaries by
 *       locating each Annex-B NAL unit before RTP packetization.
 */
static CVI_U32 app_ipcam_Rtp_StartCodeLen(const CVI_U8 *data, CVI_U32 data_len,
	CVI_U32 offset)
{
	if (offset + 3 <= data_len && data[offset] == 0 && data[offset + 1] == 0) {
		if (data[offset + 2] == 1)
			return 3;
		if (offset + 4 <= data_len && data[offset + 2] == 0 && data[offset + 3] == 1)
			return 4;
	}
	return 0;
}

/**
 * @brief Send one RTP packet containing a complete NAL payload or FU-A fragment.
 *
 * @param payload RTP payload bytes without the RTP header.
 * @param payload_len Number of payload bytes, limited by APP_RTP_MAX_PAYLOAD.
 * @param marker CVI_TRUE marks the final packet of one access unit.
 * @param timestamp RTP 90 kHz timestamp for the access unit.
 * @return CVI_SUCCESS when sendto accepts the packet; CVI_FAILURE otherwise.
 * @note Step 1 builds the RFC 3550 header and advances the RTP sequence number.
 *       Step 2 submits the datagram with MSG_DONTWAIT. "tx_send_fail" means the
 *       kernel did not accept that packet, commonly because the UDP send queue is
 *       under pressure. Debug params: count is total failures, seq is the already
 *       allocated RTP sequence, ts is the 90 kHz timestamp, payload is bytes,
 *       sent is sendto result, and errno identifies the socket failure. A failed
 *       packet can later appear as "seq_gap" on RX; the log is rate-limited.
 */
static CVI_S32 app_ipcam_Rtp_SendPacket(const CVI_U8 *payload, CVI_U32 payload_len,
	CVI_BOOL marker, CVI_U32 timestamp)
{
	CVI_U8 packet[APP_RTP_MAX_PACKET];
	CVI_U16 sequence;
	ssize_t sent;
	int saved_errno;

	if (payload_len > APP_RTP_MAX_PAYLOAD)
		return CVI_FAILURE;
	sequence = g_stRtpCtx.u16Sequence++;
	packet[0] = 0x80;
	packet[1] = (marker ? 0x80 : 0) | APP_RTP_PAYLOAD_TYPE;
	packet[2] = sequence >> 8;
	packet[3] = sequence & 0xff;
	packet[4] = timestamp >> 24;
	packet[5] = timestamp >> 16;
	packet[6] = timestamp >> 8;
	packet[7] = timestamp & 0xff;
	packet[8] = 0x43;
	packet[9] = 0x56;
	packet[10] = 0x49;
	packet[11] = 0x31;
	memcpy(packet + APP_RTP_HEADER_LEN, payload, payload_len);
	sent = sendto(g_stRtpCtx.s32VideoFd, packet, APP_RTP_HEADER_LEN + payload_len,
		MSG_DONTWAIT, (const struct sockaddr *)&g_stRtpCtx.stPeerAddr,
		sizeof(g_stRtpCtx.stPeerAddr));
	if (sent >= 0)
		return CVI_SUCCESS;

	saved_errno = errno;
	g_stRtpCtx.u32TxSendFailCount++;
	if ((g_stRtpCtx.u32TxSendFailCount & 0x1f) == 1) {
		APP_PROF_LOG_PRINT(LEVEL_WARN,
			"tx_send_fail count=%u seq=%u ts=%u payload=%u sent=%zd errno=%d\n",
			g_stRtpCtx.u32TxSendFailCount, sequence, timestamp, payload_len,
			sent, saved_errno);
	}

	return CVI_FAILURE;
}

/**
 * @brief Packetize one H.264 NAL unit as a single RTP payload or FU-A fragments.
 *
 * @param nal H.264 NAL bytes without the Annex-B start code.
 * @param nal_len Number of NAL bytes.
 * @param marker CVI_TRUE when this is the final NAL of the access unit.
 * @param timestamp RTP 90 kHz timestamp shared by all fragments of the AU.
 * @return CVI_SUCCESS when every generated RTP packet is accepted; CVI_FAILURE
 *         on an invalid or unsent NAL packet.
 * @note Step 1 sends a small NAL as one RTP payload. Step 2 converts an oversized
 *       NAL to H.264 FU-A fragments. Step 3 sets the RTP marker only on the final
 *       fragment of the final NAL, allowing RX to emit exactly one complete AU.
 */
static CVI_S32 app_ipcam_Rtp_SendNal(const CVI_U8 *nal, CVI_U32 nal_len,
	CVI_BOOL marker, CVI_U32 timestamp)
{
	CVI_U32 offset;

	// Step 1: A NAL that fits the UDP payload limit is sent without fragmentation.
	if (nal_len <= APP_RTP_MAX_PAYLOAD)
		return app_ipcam_Rtp_SendPacket(nal, nal_len, marker, timestamp);
	if (nal_len < 2)
		return CVI_FAILURE;

	// Step 2: Split a large NAL into RFC 6184 FU-A payloads.
	offset = 1;
	while (offset < nal_len) {
		CVI_U8 fragment[APP_RTP_MAX_PAYLOAD];
		CVI_U32 fragment_len = nal_len - offset;
		CVI_BOOL start = offset == 1;
		CVI_BOOL end;

		if (fragment_len > APP_RTP_MAX_PAYLOAD - 2)
			fragment_len = APP_RTP_MAX_PAYLOAD - 2;
		end = offset + fragment_len == nal_len;
		fragment[0] = (nal[0] & 0xe0) | 28;
		fragment[1] = (start ? 0x80 : 0) | (end ? 0x40 : 0) | (nal[0] & 0x1f);
		memcpy(fragment + 2, nal + offset, fragment_len);
		// Step 3: Only the final FU-A fragment inherits the AU marker bit.
		if (app_ipcam_Rtp_SendPacket(fragment, fragment_len + 2,
			marker && end, timestamp) != CVI_SUCCESS)
			return CVI_FAILURE;
		offset += fragment_len;
	}

	return CVI_SUCCESS;
}

/**
 * @brief Convert one VENC Annex-B access unit into RTP/UDP packets.
 *
 * @param venc_chn Source VENC channel.
 * @param data Annex-B H.264 access-unit bytes from VENC.
 * @param data_len Number of bytes in data.
 * @param pts VENC presentation timestamp in milliseconds.
 * @return CVI_SUCCESS when RTP is disabled, the channel is unrelated, or all NAL
 *         units are packetized; CVI_FAILURE when packetization or UDP send fails.
 * @note Step 1 scans Annex-B start codes to isolate NAL units. Step 2 packetizes
 *       each NAL independently, using FU-A when required. Step 3 marks the last
 *       NAL so RX knows when its reassembled AU can be submitted to VDEC.
 */
int app_ipcam_Rtp_SendFrame(CVI_S32 venc_chn, const CVI_U8 *data,
	CVI_U32 data_len, CVI_U64 pts)
{
	CVI_U32 offset = 0;
	CVI_U32 start;
	CVI_U32 next;
	CVI_U32 start_code_len;
	CVI_U32 next_start_code_len;
	CVI_U32 timestamp = (CVI_U32)(pts * 90 / 1000);

	if (!g_stRtpCtx.stParam.bEnable || g_stRtpCtx.stParam.enRole != APP_RTP_ROLE_TX ||
		venc_chn != g_stRtpCtx.stParam.s32VencChn || data == NULL)
		return CVI_SUCCESS;

	// Step 1: Locate the next Annex-B NAL start code in the VENC access unit.
	while (offset < data_len) {
		while (offset < data_len && app_ipcam_Rtp_StartCodeLen(data, data_len, offset) == 0)
			offset++;
		if (offset >= data_len)
			break;
		start_code_len = app_ipcam_Rtp_StartCodeLen(data, data_len, offset);
		start = offset + start_code_len;
		next = start;
		while (next < data_len && app_ipcam_Rtp_StartCodeLen(data, data_len, next) == 0)
			next++;
		next_start_code_len = app_ipcam_Rtp_StartCodeLen(data, data_len, next);
		// Step 2: Packetize one NAL; the last NAL carries the AU marker bit.
		if (next > start && app_ipcam_Rtp_SendNal(data + start, next - start,
			next_start_code_len == 0, timestamp) != CVI_SUCCESS)
			return CVI_FAILURE;
		offset = next;
	}

	return CVI_SUCCESS;
}

/**
 * @brief Validate and reassemble one RTP packet into an H.264 access unit.
 *
 * @param packet Received RTP datagram bytes.
 * @param packet_len Number of received bytes.
 * @param data Output pointer to the completed AU buffer.
 * @param data_len Output completed AU length.
 * @param pts Output RTP timestamp when an AU completes.
 * @return CVI_SUCCESS only when this packet completes one decodable AU;
 *         CVI_FAILURE for an incomplete, invalid, lost, or discarded AU.
 * @note Step 1 validates the RTP header and locates the payload. Step 2 checks
 *       sequence continuity. "seq_gap" means received sequence differs from the
 *       expected value, indicating loss, reordering, or a TX-side send failure;
 *       the partial AU is discarded and an IDR is requested. Step 3 rebuilds
 *       single-NAL or FU-A payloads into one Annex-B AU. Step 4 emits the AU only
 *       when the marker arrives and an IDR is available after resynchronization.
 */
static CVI_S32 app_ipcam_Rtp_ProcessPacket(const CVI_U8 *packet, CVI_U32 packet_len,
	CVI_U8 **data, CVI_U32 *data_len, CVI_U64 *pts)
{
	CVI_U32 header_len = APP_RTP_HEADER_LEN;
	CVI_U32 payload_len;
	const CVI_U8 *payload;
	CVI_U16 sequence;
	CVI_BOOL marker;
	CVI_U8 start_code[] = {0, 0, 0, 1};

	// Step 1: Validate the fixed RTP header and locate any optional header fields.
	if (packet_len < APP_RTP_HEADER_LEN || (packet[0] >> 6) != 2 ||
		(packet[1] & 0x7f) != APP_RTP_PAYLOAD_TYPE)
		return CVI_FAILURE;
	header_len += (packet[0] & 0x0f) * 4;
	if (packet[0] & 0x10) {
		CVI_U32 extension_len;

		if (packet_len < header_len + 4)
			return CVI_FAILURE;
		extension_len = (((CVI_U32)packet[header_len + 2] << 8) |
			packet[header_len + 3]) * 4;
		header_len += 4 + extension_len;
	}
	if (packet_len <= header_len)
		return CVI_FAILURE;
	sequence = ((CVI_U16)packet[2] << 8) | packet[3];
	// Step 2: A missing or reordered packet invalidates the current AU.
	if (g_stRtpCtx.bHaveSequence && sequence != g_stRtpCtx.u16ExpectedSequence) {
		g_stRtpCtx.u32SeqGapCount++;
		if ((g_stRtpCtx.u32SeqGapCount & 0x1f) == 1) {
			APP_PROF_LOG_PRINT(LEVEL_WARN,
				"seq_gap expected=%u received=%u gap=%u\n",
				g_stRtpCtx.u16ExpectedSequence, sequence, g_stRtpCtx.u32SeqGapCount);
		}
		app_ipcam_Rtp_ResetAu(CVI_TRUE);
		app_ipcam_Rtp_RequestIdr();
	}
	g_stRtpCtx.u16ExpectedSequence = sequence + 1;
	g_stRtpCtx.bHaveSequence = CVI_TRUE;
	payload = packet + header_len;
	payload_len = packet_len - header_len;
	marker = (packet[1] & 0x80) != 0;

	// Step 3: Rebuild either an RFC 6184 FU-A sequence or a complete single NAL.
	if ((payload[0] & 0x1f) == 28) {
		CVI_BOOL start;
		CVI_BOOL end;
		CVI_U8 nal_header;

		if (payload_len < 2)
			return CVI_FAILURE;
		start = (payload[1] & 0x80) != 0;
		end = (payload[1] & 0x40) != 0;
		nal_header = (payload[0] & 0xe0) | (payload[1] & 0x1f);
		if (start) {
			if (app_ipcam_Rtp_Append(start_code, sizeof(start_code)) != CVI_SUCCESS ||
				app_ipcam_Rtp_Append(&nal_header, 1) != CVI_SUCCESS)
				return CVI_FAILURE;
		} else if (g_stRtpCtx.u32AuLen == 0) {
			return CVI_FAILURE;
		}
		if (app_ipcam_Rtp_Append(payload + 2, payload_len - 2) != CVI_SUCCESS)
			return CVI_FAILURE;
		if (marker && !end)
			return CVI_FAILURE;
	} else if (app_ipcam_Rtp_Append(start_code, sizeof(start_code)) != CVI_SUCCESS ||
		app_ipcam_Rtp_Append(payload, payload_len) != CVI_SUCCESS) {
		return CVI_FAILURE;
	}

	// Step 4: Publish only when the marker closes one complete access unit.
	if (!marker)
		return CVI_FAILURE;
	if (g_stRtpCtx.bWaitIdr && !app_ipcam_Rtp_IsIdr(g_stRtpCtx.pu8AuBuf, g_stRtpCtx.u32AuLen)) {
		app_ipcam_Rtp_ResetAu(CVI_TRUE);
		app_ipcam_Rtp_RequestIdr();
		return CVI_FAILURE;
	}
	*data = g_stRtpCtx.pu8AuBuf;
	*data_len = g_stRtpCtx.u32AuLen;
	*pts = ((CVI_U64)packet[4] << 24) | ((CVI_U64)packet[5] << 16) |
		((CVI_U64)packet[6] << 8) | packet[7];
	g_stRtpCtx.u32AuLen = 0;
	g_stRtpCtx.bWaitIdr = CVI_FALSE;
	return CVI_SUCCESS;
}

/**
 * @brief Drop the current RX access unit after VDEC cannot accept it.
 *
 * @param vdec_chn Destination VDEC channel that rejected the AU.
 * @return CVI_SUCCESS when the configured RX channel is reset; CVI_FAILURE when
 *         the caller is not the configured RX VDEC channel.
 * @note "vdec_drop" is raised by VDEC after nonblocking SendStream fails and its
 *       bounded buffer-full retry is exhausted. Debug params: drop is the total
 *       discarded AUs and gap is the accumulated RTP sequence-gap count. The AU
 *       is discarded, RX waits for an IDR, and an IDR request is sent to TX so a
 *       decoder never continues from an incomplete reference chain.
 */
int app_ipcam_Rtp_DropFrame(CVI_S32 vdec_chn)
{
	if (g_stRtpCtx.stParam.enRole != APP_RTP_ROLE_RX ||
		vdec_chn != g_stRtpCtx.stParam.s32VdecChn)
		return CVI_FAILURE;

	g_stRtpCtx.u32VdecDropCount++;
	if ((g_stRtpCtx.u32VdecDropCount & 0x1f) == 1) {
		APP_PROF_LOG_PRINT(LEVEL_WARN, "vdec_drop drop=%u gap=%u\n",
			g_stRtpCtx.u32VdecDropCount, g_stRtpCtx.u32SeqGapCount);
	}
	app_ipcam_Rtp_ResetAu(CVI_TRUE);
	app_ipcam_Rtp_RequestIdr();
	return CVI_SUCCESS;
}

/**
 * @brief Receive UDP packets until one complete H.264 access unit is available.
 *
 * @param vdec_chn Destination VDEC channel for this RTP stream.
 * @param data Output pointer to the reassembled AU buffer owned by RTP.
 * @param data_len Output AU length.
 * @param pts Output RTP timestamp.
 * @param timeout_ms Maximum wait for the first UDP packet in milliseconds.
 * @return CVI_SUCCESS when a complete AU is ready; CVI_FAILURE on timeout, socket
 *         read failure, invalid input, or when no complete AU can be reassembled.
 * @note Step 1 waits for one UDP datagram. Step 2 repeatedly passes packets to
 *       the AU reassembler until the marker closes an AU. After the first packet,
 *       timeout is set to zero so already queued FU-A fragments are drained
 *       without adding latency to the VDEC path.
 */
int app_ipcam_Rtp_RecvFrame(CVI_S32 vdec_chn, CVI_U8 **data,
	CVI_U32 *data_len, CVI_U64 *pts, CVI_S32 timeout_ms)
{
	fd_set read_fds;
	struct timeval timeout;
	CVI_U8 packet[APP_RTP_MAX_PACKET];

	if (data == NULL || data_len == NULL || pts == NULL ||
		g_stRtpCtx.stParam.enRole != APP_RTP_ROLE_RX ||
		vdec_chn != g_stRtpCtx.stParam.s32VdecChn)
		return CVI_FAILURE;

	// Step 1: Wait for the first datagram, then drain queued fragments immediately.
	while (1) {
		ssize_t packet_len;
		int ret;

		FD_ZERO(&read_fds);
		FD_SET(g_stRtpCtx.s32VideoFd, &read_fds);
		timeout.tv_sec = timeout_ms / 1000;
		timeout.tv_usec = (timeout_ms % 1000) * 1000;
		ret = select(g_stRtpCtx.s32VideoFd + 1, &read_fds, NULL, NULL, &timeout);
		if (ret <= 0)
			return CVI_FAILURE;
		packet_len = recv(g_stRtpCtx.s32VideoFd, packet, sizeof(packet), 0);
		if (packet_len <= 0)
			return CVI_FAILURE;
		// Step 2: Return only when the packet sequence completes one access unit.
		if (app_ipcam_Rtp_ProcessPacket(packet, packet_len, data, data_len, pts) == CVI_SUCCESS)
			return CVI_SUCCESS;
		timeout_ms = 0;
	}
}
