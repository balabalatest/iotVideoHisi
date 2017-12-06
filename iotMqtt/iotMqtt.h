/*
 * iotMqtt.h
 *
 *  Created on: Nov 20, 2017
 *      Author: lichen
 */

#ifndef IOTMQTT_H_
#define IOTMQTT_H_

#include "MQTTClient.h"

struct deviceInfo
{
  char* cameraName;
  char* clientid;
  char* username;
  char* password;
};

int iotMqtt_Init(void);
MQTTMessage * iotMqtt_GetMessage(void);
int iotMqtt_Connect(const char *clientid, const char *username, const char *password, int txBufsize, int rxBufsize);
int iotMqtt_Disconnect(void);
void* iotMqtt_Schedule(void *arg);



#endif /* IOTMQTT_H_ */
