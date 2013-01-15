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
#include "circular_buffer.h"


struct circular_buffer_s {
  int capacity;
  int size;
  int front;
  int rear;
  char* elements;
};


#define CB_CAPACITY_DEF   (8)
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




circular_buffer_t 
circular_buffer_create(int capacity) 
{
  circular_buffer_t cb = (circular_buffer_t)malloc(sizeof(*cb));
  ASSERT_CB(cb);

  cb->capacity = (capacity < CB_CAPACITY_DEF ? CB_CAPACITY_DEF : capacity);
  cb->size = 0;
  cb->front = cb->rear = 0;
  cb->elements = (char*)malloc(sizeof(char) * cb->capacity);
  if (NULL == cb->elements) {
    fprintf(stderr, "create circular buffer failed ...");
    exit(0);
  }

  return cb;
}

void 
circular_buffer_delete(circular_buffer_t* cb) 
{
  if (NULL != *cb) {
    if (NULL != (*cb)->elements) 
      free((*cb)->elements);
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

void 
circular_buffer_put(circular_buffer_t cb, char c) 
{
  ASSERT_CB(cb);
  ASSERT_CB_OVERFLOW(cb);

  cb->elements[cb->rear] = c;
  cb->rear = (cb->rear + 1) % cb->capacity;
  ++cb->size;
}

char 
circular_buffer_get(circular_buffer_t cb) 
{
  char c;
  ASSERT_CB(cb);
  ASSERT_CB_UNDERFLOW(cb);

  c = cb->elements[cb->front];
  cb->front = (cb->front + 1) % cb->capacity;
  --cb->size;

  return c;
}
