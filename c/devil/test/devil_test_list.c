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
#include "../src/devil_list.h"

void
devil_test_list(void)
{
  devil_list_t* list;
  int i;
  devil_list_iter_t iter;
  fprintf(stdout, "test list module of devil <%s>\n", __func__);

  fprintf(stdout, "\ttest devil_list_create\n");
  list = devil_list_create();
  DEVIL_ASSERT(NULL != list);
  DEVIL_ASSERT(0 == devil_list_size(list));
  DEVIL_ASSERT(devil_list_empty(list));

  fprintf(stdout, "\ttest devil_list_pushback\n");
  for (i = 0; i < 10; ++i)
    devil_list_pushback(list, (void*)i);
  DEVIL_ASSERT(!devil_list_empty(list));
  DEVIL_ASSERT(10 == devil_list_size(list));
  DEVIL_ASSERT(0 == (int)devil_list_front(list));
  DEVIL_ASSERT(9 == (int)devil_list_back(list));

  fprintf(stdout, "\ttest devil_list_pushfront\n");
  for (i = 0; i < 5; ++i)
    devil_list_pushfront(list, (void*)((i + 1) * 100));
  DEVIL_ASSERT(15 == devil_list_size(list));
  DEVIL_ASSERT(500 == (int)devil_list_front(list));
  DEVIL_ASSERT(9 == (int)devil_list_back(list));

  fprintf(stdout, "\ttest devil_list_iter_t\n");
  iter = devil_list_begin(list);
  for (; iter != devil_list_end(list); iter = devil_list_iter_next(iter))
    fprintf(stdout, "\t\tlist item value => %d\n", (int)devil_list_iter_value(iter));

  fprintf(stdout, "\ttest devil_list_popfront\n");
  DEVIL_ASSERT(500 == (int)devil_list_popfront(list));
  DEVIL_ASSERT(14 == devil_list_size(list));

  fprintf(stdout, "\ttest devil_list_popback\n");
  DEVIL_ASSERT(9 == (int)devil_list_popback(list));
  DEVIL_ASSERT(13 == devil_list_size(list));

  fprintf(stdout, "\ttest devil_list_clear\n");
  devil_list_clear(list);
  DEVIL_ASSERT(devil_list_empty(list));
  DEVIL_ASSERT(0 == devil_list_size(list));

  fprintf(stdout, "test list module of devil : all passed\n");
}
