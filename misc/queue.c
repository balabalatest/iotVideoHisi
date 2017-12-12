/*
 * queue.c
 *
 *  Created on: Nov 28, 2017
 *      Author: lichen
 */

#include "queue.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

struct queue *q = (struct queue *) 0;

int queue_init(int size)
{
  q = (struct queue *) calloc(1, sizeof(struct queue));
  if (!q) {
    printf("QueueInit Failed!!!\n");
    return false;
  }

  q->memPerSize = size;
  q->memStroage = (uint8_t *) calloc(QUEUE_SIZE, size);
  if (!q->memStroage) {
    free(q);
    printf("QueueInit Failed!!!\n");
    return false;
  }

  int i;
  uint8_t *p;

  for (i = 0, p = q->memStroage; i < QUEUE_SIZE; i++, p += size) {
    q->memArray[i].buf = p;
  }

  return true;
}

int queue_deinit(void)
{
  if (q) {
    if (q->memStroage) {
      free(q->memStroage);
      q->memStroage = NULL;
    }
    free(q);
    q = NULL;
  }
}

int queue_full(void)
{
  return ((1 + q->tail) % QUEUE_SIZE == q->head) ? 1:0;
}

int queue_empty(void)
{
  return (q->head == q->tail) ? 1:0;
}

void queue_flush(void)
{
  while(q->head != q->tail) {
    q->head = (++q->head) % QUEUE_SIZE;
  }
}

int queue_send(uint8_t *data, int dataLen)
{
  if ((1 + q->tail) % QUEUE_SIZE == q->head) {
    printf("Queue Buffer Full!\n");
    return 0;
  }

  int len = MIN(dataLen, q->memPerSize);
  memcpy(q->memArray[q->tail].buf, data, len);
  q->memArray[q->tail].bufLen = len;

  if (++q->tail >= QUEUE_SIZE) {
    q->tail = 0;
  }

  return len;
}

int queue_recv(uint8_t *buf, int bufLen)
{
  if (q->head == q->tail)
    return 0;

  int len = MIN(bufLen, q->memArray[q->head].bufLen);
  memcpy(buf, q->memArray[q->head].buf, len);

  if (++q->head >= QUEUE_SIZE) {
    q->head = 0;
  }

  return len;
}
