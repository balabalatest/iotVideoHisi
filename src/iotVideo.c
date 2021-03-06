/*
 ============================================================================
 Name        : iotVideo.c
 Author      : lichen
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "iotMqtt.h"
#include "cJSON.h"

#define USE_QUEUE
//#define USE_FIFO

//#define QUEUE_BUFSIZE  (128*1024)

#ifdef USE_QUEUE
#include "queue.h"
#else
#include "ringfifo.h"
struct ringbuf ringinfo;
#endif


#include "sample_comm.h"


static struct deviceInfo device =
{
  .cameraName = "hisi",
  .clientid = "20444004",
  .username = "104564",
  .password = "cameraDeviceAuth",
};

void usage()
{
  printf("Usage: -- <options>, where options are:\n");
  printf("  --name (default is hisi)\n");
  printf("  --clientid \n");
  printf("  --username \n");
  printf("  --password \n");
  exit(-1);
}

void getopts(int argc, char** argv)
{
  int count = 1;

  while (count < argc)
  {
    if (strcmp(argv[count], "--name") == 0) {
      if (++count < argc)
        device.cameraName = argv[count];
      else
        usage();
    } else if (strcmp(argv[count], "--clientid") == 0) {
      if (++count < argc)
        device.clientid = argv[count];
      else
        usage();
    } else if (strcmp(argv[count], "--username") == 0) {
      if (++count < argc)
        device.username = argv[count];
      else
        usage();
    } else if (strcmp(argv[count], "--password") == 0) {
      if (++count < argc)
        device.password = argv[count];
      else
        usage();
    }
    count++;
  }
}

int mainRunning = 0;

static void HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
      mainRunning = 0;
      printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    if (SIGPIPE == signo) {
      printf("\033[0;31mSIGPIPE! Server maybe closed!!!\033[0;39m\n");
    }
}

int main(int argc, char **argv)
{
  int rc;
  pthread_t id;

#if 1
  getopts(argc, argv);

  rc = iotMqtt_StartTask(&device);
  if (rc != SUCCESS) {
    printf("iotMqtt_StartTask() Failed!!!\n");
    exit(-1);
  }
#endif

  mainRunning = 0;
  signal(SIGINT, HandleSig);
  signal(SIGTERM, HandleSig);
  signal(SIGPIPE, HandleSig);
  mainRunning = 1;

  while (mainRunning) {

    if (!iotMqtt_isConnect())
      break;

    uint8_t cmdbuf[256] = {0};
    int cmdid = func_MqttGetCmd(cmdbuf);
    if (cmdid == 1) {
      strcat(cmdbuf, "?");
      strcat(cmdbuf, device.clientid);
    }

    func_RtmpSchedule(cmdid, cmdbuf);

//    usleep(1);
  }

  func_RtmpScheduleStop();

  iotMqtt_Disconnect();

  printf("System Exit...!!!\n");

  exit(0);
}

#define QUEUE_BUFSIZE  (160*1024)
static uint8_t bufRecv[QUEUE_BUFSIZE];
extern void* SAMPLE_VENC_1080P_CLASSIC(HI_VOID *arg);

int main0(int argc, char *argv[])
{
  static pthread_t id;

  static int mode = 2;

  int width, height, framerate = 30, bitrate = 2048;
  if (mode == 0) {
    width = 1280; height = 720;
  } else if (mode == 1) {
    width = 640; height = 480;
  } else if (mode == 2) {
    width = 320; height = 240; bitrate = 512;
  }


  char url[100];
  if(argc!=2)
  {
    printf("Usage: %s URL\n", argv[0]);
    return -1;
  }
  sprintf(url, "rtmp://%s/live/stream",argv[1]);
  printf("Server: %s\n",url);

  if (!queue_init(QUEUE_BUFSIZE)) {
    printf("queue init failed!!!\n");
  }
  int rc = iotRtmp_Connect(url, 10);
  if (!rc) {
    printf("connect %s failed!!!\n",url);
    return -1;
  }

  pthread_create(&id,NULL,SAMPLE_VENC_1080P_CLASSIC,(void*)&mode);


retry:
  while (queue_empty()) usleep(33000);

#if 1
  rc = iotRtmp_SendMetadata(width, height, framerate, bitrate);
  if (rc == 0) {
    printf("iotRtmp_SendMetadata failed!!!\n");
  }
#endif

  uint32_t ts = 0;

  int len = queue_recv(bufRecv, QUEUE_BUFSIZE);
  if (len) {

    ts = RTMP_GetTime();
    rc = iotRtmp_SendFirstFrame(bufRecv, len, 0);
    if (rc == 0)
      goto retry;
    if (rc == -1) {
      printf("iotRtmp_SendFirstFrame failed!!!\n");
      while(1);
    } else if (rc > 0) {
      printf("iotRtmp_SendFirstFrame OK!\n");
    }
  } else {
    printf("queue_recv len invalid!!!\n");
  }


  while (1) {
    int len = queue_recv(bufRecv, QUEUE_BUFSIZE);
    if (len > 0) {

      uint32_t t1 = RTMP_GetTime();

//      if (!iotRtmp_SendH264Packet(bufRecv, len, ts+=33))  /* will make VLC error: Gray Screen */

      if (!iotRtmp_SendH264Packet(bufRecv, len, RTMP_GetTime() - ts))
        printf("iotRtmp_SendH264Packet() error!!!\n");

      uint32_t t2 = RTMP_GetTime();
      if (t2-t1 > 500)
        printf("Send time: %u\n", t2-t1);

    }
  }

  queue_deinit();
  iotRtmp_Disconnect();

  return 0;
}


