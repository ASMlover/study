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


static KL_State* s_current_state;

KL_State* 
KL_get_state(void)
{
  return s_current_state;
}

void 
KL_set_state(KL_State* L)
{
  s_current_state = L;
}


KL_Function* 
KL_lookup_func(const char* name)
{
  KL_Function* func = NULL;
  KL_State* L = KL_get_state();

  for (func = L->func_list; NULL != func; func = func->next) {
    if (0 == strcmp(func->name, name))
      break;
  }

  return func;
}


void* 
KL_util_malloc(size_t size)
{
  KL_State* L = KL_get_state();
  void* ptr = KL_storage_malloc(L->kl_storage, size);

  return ptr;
}

void* 
KL_util_exec_malloc(KL_State* L, size_t size)
{
  void* ptr = KL_storage_malloc(L->exec_storage, size);

  return ptr;
}

KL_Variable* 
KL_lookup_local_variable(KL_LocalEnv* env, const char* identifier)
{
  KL_Variable* var;

  if (NULL == env)
    return NULL;

  for (var = env->variable; NULL != var; var = var->next) {
    if (0 == strcmp(var->name, identifier))
      return var;
  }

  return NULL;
}

KL_Variable* 
KL_lookup_global_variable(KL_State* L, const char* identifier)
{
  KL_Variable* var;

  for (var = L->variable; NULL != var; var = var->next) {
    if (0 == strcmp(var->name, identifier))
      return var;
  }

  return NULL;
}

void 
KL_add_local_variable(KL_LocalEnv* env, 
    const char* identifier, KL_Value* value)
{
  KL_Variable* new_variable 
    = (KL_Variable*)KL_malloc(sizeof(*new_variable));
  new_variable->name = (char*)identifier;
  new_variable->value = *value;
  new_variable->next = env->variable;
  env->variable = new_variable;
}

void 
KL_add_global_variable(KL_State* L, 
    const char* identifier, KL_Value* value)
{
  KL_Variable* new_variable
    = (KL_Variable*)KL_util_exec_malloc(L, sizeof(*new_variable));
  new_variable->name = (char*)KL_util_exec_malloc(L, 
      strlen(identifier) + 1);
  strcpy(new_variable->name, identifier);
  new_variable->next = L->variable;
  L->variable = new_variable;
  new_variable->value = *value;
}

char* 
KL_get_oper_string(int expr_type)
{
  char* str = "";

  switch (expr_type) {
  case ET_BOOL:
  case ET_INT:
  case ET_REAL:
  case ET_STR:
  case ET_ID:
    //! bad expression type:
    break;
  case ET_ASSIGN:
    str = "=";
    break;
  case ET_ADD:
    str = "+";
    break;
  case ET_SUB:
    str = "-";
    break;
  case ET_MUL:
    str = "*";
    break;
  case ET_DIV:
    str = "/";
    break;
  case ET_MOD:
    str = "%";
    break;
  case ET_EQ:
    str = "==";
    break;
  case ET_NEQ:
    str = "<>";
    break;
  case ET_GT:
    str = ">";
    break;
  case ET_GE:
    str = ">=";
    break;
  case ET_LT:
    str = "<";
    break;
  case ET_LE:
    str = "<=";
    break;
  case ET_AND:
    str = "&&";
    break;
  case ET_OR:
    str = "||";
    break;
  case ET_MINUS:
    str = "-";
    break;
  case ET_FUNC_CALL:
  case ET_NIL:
  default:
    //! bad expression
    break;
  }

  return str;
}
