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
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"


int 
main(int argc, char* argv[])
{
  int i;
  double* d;
  queue_t q = queue_create();

  assert(queue_empty(q));

  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % 2324 * 1.45601;
    fprintf(stdout, "[%d] queue enqueue value is : %lf\n", i, *d);
    queue_enqueue(q, d);
  }
  assert(10 == queue_size(q));

  fprintf(stdout, "\n");
  while (!queue_empty(q)) {
    d = (double*)queue_dequeue(q);
    fprintf(stdout, "queue dequeue value is : %lf\n", *d);
    free(d);
  }
  assert(0 == queue_size(q));
  
  fprintf(stdout, "\n");
  srand(time(0));
  for (i = 0; i < 5; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % 5653 * 1.456546;
    fprintf(stdout, "[%d] queue enqueue value is : %lf\n", i, *d);
    queue_enqueue(q, d);
  }
  assert(5 == queue_size(q));

  fprintf(stdout, "\n");
  while (!queue_empty(q)) {
    d = (double*)queue_dequeue(q);
    fprintf(stdout, "queue dequeue value is : %lf\n", *d);
    free(d);
  }
  assert(0 == queue_size(q));

  queue_delete(&q);
  return 0;
}
