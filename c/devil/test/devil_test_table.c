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
#include "../src/devil_table.h"

void
devil_test_table(void)
{
  devil_table_t* table;
  fprintf(stdout, "test table module of devil : <%s>\n", __func__);

  fprintf(stdout, "\ttest devil_table_create\n");
  table = devil_table_create(253);
  DEVIL_ASSERT(NULL != table);
  DEVIL_ASSERT(0 == devil_table_size(table));

  fprintf(stdout, "\ttest devil_table_exsits\n");
  DEVIL_ASSERT(!devil_table_exsits(table, "xxx"));

  fprintf(stdout, "\ttest devil_table_set\n");
  devil_table_set(table, "1", (void*)1, NULL);
  devil_table_set(table, "2", (void*)2, NULL);
  DEVIL_ASSERT(2 == devil_table_size(table));
  DEVIL_ASSERT(devil_table_exsits(table, "1"));
  DEVIL_ASSERT(devil_table_exsits(table, "2"));
  devil_table_set(table, "1", (void*)100, NULL);


  fprintf(stdout, "\ttest devil_table_get\n");
  DEVIL_ASSERT(100 == (int)devil_table_get(table, "1"));
  DEVIL_ASSERT(2 == (int)devil_table_get(table, "2"));

  fprintf(stdout, "\ttest devil_table_remove\n");
  devil_table_remove(table, "2");
  DEVIL_ASSERT(1 == devil_table_size(table));
  DEVIL_ASSERT(100 == (int)devil_table_get(table, "1"));
  devil_table_remove(table, "1");
  DEVIL_ASSERT(0 == devil_table_size(table));

  fprintf(stdout, "\ttest devil_table_clear\n");
  devil_table_set(table, "100", (void*)100, NULL);
  DEVIL_ASSERT(1 == devil_table_size(table));
  devil_table_clear(table);
  DEVIL_ASSERT(0 == devil_table_size(table));

  devil_table_release(table);

  fprintf(stdout, "test table module of devil : all passed\n");
}
