/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "circular_buffer.h"


struct circular_buffer_s {
  int capacity;
  int size;
  int front;
  int rear;
  char* buffer;
};

#define CB_CAPACITY_DEF   (1024)
#define ASSERT_CB(cb) {\
  if (NULL == (cb)) {\
    fprintf(stderr, "circular buffer invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_CB_UNDERFLOW(cb) {\
  if (0 == (cb)->size) {\
    fprintf(stderr, "circular buffer underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_CB_OVERFLOW(cb) {\
  if ((cb)->capacity == (cb)->size) {\
    fprintf(stderr, "circular buffer overflow ...");\
    exit(0);\
  }\
}
#define ASSERT_CB_ARGUMENTS(buf, len) {\
  if (NULL == (buf) || (len) <= 0) {\
    fprintf(stderr, "circular buffer arguments failed ...");\
    exit(0);\
  }\
}




circular_buffer_t 
circular_buffer_create(int capacity) 
{
  circular_buffer_t cb = (circular_buffer_t)malloc(sizeof(*cb));
  ASSERT_CB(cb);

  cb->capacity = (capacity < CB_CAPACITY_DEF ? CB_CAPACITY_DEF : capacity);
  cb->size = 0;
  cb->front = cb->rear = 0;
  cb->buffer = (char*)malloc(sizeof(char) * cb->capacity);
  if (NULL == cb->buffer) {
    fprintf(stderr, "create circular buffer failed ...");
    exit(0);
  }

  return cb;
}

void 
circular_buffer_delete(circular_buffer_t* cb) 
{
  if (NULL != *cb) {
    if (NULL != (*cb)->buffer)
      free((*cb)->buffer);
    free(*cb);
    *cb = NULL;
  }
}

int 
circular_buffer_empty(circular_buffer_t cb) 
{
  ASSERT_CB(cb);

  return (0 == cb->size);
}

int 
circular_buffer_size(circular_buffer_t cb)
{
  ASSERT_CB(cb);

  return (cb->size);
}

int 
circular_buffer_put(circular_buffer_t cb, const char* buf, int len)
{
  int copy_len, free_len;
  ASSERT_CB(cb);
  ASSERT_CB_OVERFLOW(cb);
  ASSERT_CB_ARGUMENTS(buf, len);

  free_len = cb->capacity - cb->size;
  copy_len = (len < free_len ? len : free_len);
  if (cb->rear < cb->front) 
    memcpy(cb->buffer + cb->rear, buf, copy_len);
  else {
    int left_len, tail_len = cb->capacity - 1 - cb->rear;
    tail_len = (copy_len > tail_len ? tail_len : copy_len);
    left_len = copy_len - tail_len;

    memcpy(cb->buffer + cb->rear, buf, tail_len);
    if (left_len > 0) 
      memcpy(cb->buffer, buf + tail_len, left_len);
  }
  cb->rear = (cb->rear + copy_len + 1) % cb->capacity;
  cb->size += copy_len;

  return copy_len;
}

int 
circular_buffer_get(circular_buffer_t cb, int len, char* buf)
{
  int copy_len;
  ASSERT_CB(cb);
  ASSERT_CB_UNDERFLOW(cb);
  ASSERT_CB_ARGUMENTS(buf, len);

  copy_len = (cb->size > len ? len : cb->size);
  if (cb->front < cb->rear) 
    memcpy(buf, cb->buffer + cb->front, copy_len);
  else {
    int left_len, tail_len = cb->capacity - 1 - cb->front;
    tail_len = (copy_len > tail_len ? tail_len : copy_len);
    left_len = copy_len - tail_len;

    memcpy(buf, cb->buffer + cb->front, tail_len);
    if (left_len > 0)
      memcpy(buf + tail_len, cb->buffer, left_len);
  }
  cb->front = (cb->front + copy_len + 1) % cb->capacity;
  cb->size -= copy_len;

  return copy_len;
}