#if 0
extern void* SAMPLE_VENC_1080P_CLASSIC(void *arg);
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
static void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

int main(int argc, char **argv)
{
  int rc;
  pthread_t id;

#if 1
  getopts(argc, argv);

  rc = iotMqtt_StartTask(&device);
  if (rc != SUCCESS) {
    printf("iotMqtt_StartTask() Failed!!!\n");
    sleep(1);
    exit(-1);
  }
#endif

  uint32_t rtmpTime  = 0;
  uint8_t url[256] = {0};

  int rtmpState = 0;
  sprintf(url, "rtmp://%s/live/livestream", "192.168.1.100");

  while (1) {

    MQTTMessage *message;
    message = iotMqtt_GetMessage();

    if (message) {
      printf("Get Platform Cmd!\n");
      cJSON *json = cJSON_Parse((char*)message->payload);
      if (json) {
#if 1
        cJSON *type = cJSON_GetObjectItem(json, "type");
        if (type) {
          if (strcmp(type->valuestring, "video"))
            printf("Platform cmd type error!\n");
        }
        cJSON *cmdid = cJSON_GetObjectItem(json, "cmdId");

        if (cmdid) {
          printf("Platform cmdid: %d\n", cmdid->valueint);
          switch (cmdid->valueint) {
          case 1:
            break;
          case 6:
            break;
          default:
            break;
          }
        }
#endif

        cJSON *cmd  = cJSON_GetObjectItem(json, "cmd");
        cJSON *jurl = cJSON_GetObjectItem(cmd,  "pushUrl");
        if (cmd && jurl) {
          rtmpState = 1;
          memset(url, 0, sizeof(url));
          snprintf(url, sizeof(url), "%s?%s", jurl->valuestring, device.clientid);
          printf("url: %s\n", url);
        }
        cJSON_Delete(json);
      }
    }

    switch (rtmpState) {
    case 0:
      break;

    case 1:

      if (iotRtmp_Connect(url, 10)) {

int mode = 0;
int width, height, framerate = 30;
int bitrate = 2048;

        if (mode == 0) {
          width = 1280; height = 720;
        } else if (mode == 1) {
          width = 640; height = 480;
        } else if (mode == 2) {
          width = 320; height = 240; bitrate = 512;
        }
        iotRtmp_SendMetadata(width, height, framerate, bitrate);


        rtmpState = 2;
        rtmpTime  = RTMP_GetTime();


#ifdef USE_QUEUE
        if (!queue_init(QUEUE_BUFSIZE))
        {
          printf("Queue init Failed!!!\n");
          rtmpState = 3;
        }
#else
        ringmalloc(QUEUE_BUFSIZE);
#endif
        pthread_create(&id, NULL, SAMPLE_VENC_1080P_CLASSIC, (void*)&mode);

        signal(SIGINT, SAMPLE_VENC_HandleSig);
        signal(SIGTERM, SAMPLE_VENC_HandleSig);

        rtmpTime  = RTMP_GetTime();
      }
      break;

    case 2: {
#ifdef USE_QUEUE
      uint8_t buf[QUEUE_BUFSIZE];
      int len = queue_recv(buf, QUEUE_BUFSIZE);
      if (len) {
        if (!iotRtmp_SendH264Packet(buf, len, RTMP_GetTime()-rtmpTime)) {
          printf("iotRtmp_SendH264Packet() error!!!\n");
          rtmpState = 3;
        }
      }
#else
      int len = ringget(&ringinfo);
      if (len) {
        if (!iotRtmp_SendH264Packet(ringinfo.buffer, ringinfo.size, RTMP_GetTime()-rtmpTime)) {
          printf("iotRtmp_SendH264Packet() error!!!\n");
          rtmpState = 3;
        }
      }
#endif
    } break;

    case 3:
#ifdef USE_QUEUE
      queue_deinit();
#else
      ringfree();
#endif

      iotRtmp_Disconnect();
      break;
    }

//    usleep(1);
  }

  return 0;
}


#endif
