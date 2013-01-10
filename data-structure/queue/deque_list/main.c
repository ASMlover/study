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
#include <time.h>
#include <stdlib.h>
#include "deque.h"


#define SHOW_FUNC_INFO(in, out) {\
  if (deque_push_front == (in))\
    fprintf(stdout, "\n{in=>deque_push_front, ");\
  else\
    fprintf(stdout, "\n{in=>deque_push_rear, ");\
  if (deque_pop_front == (out))\
    fprintf(stdout, "out=>deque_pop_front}\n");\
  else\
    fprintf(stdout, "out=>deque_pop_rear}\n");\
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
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n * rand_d;
    in(q, d);
    fprintf(stdout, "[%02d] deque enqueue element : %lf\n", i, *d);
  }
  fprintf(stdout, "deque size is : %d ===>\n", deque_size(q));
  d = deque_peek_front(q);
  fprintf(stdout, "\tfront element of deque : %lf\n", *d);
  d = deque_peek_rear(q);
  fprintf(stdout, "\trear element of deque: %lf\n", *d);
  while (!deque_empty(q)) {
    d = out(q);
    fprintf(stdout, "deque dequeue element : %lf\n", *d);
    free(d);
  }
}



int
main(int argc, char* argv[]) 
{
  deque_t q = deque_create();

  deque_test_enqueue_dequeue(q, 
      deque_push_front, deque_pop_front, 34352, 0.10234);
  deque_test_enqueue_dequeue(q, 
      deque_push_front, deque_pop_rear, 454523, 1.3022);
  deque_test_enqueue_dequeue(q, 
      deque_push_rear, deque_pop_front, 78797, 0.17672);
  deque_test_enqueue_dequeue(q, 
      deque_push_rear, deque_pop_rear, 56239, 1.7987);

  deque_delete(&q);
  return 0;
}
