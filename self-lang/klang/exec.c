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



static KL_StmtResult exec_stmt(KL_State* L, 
    KL_LocalEnv* env, KL_Stmt* stmt);


static KL_StmtResult 
exec_expr_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_StmtResult result;
  KL_Value value;

  result.type = SRT_NORMAL;

  value = KL_eval_expr(L, env, stmt->stmt.expr_s);
  if (VT_STR == value.val_type)
    KL_release_string(value.value.str_val);

  return result;
}

static KL_StmtResult 
exec_global_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_IDList* id_list;
  KL_StmtResult result;

  result.type = SRT_NORMAL;

  if (NULL == env) {
    //! runtime error
  }

  for (id_list = stmt->stmt.global_s.id_list; 
      NULL != id_list; id_list = id_list->next) {
    KL_GlobalVariableRef* ref;
    KL_GlobalVariableRef* new_ref;
    KL_Variable* variable;

    for (ref = env->global_variable; NULL != ref; ref = ref->next) {
      if (0 == strcmp(ref->variable->name, id_list->name))
        goto NEXT_IDENTIFIER;
    }

    variable = KL_lookup_global_variable(L, id_list->name);
    if (NULL == variable) {
      //! runtime error
    }

    new_ref = (KL_GlobalVariableRef*)KL_malloc(sizeof(*new_ref));
    new_ref->variable = variable;
    new_ref->next = env->global_variable;
    env->global_variable = new_ref;

NEXT_IDENTIFIER:
    {
    }
  }

  return result;
}

static KL_StmtResult 
exec_if_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_StmtResult result;
  KL_Value cond;

  result.type = SRT_NORMAL;
  cond = KL_eval_expr(L, env, stmt->stmt.if_s.cond);
  if (VT_BOOL != cond.val_type) {
    //! runtime error
  }

  if (cond.value.bool_val) {
    result = KL_exec_stmt_list(L, env, 
        stmt->stmt.if_s.then_block->stmt_list);
  }
  else {
    result = KL_exec_stmt_list(L, env, 
        stmt->stmt.if_s.else_block->stmt_list);
  }

  return result;
}

static KL_StmtResult 
exec_while_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_StmtResult result;
  KL_Value cond;

  result.type = SRT_NORMAL;
  for (;;) {
    cond = KL_eval_expr(L, env, stmt->stmt.while_s.cond);
    if (VT_BOOL != cond.val_type) {
      //! runtime error
    }

    if (!cond.value.bool_val)
      break;

    result = KL_exec_stmt_list(L, env, 
        stmt->stmt.while_s.block->stmt_list);

    if (SRT_RETURN == result.type) {
      break;
    }
    else if (SRT_BREAK == result.type) {
      result.type = SRT_NORMAL;
      break;
    }
  }

  return result;
}

static KL_StmtResult 
exec_return_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_StmtResult result;

  result.type = SRT_RETURN;
  if (stmt->stmt.ret_s.ret_expr) {
    result.stmt_result.value 
      = KL_eval_expr(L, env, stmt->stmt.ret_s.ret_expr);
  }
  else {
    result.stmt_result.value.val_type = VT_NIL;
  }

  return result;
}

static KL_StmtResult 
exec_break_stmt(KL_State* L, KL_LocalEnv* env, KL_Stmt* stmt)
{
  KL_StmtResult result;
  result.type = SRT_BREAK;

  return result;
}
