#include <pthread.h>

#include "MQTTClient.h"
#include "iotMqtt.h"


static MQTTMessage* message = NULL;
static int messageValid   = FAILURE;

static unsigned char *txBuf = NULL;
static unsigned char *rxBuf = NULL;

static Network network;
static MQTTClient client;

static pthread_t pid;

static void messageArrived(MessageData* md)
{
  MQTTMessage* msg = md->message;

  if (0)
    printf("%.*s", (int)msg->payloadlen, (char*)msg->payload);

  message = md->message;
  messageValid = SUCCESS;
}

MQTTMessage * iotMqtt_GetMessage(void)
{
  if ( (messageValid != SUCCESS) || !message ) {
    return NULL;
  }

  messageValid = FAILURE;
  return message;
}

int iotMqtt_Init()
{
  message = NULL;
  messageValid   = FAILURE;
  txBuf = NULL;
  rxBuf = NULL;
}

int iotMqtt_Connect(const char *clientid, const char *username, const char *password, int txBufsize, int rxBufsize)
{
const int  cmd_timeout = 1000;
const int  port = 6002;
const char *host = "183.230.40.39";


  int rc = FAILURE;

  NetworkInit(&network);
  if (NetworkConnect(&network, host, port) != 0) {
    printf("NetworkConnect() Failed!\n");
    NetworkDisconnect(&network);
    return rc;
  }

  txBuf = (unsigned char *) malloc(txBufsize);
  rxBuf = (unsigned char *) malloc(rxBufsize);

  if (!txBuf || !rxBuf) {
    printf("Malloc Failed!\n");
    goto exit;
  }

  MQTTClientInit(&client, &network, cmd_timeout, txBuf, txBufsize, rxBuf, rxBufsize);

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.MQTTVersion = 4;  /* OneNet must be 4 */
  data.clientID.cstring = clientid;
  data.username.cstring = username;
  data.password.cstring = password;
  data.keepAliveInterval = 120;  /* at least 120s */
  data.cleansession = 1;

  rc = MQTTConnect(&client, &data);
  if (rc != SUCCESS) {
    printf("MQTTConnect Failed!\n");
    goto exit;
  }

  /* TODO: cmd & cmd callback */
  rc = MQTTSubscribe(&client, "$creq/+", QOS0, messageArrived);
  if (rc != SUCCESS) {
    printf("MQTTSubscribe Failed!\n");
    goto exit;
  }

  return rc;

exit:
  if (txBuf) free(txBuf);
  if (rxBuf) free(rxBuf);
  NetworkDisconnect(&network);
  return rc;
}

int iotMqtt_Disconnect(void)
{
  if (txBuf) free(txBuf);
  if (rxBuf) free(rxBuf);
  NetworkDisconnect(&network);
}

int iotMqtt_isConnect(void)
{
  return MQTTIsConnected(&client);
}

int iotMqtt_PublishJson(unsigned char *jsonBuf)
{
  int rc = FAILURE;

  int offset = 3;
  int len = strlen(jsonBuf);

  unsigned char *payload = (unsigned char *)malloc (len + offset);
  if (!payload) {
    return BUFFER_OVERFLOW;
  }
  char *p = payload;
  *p++ = 1;
  *p++ = (len & 0x0000ff00UL) >> 8;
  *p++ = (len & 0x000000ffUL);

  memcpy(p, jsonBuf, len);

  MQTTMessage message;
  message.qos        = QOS0;
  message.dup        = 0;
  message.retained   = 0;
  message.payload    = (void*)payload;
  message.payloadlen = len + offset;

  rc = MQTTPublish(&client, "$dp", &message);

  free(payload);
  return rc;
}


int mqttRunning = 0;

void iotMqtt_Stop(void)
{
  if (mqttRunning)
    mqttRunning = 0;
}

void* iotMqtt_Schedule(void *arg)
{
const int timeout_ms = 3000;

  (void) arg;

  mqttRunning = 1;

  while (mqttRunning) {
    Timer timer;
    TimerInit(&timer);
    TimerCountdownMS(&timer, timeout_ms);

    if (cycle(&client, &timer) < 0) {
      printf("MQTT cycle() Failed!!!\n");
//      break;
    }
  }

  printf("MQTT Exit!\n");
  iotMqtt_Disconnect();
  return NULL;
}

int iotMqtt_StartTask(struct deviceInfo *device)
{
  int rc;
  uint8_t buf[512] = {0};
  const char *j = "{\"datastreams\":[{\"id\":\"ont_video_1_%s\",\"datapoints\":[{\"value\":{\"dst\":\"video\",\"beginTime\":\"\",\"endTime\":\"\",\"videoDesc\":\"Nothing\"}}]}]}";
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), j, device->cameraName);

  rc = iotMqtt_Connect(device->clientid, device->username, device->password, 1024, 1024);
  if (rc != SUCCESS) {
    printf("iotMqtt_Connect Failed!\n");
    return rc;
  }
  rc = iotMqtt_PublishJson(buf);
  if (rc != SUCCESS) {
    iotMqtt_Disconnect();
    printf("iotMqtt_PublishJson Failed!\n");
    return rc;
  }

  rc = pthread_create(&pid, NULL, iotMqtt_Schedule, NULL);
  pthread_detach(pid);

  if (rc != SUCCESS) {
    iotMqtt_Disconnect();
    printf("pthread_create(iotMqtt_Schedule) Failed!!!\n");
  }

  return rc;
}

#if 0
int iotMqtt_Schedule(uint32_t timeout_ms)
{
  int rc = SUCCESS;
  Timer timer;

  TimerInit(&timer);
  TimerCountdownMS(&timer, timeout_ms);

  if (cycle(&client, &timer) < 0) {
      rc = FAILURE;
  }
  return rc;
}
#endif
