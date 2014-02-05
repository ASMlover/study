/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "global.h"
#include "memory.h"
#include "klang.h"



KL_State* 
KL_create(void)
{
  KL_MemStorage* storage;
  KL_State* L;

  storage = KL_open_storage(0);
  L = (KL_State*)KL_storage_malloc(storage, sizeof(*L));

  L->kl_storage = storage;
  L->exec_storage = NULL;
  L->variable = NULL;
  L->func_list = NULL;
  L->stmt_list = NULL;
  L->lineno = 1;

  KL_set_state(L);

  return L;
}




static void 
release_global_strings(KL_State* L) 
{
  KL_Variable* tmp;
  while (NULL != L->variable) {
    tmp = L->variable;
    L->variable = tmp->next;
    if (VT_STR == tmp->value.val_type)
      KL_release_string(tmp->value.value.str_val);
  }
}

void 
KL_dispose(KL_State* L)
{
  release_global_strings(L);

  if (NULL != L->exec_storage)
    KL_dispose_storage(L->exec_storage);

  KL_dispose_storage(L->kl_storage);
}


void 
KL_compiler(KL_State* L, FILE* fp)
{
  KL_set_state(L);

  //! KL_parse(fp);

  KL_reset_string_literal_buffer();
}

void 
KL_interpret(KL_State* L)
{
  L->exec_storage = KL_open_storage(0);
  //! 
  KL_exec_stmt_list(L, NULL, L->stmt_list);
}
