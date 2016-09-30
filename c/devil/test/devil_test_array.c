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
#include "devil_test.h"
#include "../src/devil_array.h"

void
devil_test_array(void)
{
  devil_array_t* array;
  fprintf(stdout, "test array module of slib : <%s>\n", __func__);


  fprintf(stdout, "\ttest devil_array_create\n");
  array = devil_array_create(128);
  DEVIL_ASSERT(NULL != array);
  DEVIL_ASSERT(0 == devil_array_size(array));
  DEVIL_ASSERT(devil_array_empty(array));
  DEVIL_ASSERT(128 == devil_array_capacity(array));

  fprintf(stdout, "\ttest devil_array_pushback\n");
  devil_array_pushback(array, (void*)100);
  devil_array_pushback(array, (void*)200);
  DEVIL_ASSERT(2 == devil_array_size(array));
  DEVIL_ASSERT(!devil_array_empty(array));

  fprintf(stdout, "\ttest devil_array_popback\n");
  DEVIL_ASSERT(200 == (int)devil_array_popback(array));
  DEVIL_ASSERT(1 == devil_array_size(array));

  fprintf(stdout, "\ttest devil_array_front and devil_array_back\n");
  devil_array_pushback(array, (void*)200);
  devil_array_pushback(array, (void*)300);
  DEVIL_ASSERT(100 == (int)devil_array_front(array));
  DEVIL_ASSERT(300 == (int)devil_array_back(array));

  fprintf(stdout, "\ttest devil_array_set\n");
  devil_array_set(array, 0, (void*)1000);
  devil_array_set(array, 4, (void*)400);
  DEVIL_ASSERT(3 == devil_array_size(array));
  DEVIL_ASSERT(1000 == (int)devil_array_front(array));

  fprintf(stdout, "\ttest devil_array_get\n");
  DEVIL_ASSERT(1000 == (int)devil_array_get(array, 0));
  DEVIL_ASSERT(200 == (int)devil_array_get(array, 1));
  DEVIL_ASSERT(300 == (int)devil_array_get(array, 2));

  fprintf(stdout, "\ttest devil_array_clear\n");
  devil_array_clear(array);
  DEVIL_ASSERT(0 == devil_array_size(array));
  DEVIL_ASSERT(devil_array_empty(array));

  devil_array_release(array);
  fprintf(stdout, "test array module of slib : all passed\n");
}
