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
#include "list.h"


#define SHOW_FUNC(s, in, out) {\
  if (list_push_front == (in))\
    fprintf(stdout, "\n%s => {in=>list_push_front, ", (s));\
  else\
    fprintf(stdout, "\n%s => {in=>list_push_back, ", (s));\
  if (list_pop_front == (out))\
    fprintf(stdout, "out=>list_pop_front}\n");\
  else\
    fprintf(stdout, "out=>list_pop_back}\n");\
}

static void 
list_test(list_t l, 
  void (*in)(list_t, element_t), 
  element_t (*out)(list_t), 
  int rand_n, double rand_d)
{
  int i;
  double* d;

  SHOW_FUNC("list_test", in, out);
  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n * rand_d;
    fprintf(stdout, "list append {index=>%d, value=>%lf}\n", i, *d);
    in(l, d);
  }
  fprintf(stdout, "the list size is : %d\n", list_size(l));
  while (!list_empty(l)) {
    d = (double*)out(l);
    fprintf(stdout, "the removed element {value=>%lf}\n", *d);
    free(d);
  }
  fprintf(stdout, "the list size is : %d\n", list_size(l));
}

int 
main(int argc, char* argv[])
{
  int i;
  double* d;
  list_t l = list_create();

  list_test(l, list_push_front, list_pop_front, 45466, 0.001238);
  list_test(l, list_push_front, list_pop_back, 786723, 0.12078);
  list_test(l, list_push_back, list_pop_front, 96753, 1.008962);
  list_test(l, list_push_back, list_pop_back, 2344, 0.12046);

  srand(time(0));
  for (i = 0; i < 5; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % 24435 * 0.00431;
    list_push_back(l, d);
  }
  list_circular(l, 5);
  while (!list_empty(l)) {
    d = list_pop_back(l);
    free(d);
  }

  list_delete(&l);

  return 0;
}
