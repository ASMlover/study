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


void 
KL_function_def(const char* identifier, 
    KL_ParamList* param_list, KL_Block* block)
{
  KL_Function* func;
  KL_State* L;

  if (KL_lookup_func(identifier)) 
    return;

  L = KL_get_state();

  func = KL_util_malloc(sizeof(*func));
  func->name = (char*)identifier;
  func->func_type = FT_DEFINE;
  func->func.define.param = param_list;
  func->func.define.block = block;
  func->next = L->func_list;
  L->func_list = func;
}

KL_ParamList* 
KL_create_param(const char* identifier)
{
  KL_ParamList* p = (KL_ParamList*)KL_util_malloc(sizeof(*p));
  p->name = (char*)identifier;
  p->next = NULL;

  return p;
}

KL_ParamList* 
KL_chain_param(KL_ParamList* list, const char* identifier)
{
  KL_ParamList* param;

  for (param = list; NULL != param->next; param = param->next) {
  }
  param->next = KL_create_param(identifier);

  return list;
}

KL_ArgList* 
KL_create_arg_list(KL_Expr* expr) 
{
  KL_ArgList* arg = (KL_ArgList*)KL_util_malloc(sizeof(*arg));
  arg->expr = expr;
  arg->next = NULL;

  return arg;
}

KL_ArgList* 
KL_chain_arg_list(KL_ArgList* list, KL_Expr* expr)
{
  KL_ArgList* arg;

  for (arg = list; NULL != arg->next; arg = arg->next) {
  }
  arg->next = KL_create_arg_list(expr);

  return list;
}
