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
#include "../src/sl_table.h"




void 
sl_test_table(void)
{
  sl_table_t* table;
  fprintf(stdout, "test table module of slib : <%s>\n", __func__);

  fprintf(stdout, "\ttest sl_table_create\n");
  table = sl_table_create(253);
  ASSERT(NULL != table);
  ASSERT(0 == sl_table_size(table));

  fprintf(stdout, "\ttest sl_table_exsits\n");
  ASSERT(!sl_table_exsits(table, "xxx"));

  fprintf(stdout, "\ttest sl_table_set\n");
  sl_table_set(table, "1", (void*)1, NULL);
  sl_table_set(table, "2", (void*)2, NULL);
  ASSERT(2 == sl_table_size(table));
  ASSERT(sl_table_exsits(table, "1"));
  ASSERT(sl_table_exsits(table, "2"));
  sl_table_set(table, "1", (void*)100, NULL);


  fprintf(stdout, "\ttest sl_table_get\n");
  ASSERT(100 == (int)sl_table_get(table, "1"));
  ASSERT(2 == (int)sl_table_get(table, "2"));

  fprintf(stdout, "\ttest sl_table_remove\n");
  sl_table_remove(table, "2");
  ASSERT(1 == sl_table_size(table));
  ASSERT(100 == (int)sl_table_get(table, "1"));
  sl_table_remove(table, "1");
  ASSERT(0 == sl_table_size(table));

  fprintf(stdout, "\ttest sl_table_clear\n");
  sl_table_set(table, "100", (void*)100, NULL);
  ASSERT(1 == sl_table_size(table));
  sl_table_clear(table);
  ASSERT(0 == sl_table_size(table));

  sl_table_release(table);

  fprintf(stdout, "test table module of slib : all passed\n");
}
