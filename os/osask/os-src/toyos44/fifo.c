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
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#include "fifo.h"


#define FLAGS_OVERRUN   (0x0001)



void 
fifo_init(fifo8_t* fifo, unsigned char* buf, int storage)
{
  /* initialize FIFO buffer */
  fifo->buf = buf;
  fifo->storage = storage;  /* size of buffer */
  fifo->size = 0;
  fifo->front = 0;
  fifo->rear = 0;
  fifo->flags = 0;
}

int 
fifo_put(fifo8_t* fifo, unsigned char data)
{
  if (fifo->size == fifo->storage) {
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }

  fifo->buf[fifo->rear] = data;
  fifo->rear = (fifo->rear + 1) % fifo->storage;
  ++fifo->size;

  return 0;
}

int 
fifo_get(fifo8_t* fifo)
{
  int data;

  if (0 == fifo->size)
    return -1;

  data = fifo->buf[fifo->front];
  fifo->front = (fifo->front + 1) % fifo->storage;
  --fifo->size;

  return data;
}

int 
fifo_size(fifo8_t* fifo)
{
  return fifo->size;
}
