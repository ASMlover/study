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
#include "vlist.h"

static void 
vlist_test(vlist_t vl) 
{
  int i;
  double* d;

  fprintf(stdout, "vlist testing ...\n");
  srand(time(0));
  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = rand() % 2434 * 0.01256;
    fprintf(stdout, "insert vlist {index=>%d, value=>%lf}\n", i, *d);
    vlist_insert(vl, d);
  }
  fprintf(stdout, "vlist size is : %d\n", vlist_size(vl));
  fprintf(stdout, "vlist get(3) = %lf\n", *(double*)vlist_get(vl, 9));
  while (!vlist_empty(vl)) {
    d = (double*)vlist_remove(vl);
    fprintf(stdout, "the removed element {value=>%lf}\n", *d);
    free(d);
  }
  fprintf(stdout, "vlist size is : %d\n", vlist_size(vl));
}

int 
main(int argc, char* argv[])
{
  vlist_t vl = vlist_create();

  vlist_test(vl);

  vlist_delete(&vl);
  return 0;
}
