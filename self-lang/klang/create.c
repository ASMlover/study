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

  if (NULL == list)
    return KL_create_param(identifier);
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

  if (NULL == list)
    return KL_create_arg_list(expr);
  for (arg = list; NULL != arg->next; arg = arg->next) {
  }
  arg->next = KL_create_arg_list(expr);

  return list;
}

KL_StmtList* 
KL_create_stmt_list(KL_Stmt* stmt)
{
  KL_StmtList* stmt_list = (KL_StmtList*)KL_util_malloc(sizeof(*stmt_list));
  stmt_list->stmt = stmt;
  stmt_list->next = NULL;

  return stmt_list;
}

KL_StmtList* 
KL_chain_stmt_list(KL_StmtList* list, KL_Stmt* stmt)
{
  KL_StmtList* stmt_list;

  if (NULL == list)
    return KL_create_stmt_list(stmt);

  for (stmt_list = list; 
      NULL != stmt_list->next; 
      stmt_list = stmt_list->next) {
  }
  stmt_list->next = KL_create_stmt_list(stmt);

  return list;
}

KL_Expr* 
KL_alloc_expr(int expr_type)
{
  KL_Expr* expr = (KL_Expr*)KL_util_malloc(sizeof(*expr));
  expr->expr_type = expr_type;
  expr->lineno = KL_get_state()->lineno;

  return expr;
}

KL_Expr* 
KL_create_assign_expr(const char* variable, KL_Expr* operand)
{
  KL_Expr* expr = KL_alloc_expr(ET_ASSIGN);
  expr->expr.assign_expr.variable = (char*)variable;
  expr->expr.assign_expr.operand = operand;

  return expr;
}



static KL_Expr 
convert_valie_to_expr(KL_Value* value)
{
  KL_Expr expr;

  if (VT_INT == value->val_type) {
    expr.expr_type = ET_INT;
    expr.expr.int_val = value->value.int_val;
  }
  else if (VT_REAL == value->val_type) {
    expr.expr_type = ET_REAL;
    expr.expr.real_val = value->value.real_val;
  }
  else {
    expr.expr_type = ET_BOOL;
    expr.expr.bool_val = value->value.bool_val;
  }

  return expr;
}

KL_Expr* 
KL_create_bin_expr(int expr_type, KL_Expr* left, KL_Expr* right)
{
  if ((ET_INT == left->expr_type || ET_REAL == left->expr_type)
      && (ET_INT == right->expr_type || ET_REAL == right->expr_type)) {
    KL_Value v = {0};

    //! TODO:
    //! v = KL_eval_bin_expr(KL_get_state(), NULL, expr_type, left, right);

    *left = convert_valie_to_expr(&v);

    return left;
  }
  else {
    KL_Expr* expr = KL_alloc_expr(expr_type);
    expr->expr.binary_expr.left = left;
    expr->expr.binary_expr.right = right;

    return expr;
  }
}

KL_Expr* 
KL_create_minus_expr(KL_Expr* operand)
{
  if (ET_INT == operand->expr_type || ET_REAL == operand->expr_type) {
    KL_Value v = {0};

    //! TODO:
    //! v = KL_eval_minus_expr(KL_get_state(), NULL, operand);
    
    *operand = convert_valie_to_expr(&v);

    return operand;
  }
  else {
    KL_Expr* expr = KL_alloc_expr(ET_MINUS);
    expr->expr.minus_expr = operand;

    return expr;
  }
}

KL_Expr* 
KL_create_identifier_expr(const char* identifier) 
{
  KL_Expr* expr = KL_alloc_expr(ET_ID);
  expr->expr.identifier = (char*)identifier;

  return expr;
}

KL_Expr* 
KL_create_func_call_expr(const char* func_name, KL_ArgList* argument)
{
  KL_Expr* expr = KL_alloc_expr(ET_FUNC_CALL);
  expr->expr.func_call_expr.identifier = (char*)func_name;
  expr->expr.func_call_expr.argument = argument;

  return expr;
}

KL_Expr* 
KL_create_boolean_expr(KL_Boolean value)
{
  KL_Expr* expr = KL_alloc_expr(ET_BOOL);
  expr->expr.bool_val = value;

  return expr;
}

KL_Expr* 
KL_create_nil_expr(void)
{
  KL_Expr* expr = KL_alloc_expr(ET_NIL);
  return expr;
}



static KL_Stmt* 
alloc_stmt(int stmt_type)
{
  KL_Stmt* stmt = (KL_Stmt*)KL_util_malloc(sizeof(*stmt));
  stmt->stmt_type = stmt_type;
  stmt->lineno = KL_get_state()->lineno;

  return stmt;
}

KL_Stmt* 
KL_create_global_stmt(KL_IDList* id_list)
{
  KL_Stmt* stmt = alloc_stmt(ST_GLOBAL);
  stmt->stmt.global_s.id_list = id_list;

  return stmt;
}

KL_IDList* 
KL_create_global_identifier(const char* identifier)
{
  KL_IDList* id_list = (KL_IDList*)KL_util_malloc(sizeof(*id_list));
  id_list->name = (char*)identifier;
  id_list->next = NULL;

  return id_list;
}

KL_IDList* 
KL_chain_identifier(KL_IDList* list, const char* identifier)
{
  KL_IDList* id_list;

  if (NULL == list)
    return KL_create_global_identifier(identifier);
  for (id_list = list; NULL != id_list->next; id_list = id_list->next) {
  }
  id_list->next = KL_create_global_identifier(identifier);

  return list;
}


KL_Stmt* 
KL_create_if_stmt(KL_Expr* cond, KL_Block* then_block, KL_Block* else_block)
{
  KL_Stmt* stmt = alloc_stmt(ST_IF);
  stmt->stmt.if_s.cond = cond;
  stmt->stmt.if_s.then_block = then_block;
  stmt->stmt.if_s.else_block = else_block;

  return stmt;
}

KL_Stmt* 
KL_create_while_stmt(KL_Expr* cond, KL_Block* block)
{
  KL_Stmt* stmt = alloc_stmt(ST_WHILE);
  stmt->stmt.while_s.cond = cond;
  stmt->stmt.while_s.block = block;

  return stmt;
}

KL_Block* 
KL_create_block(KL_StmtList* stmt_list)
{
  KL_Block* block = (KL_Block*)KL_util_malloc(sizeof(*block));
  block->stmt_list = stmt_list;

  return block;
}

KL_Stmt* 
KL_create_expr_stmt(KL_Expr* expr)
{
  KL_Stmt* stmt = alloc_stmt(ST_EXPR);
  stmt->stmt.expr_s = expr;

  return stmt;
}

KL_Stmt* 
KL_create_return_stmt(KL_Expr* expr)
{
  KL_Stmt* stmt = alloc_stmt(ST_RET);
  stmt->stmt.ret_s.ret_expr = expr;

  return stmt;
}

KL_Stmt* 
KL_create_break_stmt(void)
{
  return alloc_stmt(ST_BREAK);
}
