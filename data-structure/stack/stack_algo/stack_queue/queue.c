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
#include "../base/stack.h"
#include "queue.h"


struct queue_s {
  stack_t s_in;
  stack_t s_out;
};

#define ASSERT_QUEUE(q) {\
  if (NULL == (q)) {\
    fprintf(stderr, "queue invalid ...");\
    exit(0);\
  }\
}


queue_t 
queue_create(void)
{
  queue_t q = (queue_t)malloc(sizeof(*q));
  ASSERT_QUEUE(q);

  q->s_in = stack_create(128);
  q->s_out = stack_create(128);

  return q;
}

void 
queue_delete(queue_t* q) 
{
  if (NULL != *q) {
    stack_delete(&(*q)->s_in);
    stack_delete(&(*q)->s_out);

    free(*q);
    *q =  NULL;
  }
}

int 
queue_size(queue_t q) 
{
  ASSERT_QUEUE(q);

  return (stack_size(q->s_in) + stack_size(q->s_out));
}

void 
queue_enqueue(queue_t q, element_t e)
{
  ASSERT_QUEUE(q);

  stack_push(q->s_in, e);
}

element_t 
queue_dequeue(queue_t q)
{
  ASSERT_QUEUE(q);

  if (!stack_empty(q->s_out))
    return stack_pop(q->s_out);
  else {
    while (stack_size(q->s_in) > 1)
      stack_push(q->s_out, stack_pop(q->s_in));
    
    return stack_pop(q->s_in);
  }
}
