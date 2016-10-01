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
#include "devil_test.h"
#include "../src/devil_queue.h"

void
devil_test_queue(void)
{
  devil_queue_t* queue = NULL;
  void* v;
  size_t size;

  fprintf(stdout, "test devil_queue_t <%s>\n", __func__);

  fprintf(stdout, "\ttest devil_queue_size\n");
  queue = devil_queue_create();
  DEVIL_ASSERT(NULL != queue);
  size = devil_queue_size(queue);
  DEVIL_ASSERT(0 == size);

  fprintf(stdout, "\ttest devil_queue_push\n");
  devil_queue_push(queue, (void*)34);
  devil_queue_push(queue, (void*)57);
  devil_queue_push(queue, (void*)9);
  size = devil_queue_size(queue);
  DEVIL_ASSERT(3 == size);

  fprintf(stdout, "\ttest devil_queue_pop\n");
  v = devil_queue_pop(queue);
  DEVIL_ASSERT(34 == (int)v);
  size = devil_queue_size(queue);
  DEVIL_ASSERT(2 == size);

  v = devil_queue_pop(queue);
  DEVIL_ASSERT(57 == (int)v);
  size = devil_queue_size(queue);
  DEVIL_ASSERT(1 == size);

  v = devil_queue_pop(queue);
  DEVIL_ASSERT(9 == (int)v);
  size = devil_queue_size(queue);
  DEVIL_ASSERT(0 == size);

  fprintf(stdout, "\ttest devil_queue_release\n");
  devil_queue_release(queue);
  fprintf(stdout, "test devil_queue_t : all passed\n");
}
