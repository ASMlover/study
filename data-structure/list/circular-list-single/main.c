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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "list.h"


static void 
visit_double(element_t e) 
{
  fprintf(stdout, "list element {value=>%lf}\n", *(double*)e);
}

static void 
list_test(list_t l, int rand_n, double rand_d) 
{
  int i;
  double* d;

  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n * rand_d;
    fprintf(stdout, "list append {index=>%d, value=>%lf}\n", i, *d);
    list_append(l, d);
  }
  fprintf(stdout, "circulr list size : %d\n", list_size(l));
  list_traverse(l, visit_double);

  while (!list_empty(l)) {
    d = (double*)list_remove(l);
    fprintf(stdout, "removed element {value=>%lf}\n", *d);
    free(d);
  }
  fprintf(stdout, "circulr list size : %d\n", list_size(l));
}


int 
main(int argc, char* argv[])
{
  list_t l = list_create();

  list_test(l, 3546, 0.07801);

  list_delete(&l);

  return 0;
}
