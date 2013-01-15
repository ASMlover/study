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
#include "circular_buffer.h"


static void 
circular_buffer_test(circular_buffer_t cb)
{
  char c = 'a';
  int i;

  fprintf(stdout, "circular buffer size : %d\n", circular_buffer_size(cb));
  for (i = 0; i < 26; ++i) 
    circular_buffer_put(cb, c + i);
  fprintf(stdout, "circular buffer size : %d\n", circular_buffer_size(cb));
  while (!circular_buffer_empty(cb)) {
    c = circular_buffer_get(cb);
    fprintf(stdout, "the put element is : %c\n", c);
  }
  fprintf(stdout, "circular buffer size : %d\n", circular_buffer_size(cb));
}


int 
main(int argc, char* argv[])
{
  circular_buffer_t cb = circular_buffer_create(26);

  circular_buffer_test(cb);

  circular_buffer_delete(&cb);

  return 0;
}
