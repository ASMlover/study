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
#include "../src/sl_list.h"





void 
sl_test_list(void)
{
  sl_list_t* list;
  int i;
  sl_list_iter_t* iter;
  fprintf(stdout, "test list module of slib <%s>\n", __func__);


  fprintf(stdout, "\ttest sl_list_create\n");
  list = sl_list_create();
  ASSERT(NULL != list);
  ASSERT(0 == sl_list_size(list));
  ASSERT(sl_list_empty(list));

  fprintf(stdout, "\ttest sl_list_pushback\n");
  for (i = 0; i < 10; ++i) 
    sl_list_pushback(list, (void*)i);
  ASSERT(!sl_list_empty(list));
  ASSERT(10 == sl_list_size(list));
  ASSERT(0 == (int)sl_list_front(list));
  ASSERT(9 == (int)sl_list_back(list));

  fprintf(stdout, "\ttest sl_list_pushfront\n");
  for (i = 0; i < 5; ++i)
    sl_list_pushfront(list, (void*)((i + 1) * 100));
  ASSERT(15 == sl_list_size(list));
  ASSERT(500 == (int)sl_list_front(list));
  ASSERT(9 == (int)sl_list_back(list));

  fprintf(stdout, "\ttest sl_list_iter_t\n");
  iter = sl_list_begin(list);
  for (; iter != sl_list_end(list); iter = sl_list_iter_next(iter))
    fprintf(stdout, "\t\tlist item value => %d\n", 
        (int)sl_list_iter_value(iter));

  fprintf(stdout, "\ttest sl_list_popfront\n");
  ASSERT(500 == (int)sl_list_popfront(list));
  ASSERT(14 == sl_list_size(list));

  fprintf(stdout, "\ttest sl_list_popback\n");
  ASSERT(9 == (int)sl_list_popback(list));
  ASSERT(13 == sl_list_size(list));


  fprintf(stdout, "\ttest sl_list_clear\n");
  sl_list_clear(list);
  ASSERT(sl_list_empty(list));
  ASSERT(0 == sl_list_size(list));
  
  
  fprintf(stdout, "test list module of slib : all passed\n");
}
