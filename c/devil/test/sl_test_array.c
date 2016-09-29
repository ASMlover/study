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
#include "sl_test_header.h"
#include "../src/sl_array.h"



void 
sl_test_array(void)
{
  sl_array_t* array;
  fprintf(stdout, "test array module of slib : <%s>\n", __func__);


  fprintf(stdout, "\ttest sl_array_create\n");
  array = sl_array_create(128);
  ASSERT(NULL != array);
  ASSERT(0 == sl_array_size(array));
  ASSERT(sl_array_empty(array));
  ASSERT(128 == sl_array_capacity(array));

  fprintf(stdout, "\ttest sl_array_pushback\n");
  sl_array_pushback(array, (void*)100);
  sl_array_pushback(array, (void*)200);
  ASSERT(2 == sl_array_size(array));
  ASSERT(!sl_array_empty(array));

  fprintf(stdout, "\ttest sl_array_popback\n");
  ASSERT(200 == (int)sl_array_popback(array));
  ASSERT(1 == sl_array_size(array));

  fprintf(stdout, "\ttest sl_array_front and sl_array_back\n");
  sl_array_pushback(array, (void*)200);
  sl_array_pushback(array, (void*)300);
  ASSERT(100 == (int)sl_array_front(array));
  ASSERT(300 == (int)sl_array_back(array));
  
  fprintf(stdout, "\ttest sl_array_set\n");
  sl_array_set(array, 0, (void*)1000);
  sl_array_set(array, 4, (void*)400);
  ASSERT(3 == sl_array_size(array));
  ASSERT(1000 == (int)sl_array_front(array));

  fprintf(stdout, "\ttest sl_array_get\n");
  ASSERT(1000 == (int)sl_array_get(array, 0));
  ASSERT(200 == (int)sl_array_get(array, 1));
  ASSERT(300 == (int)sl_array_get(array, 2));

  fprintf(stdout, "\ttest sl_array_clear\n");
  sl_array_clear(array);
  ASSERT(0 == sl_array_size(array));
  ASSERT(sl_array_empty(array));


  sl_array_release(array);
  fprintf(stdout, "test array module of slib : all passed\n");
}
