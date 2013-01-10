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
#include "stack.h"


static void 
stack_show_information(stack_t s)
{
  fprintf(stdout, "stack size is : %d\n", stack_size(s));
  fprintf(stdout, "stack peek is : %d\n", stack_peek(s));
  fprintf(stdout, "stack max is : %d\n", stack_max(s));
  fprintf(stdout, "stack min is : %d\n\n", stack_min(s));
}

int 
main(int argc, char* argv[])
{
  stack_t s = stack_create();
  int i;

  srand(time(0));
  for (i = 0; i < 10; ++i) {
    int val = rand() % 2324;
    stack_push(s, val);
    fprintf(stdout, "[%d] pushed element value is : %d\n", i, val);
  }

  stack_show_information(s);
  fprintf(stdout, "poped stack value is : %d\n", stack_pop(s));
  stack_show_information(s);
  fprintf(stdout, "poped stack value is : %d\n", stack_pop(s));
  stack_show_information(s);
  fprintf(stdout, "poped stack value is : %d\n", stack_pop(s));
  stack_show_information(s);
  fprintf(stdout, "poped stack value is : %d\n", stack_pop(s));
  stack_show_information(s);
  fprintf(stdout, "poped stack value is : %d\n", stack_pop(s));
  stack_show_information(s);

  stack_delete(&s);

  return 0;
}
