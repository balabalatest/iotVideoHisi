
#include <stdint.h>
#include "iotRtmp.h"


static RTMP* rtmp = NULL;

int iotRtmp_Connect(const char *url, int timeout)
{
  if (!url) return FALSE;

//  RTMP_LogSetLevel(RTMP_LOGDEBUG);

  rtmp = RTMP_Alloc();
  if (!rtmp) return FALSE;

  RTMP_Init(rtmp);

  if (timeout > 0)
    rtmp->Link.timeout = timeout;
  rtmp->Link.lFlags |= RTMP_LF_LIVE;

  if (!RTMP_SetupURL(rtmp, (char *)url))
  {
      printf("SetupURL Failed!\n");
      goto exit;
  }

  RTMP_EnableWrite(rtmp);
  if (!RTMP_Connect(rtmp, NULL)){
    printf("Connect Failed\n");
    goto exit;
  }

  if (!RTMP_ConnectStream(rtmp, 0)){
    printf("ConnectStream Failed\n");
    RTMP_Close(rtmp);
    goto exit;
  }

  return TRUE;

exit:
  RTMP_Free(rtmp);
  rtmp = NULL;
  return FALSE;
}

void iotRtmp_Disconnect(void)
{
  if(rtmp) {
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    rtmp = NULL;
  }
}

int iotRtmp_SendPacket(unsigned char *buf, int bufLen, int type, uint32_t ts)
{
  int rc = FALSE;

  RTMPPacket sPacket, *packet = &sPacket;
  RTMPPacket_Reset(packet);
  if (!RTMPPacket_Alloc(packet, bufLen))
    return rc;

  packet->m_packetType      = type;
  packet->m_nBodySize       = bufLen;
  packet->m_nTimeStamp      = ts;
  packet->m_nChannel        = 0x04;
  packet->m_nInfoField2     = rtmp->m_stream_id;
  packet->m_hasAbsTimestamp = 0;
  packet->m_headerType      = RTMP_PACKET_SIZE_LARGE;
  if (type == RTMP_PACKET_TYPE_AUDIO && bufLen != 4)
    packet->m_headerType  = RTMP_PACKET_SIZE_MEDIUM;
  memcpy(packet->m_body, buf, bufLen);

  if (RTMP_IsConnected(rtmp)) {
    rc = RTMP_SendPacket(rtmp, packet, 0);
  }

  RTMPPacket_Free(packet);

  return rc;
}

int iotRtmp_SendMetadata(int width, int height, int framerate)
{
const int PACKET_SIZE  = 512;
const int VIDEOCODECID = 7;
  int rc = FALSE;

  if (!rtmp) return rc;

  RTMPPacket sPacket, *packet = &sPacket;
  RTMPPacket_Reset(packet);
  if (!RTMPPacket_Alloc(packet, PACKET_SIZE))
    return rc;

  uint8_t *p = (uint8_t *)packet->m_body, *ptr = p;
  uint8_t *pend = p + PACKET_SIZE;

  AVal avValue;
  AMFObject obj;
  AMFObjectProperty prop;

  memset(&obj, 0, sizeof(obj));
  AMF_Reset(&obj);


  avValue.av_val = "@setDataFrame";
  avValue.av_len = strlen(avValue.av_val);
  p = AMF_EncodeString(p, pend, &avValue);

  avValue.av_val = "onMetaData";
  avValue.av_len = strlen(avValue.av_val);
  p = AMF_EncodeString(p, pend, &avValue);

  avValue.av_val = "width";
  avValue.av_len = strlen(avValue.av_val);
  AMFProp_SetName(&prop, &avValue);
  prop.p_type = AMF_NUMBER;
  prop.p_vu.p_number = width;
  AMF_AddProp(&obj, &prop);

  avValue.av_val = "height";
  avValue.av_len = strlen(avValue.av_val);
  AMFProp_SetName(&prop, &avValue);
  prop.p_type = AMF_NUMBER;
  prop.p_vu.p_number = height;
  AMF_AddProp(&obj, &prop);

  avValue.av_val = "framerate";
  avValue.av_len = strlen(avValue.av_val);
  AMFProp_SetName(&prop, &avValue);
  prop.p_type = AMF_NUMBER;
  prop.p_vu.p_number = framerate;
  AMF_AddProp(&obj, &prop);

  avValue.av_val = "videocodecid";
  avValue.av_len = strlen(avValue.av_val);
  AMFProp_SetName(&prop, &avValue);
  prop.p_type = AMF_NUMBER;
  prop.p_vu.p_number = VIDEOCODECID;
  AMF_AddProp(&obj, &prop);

  p = AMF_Encode(&obj, p, pend);

  packet->m_nBodySize   = p - ptr;
  packet->m_packetType  = RTMP_PACKET_TYPE_INFO;
  packet->m_nChannel    = 0x04;
  packet->m_headerType  = RTMP_PACKET_SIZE_LARGE;
  packet->m_nInfoField2 = rtmp->m_stream_id;

  if (RTMP_IsConnected(rtmp))
    rc = RTMP_SendPacket(rtmp, packet, 0);
  RTMPPacket_Free(packet);

  AMF_Reset(&obj);

  return rc;
}

