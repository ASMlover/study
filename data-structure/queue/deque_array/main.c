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
#include <time.h>
#include "deque.h"


#define SHOW_FUNC_INFO(in, out) {\
  fprintf(stdout, "\n{");\
  if (deque_push_front == in)\
    fprintf(stdout, "deque_push_front, ");\
  else\
    fprintf(stdout, "deque_push_rear, ");\
  if (deque_pop_front == out)\
    fprintf(stdout, "deque_pop_front}\n");\
  else\
    fprintf(stdout, "deque_pop_rear}\n");\
}

static void 
deque_test_enqueue_dequeue(deque_t q, 
  void (*in)(deque_t, element_t), 
  element_t (*out)(deque_t), 
  int rand_n, double rand_d)
{
  int i;
  double* d;
  
  SHOW_FUNC_INFO(in, out);
  srand(time(0));
  for (i = 0; i < 8; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n * rand_d;
    in(q, d);

    fprintf(stdout, "[%d] the enqueue element is : %lf\n", 
      i, *d);
  }
  fprintf(stdout, "deque size is : %d ===>\n", deque_size(q));
  d = (double*)deque_peek_front(q);
  fprintf(stdout, "\tthe deque front element value is : %lf\n", *d);
  d = (double*)deque_peek_rear(q);
  fprintf(stdout, "\tthe deque rear element value is : %lf\n", *d);
  while (!deque_empty(q)) {
    d = (double*)out(q);
    fprintf(stdout, "the dequeue element is : %lf\n", *d);
    free(d);
  }
  fprintf(stdout, "deque size is : %d\n", deque_size(q));
}


int 
main(int argc, char* argv[])
{
  deque_t q = deque_create(0);

  deque_test_enqueue_dequeue(q, deque_push_front, 
    deque_pop_front, 3434, 0.1221);
  deque_test_enqueue_dequeue(q, deque_push_front, 
    deque_pop_rear, 54656, 0.1542);
  deque_test_enqueue_dequeue(q, deque_push_rear, 
    deque_pop_front, 89763, 0.3431);
  deque_test_enqueue_dequeue(q, deque_push_rear, 
    deque_pop_rear, 23209, 0.8975);

  deque_delete(&q);

  return 0;
}
