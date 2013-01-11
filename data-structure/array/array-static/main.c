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
#include "array.h"


static void 
visit_double(element_t e) 
{
  fprintf(stdout, "the array element value is : %lf\n", *(double*)e);
}

static void 
array_test(array_t a, 
  void (*set)(array_t, int, element_t), 
  element_t (*get)(array_t, int), 
  int rand_n, 
  double rand_d)
{
  int i;
  double* d;
  double** dd;

  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % rand_n *  rand_d;
    set(a, i, d);

    fprintf(stdout, "set array element {i=>%d, v=>%lf}\n", i, *d);
  }
  fprintf(stdout, "array {size=>%d, empty=>%d}\n", 
    array_size(a), array_empty(a));

  fprintf(stdout, "\nshow array elements : \n");
  array_traverse(a, visit_double);

  fprintf(stdout, "\nshow array elements : \n");
  dd = (double**)array_data(a);
  for (i = 0; i < 10; ++i) 
    fprintf(stdout, "array element is {i=>%d, v=>%lf}\n", i, *dd[i]);

  fprintf(stdout, "\nshow array elements : \n");
  for (i = 0; i < 10; ++i) {
    d = (double*)array_get(a, i);
    fprintf(stdout, "array element is {i=>%d, v=>%lf}\n", i, *d);
    free(d);
  }
}


int 
main(int argc, char* argv[])
{
  array_t a = array_create(10);

  array_test(a, array_set, array_get, 45463, 0.98792);

  array_delete(&a);
  return 0;
}
