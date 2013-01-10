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
#include <assert.h>
#include <string.h>
#include "queue.h"


static void 
queue_test_create(void)
{
  queue_t q = queue_create(8);
  int i;
  double* d;

  assert(NULL != q);
  assert(queue_empty(q));

  srand(time(0));
  for (i = 0; i < 8; ++i) {
    d = (double*)malloc(sizeof(double));
    *d = rand() * 2322 * 0.12;
    queue_enqueue(q, d);
  }
  assert(!queue_empty(q));

  while (!queue_empty(q)) {
    d = queue_dequeue(q);
    free(d);
  }

  queue_delete(&q);

  fprintf(stdout, "Testing queue_create success ...\n");
}

static void 
queue_test_enqueue(element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_sz)
{
  queue_t q = queue_create(0);
  element_t elem;

  queue_enqueue(q, e);
  elem = queue_dequeue(q);

  assert(cmp(e, elem, elem_sz));

  queue_delete(&q);

  fprintf(stdout, "Testing queue enqueue and dequeue success ...\n");
}

static void 
queue_show(void) 
{
  int i;
  double* d;
  queue_t q = queue_create(0);

  fprintf(stdout, "queue length is : %d\n", queue_size(q));
  srand(time(0));
  fprintf(stdout, "enqueue ===>\n");
  for (i = 0; i < 8; ++i) {
    d = (double*)malloc(sizeof(double));
    *d = rand() % 2434 * 1.22;
    fprintf(stdout, "%lf\t", *d);
    queue_enqueue(q, d);
  }
  fprintf(stdout, "\n\n");

  fprintf(stdout, "queue length is : %d\n", queue_size(q));
  fprintf(stdout, "dequeue ===>\n");
  for (i = 0; i < 4; ++i) {
    d = (double*)queue_dequeue(q);
    fprintf(stdout, "%lf\t", *d);
    free(d);
  }
  fprintf(stdout, "\n\n");
  
  fprintf(stdout, "queue length is : %d\n", queue_size(q));
  srand(time(0));
  fprintf(stdout, "enqueue ===>\n");
  for (i = 0; i < 4; ++i) {
    d = (double*)malloc(sizeof(double));
    *d = rand() % 4546 * 1.22;
    fprintf(stdout, "%lf\t", *d);
    queue_enqueue(q, d);
  }
  fprintf(stdout, "\n\n");
  
  fprintf(stdout, "queue length is : %d\n", queue_size(q));
  fprintf(stdout, "dequeue ===>\n");
  while (!queue_empty(q)) {
    d = (double*)queue_dequeue(q);
    fprintf(stdout, "%lf\t", *d);
    free(d);
  }
  fprintf(stdout, "\n\n");
  fprintf(stdout, "queue length is : %d\n", queue_size(q));

  queue_delete(&q);
}


static int 
queue_cmp(element_t a, element_t b, int elem_sz) 
{
  return (0 == memcmp(a, b, elem_sz));
}

int 
main(int argc, char* argv[])
{
  double* d;
  queue_test_create();

  d = (double*)malloc(sizeof(double));
  *d = 34.56;
  queue_test_enqueue(d, queue_cmp, sizeof(double));
  free(d);

  queue_show();

  return 0;
}
