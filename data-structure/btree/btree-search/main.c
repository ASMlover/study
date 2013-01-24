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
#include "btree.h"


#define ARRAY_N   (20)

static void 
visit_double(int key, element_t val) 
{
  fprintf(stdout, "The element {key=>%d, val=>%lf}\n", key, *(double*)val);
}

static void 
search_btree_test(btree_t bt, int rand_n, double rand_d) 
{
  int i;
  double* d;
  int key_array[ARRAY_N];

  fprintf(stdout, "search binary tree testing ...\n");
  srand(time(0));
  for (i = 0; i < ARRAY_N; ++i) {
    d = (double*)malloc(sizeof(*d));
    key_array[i] = rand() % rand_n;
    *d = key_array[i] * rand_d;
    fprintf(stdout, "insert => {key=>%d, val=>%lf}\n", key_array[i], *d);
    btree_insert(bt, key_array[i], d);
  }
  
  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
  btree_preorder(bt, visit_double);

  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
  btree_inorder(bt, visit_double);

  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
  btree_postorder(bt, visit_double);

  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
  for (i = 0; i < ARRAY_N; ++i) {
    d = btree_remove(bt, key_array[i]);
    fprintf(stdout, "the remove {key=>%d, val=>%lf}\n", key_array[i], *d);
    free(d);
  }
  fprintf(stdout, "\nsearch binary tree size : %d\n", btree_size(bt));
}


int 
main(int argc, char* argv[])
{
  btree_t bt = btree_create();

  search_btree_test(bt, 35466, 0.08982);

  btree_delete(&bt);
  return 0;
}