int iotRtmp_SendSpsPps(unsigned char *sps, int spsLen, unsigned char *pps, int ppsLen)
{
  int rc = FALSE;
  int pLen = spsLen + ppsLen + 32;

    RTMPPacket sPacket, *packet = &sPacket;
    RTMPPacket_Reset(packet);
    if (!RTMPPacket_Alloc(packet, pLen))
      return rc;

    int i = 0;
    unsigned char *body = (unsigned char *)packet->m_body;
    memset(body, 0, pLen);

    /* FLV videoTagHeader */
    body[i++] = 0x17;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    /* FLV videoTagBody: AVCDecoderConfigurationRecord */
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;
    /* sps */
    body[i++] = 0xe1;
    body[i++] = (uint8_t)(spsLen >> 8);
    body[i++] = (uint8_t) spsLen;
    memcpy(&body[i], sps, spsLen);
    i += spsLen;
    /* pps */
    body[i++] = 0x01;
    body[i++] = (uint8_t)(ppsLen >> 8);
    body[i++] = (uint8_t) ppsLen;
    memcpy(&body[i], pps, ppsLen);
    i += ppsLen;

    packet->m_nBodySize       = i;
    packet->m_nInfoField2     = rtmp->m_stream_id;
    packet->m_nChannel        = 0x04;
    packet->m_nTimeStamp      = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType      = RTMP_PACKET_TYPE_VIDEO;
    packet->m_headerType      = RTMP_PACKET_SIZE_MEDIUM;

  if (RTMP_IsConnected(rtmp)) {
    rc = RTMP_SendPacket(rtmp, packet, 0);
  }

  RTMPPacket_Free(packet);

  return rc;
}

int iotRtmp_SendH264Frame(unsigned char *data, unsigned int size, int isKeyframe, unsigned int ts)
{
  int rc = FALSE;
  int i = 0;

  if(data == NULL && size < 11) return rc;

  unsigned char *body = (unsigned char*) malloc(size+9);
  if (!body) return rc;

  memset(body, 0, size+9);

  if(isKeyframe) {
    body[i++] = 0x17;  // 1:Iframe  7:AVC
    body[i++] = 0x01;  // AVC NALU
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    // NALU size
    body[i++] = (uint8_t) (size>>24);
    body[i++] = (uint8_t) (size>>16);
    body[i++] = (uint8_t) (size>>8);
    body[i++] = (uint8_t)  size;
    // NALU data
    memcpy(&body[i], data, size);
// iotRtmp_SendSpsPps(metaData.sps, metaData.spsLen, metaData.pps, metaData.ppsLen);

  } else {
    body[i++] = 0x27;  // 2:Pframe  7:AVC
    body[i++] = 0x01;  // AVC NALU
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    // NALU size
    body[i++] = (uint8_t) (size>>24);
    body[i++] = (uint8_t) (size>>16);
    body[i++] = (uint8_t) (size>>8);
    body[i++] = (uint8_t)  size;
    // NALU data
    memcpy(&body[i], data, size);
  }

  rc = iotRtmp_SendPacket(body, i+size, RTMP_PACKET_TYPE_VIDEO, ts);

  free(body);

  return rc;
}


static uint32_t find_start_code(uint8_t *buf, uint32_t zeros_in_startcode);
static uint8_t * get_nal(uint32_t *len, uint8_t **offset, uint8_t *start, uint32_t total);

int iotRtmp_SendH264Packet(uint8_t *data, int dataLen, uint32_t ts)
{
  int rc = FALSE;

  if ((data == NULL) || (rtmp == NULL)) {
      return rc;
  }

  uint8_t *buf = data;
  uint8_t *bufOffset = data;
  uint32_t bufLen = dataLen;

  uint8_t *sps = NULL, *pps = NULL;
  uint32_t spsLen, ppsLen;

  while (1) {
    uint32_t nalLen;
    uint8_t *nal = get_nal(&nalLen, &bufOffset, buf, bufLen);

    if (nal == NULL) {
      rc = TRUE;
      break;
    }

    /* sps & pps */
    if (*nal == 0x67) {
      sps = nal;
      spsLen = nalLen;
      pps = get_nal(&ppsLen, &bufOffset, buf, bufLen);
      if (pps == NULL) {
        rc = FALSE;
        break;
      }

      rc = iotRtmp_SendSpsPps(sps, spsLen, pps, ppsLen);
      if (rc == FALSE) break;

    /* key frame, eg: I frame */
    } else if (*nal == 0x65) {
      if (sps && pps)
        rc = iotRtmp_SendSpsPps(sps, spsLen, pps, ppsLen);
        if (rc == FALSE) break;

        rc = iotRtmp_SendH264Frame(nal, nalLen, TRUE, ts);
        if (rc == FALSE) break;

    } else {
      rc = iotRtmp_SendH264Frame(nal, nalLen, FALSE, ts);
      if (rc == FALSE) break;
    }
  }

  return rc;
}


static uint32_t find_start_code(uint8_t *buf, uint32_t zeros_in_startcode)
{
  uint32_t info;
  uint32_t i;

  info = 1;
  if ((info = (buf[zeros_in_startcode] != 1)? 0: 1) == 0)
      return 0;

  for (i = 0; i < zeros_in_startcode; i++) {
    if (buf[i] != 0) {
        info = 0;
        break;
    }
  }

  return info;
}

static uint8_t * get_nal(uint32_t *len, uint8_t **offset, uint8_t *start, uint32_t total)
{
    uint32_t info;
    uint8_t *q ;
    uint8_t *p  =  *offset;
    *len = 0;

    if ((p - start) >= total)
        return NULL;

    while(1) {
        info =  find_start_code(p, 3);
        if (info == 1)
            break;
        p++;
        if ((p - start) >= total)
            return NULL;
    }
    q = p + 4;
    p = q;
    while(1) {
        info =  find_start_code(p, 3);
        if (info == 1)
            break;
        p++;
        if ((p - start) >= total)
            break;
    }

    *len = (p - q);
    *offset = p;
    return q;
}

