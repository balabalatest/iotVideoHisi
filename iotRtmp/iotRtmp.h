/*
 * iotRtmp.h
 *
 *  Created on: Nov 20, 2017
 *      Author: lichen
 */

#ifndef IOTRTMP_H_
#define IOTRTMP_H_

#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"
#include "librtmp/amf.h"
#include "librtmp/log.h"



int iotRtmp_Connect(const char *url, int timeout);
void iotRtmp_Disconnect(void);
int iotRtmp_SendH264Packet(uint8_t *data, int dataLen, uint32_t ts);
int iotRtmp_SendMetadata(int width, int height, int framerate, int bitrate);


#endif /* IOTRTMP_H_ */
