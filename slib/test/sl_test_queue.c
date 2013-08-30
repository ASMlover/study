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
#include "sl_test_header.h"
#include "../src/sl_queue.h"



void 
sl_test_queue(void)
{
  sl_queue_t* queue = NULL;
  void* v;
  size_t size;

  fprintf(stdout, "test sl_queue_t <%s>\n", __func__);

  fprintf(stdout, "\ttest sl_queue_size\n");
  queue = sl_queue_create();
  ASSERT(NULL != queue);
  size = sl_queue_size(queue);
  ASSERT(0 == size);

  fprintf(stdout, "\ttest sl_queue_push\n");
  sl_queue_push(queue, (void*)34);
  sl_queue_push(queue, (void*)57);
  sl_queue_push(queue, (void*)9);
  size = sl_queue_size(queue);
  ASSERT(3 == size);

  fprintf(stdout, "\ttest sl_queue_pop\n");
  v = sl_queue_pop(queue);
  ASSERT(34 == (int)v);
  size = sl_queue_size(queue);
  ASSERT(2 == size);

  v = sl_queue_pop(queue);
  ASSERT(57 == (int)v);
  size = sl_queue_size(queue);
  ASSERT(1 == size);

  v = sl_queue_pop(queue);
  ASSERT(9 == (int)v);
  size = sl_queue_size(queue);
  ASSERT(0 == size);

  fprintf(stdout, "\ttest sl_queue_release\n");
  sl_queue_release(queue);
  fprintf(stdout, "test sl_queue_t : all passed\n");
}
