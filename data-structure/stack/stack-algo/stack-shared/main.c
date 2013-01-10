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
#include "stack.h"


int 
main(int argc, char* argv[])
{
  int i;
  stack_t s = stack_create(5, 16);

  for (i = 0; i < 5; ++i) {
    fprintf(stdout, "[%d] stack is empty : %d, size is : %d\n", 
      i, stack_empty(s, i), stack_size(s, i));
  }

  srand(time(0));
  for (i = 0; i < 5; ++i) {
    int j;
    for (j = 0; j < 16; ++j)
      stack_push(s, i, j * j * (i + 1));
  }

  for (i = 0; i < 5; ++i) {
    fprintf(stdout, "[%d] stack is empty : %d, size is : %d\n", 
      i, stack_empty(s, i), stack_size(s, i));
  }

  for (i = 0; i < 5; ++i) {
    fprintf(stdout, "\nstack [%d] - top element is : %d\n\t", 
      i, stack_peek(s, i));
    while (!stack_empty(s, i)) {
      element_t e = stack_pop(s, i);
      fprintf(stdout, "%d ", e);
    }
  }


  stack_delete(&s);

  return 0;
}
