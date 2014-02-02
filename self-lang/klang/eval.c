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



static KL_Value 
eval_boolean_expr(KL_Boolean bool_val)
{
  KL_Value v;
  v.val_type = VT_BOOL;
  v.value.bool_val = bool_val;

  return v;
}

static KL_Value 
eval_int_expr(int int_val)
{
  KL_Value v;
  v.val_type = VT_INT;
  v.value.int_val = int_val;

  return v;
}

static KL_Value 
eval_real_expr(double real_val)
{
  KL_Value v;
  v.val_type = VT_REAL;
  v.value.real_val = real_val;

  return v;
}

static KL_Value 
eval_str_expr(KL_State* L, const char* str_val)
{
  KL_Value v;
  v.val_type = VT_STR;
  v.value.str_val = KL_literal_to_string(L, str_val);

  return v;
}

static KL_Value 
eval_nil_expr(void)
{
  KL_Value v;
  v.val_type = VT_NIL;

  return v;
}

static void 
refer_if_string(KL_Value* v)
{
  if (VT_STR == v->val_type)
    KL_refer_string(v->value.str_val);
}

static void 
release_if_string(KL_Value* v)
{
  if (VT_STR == v->val_type)
    KL_release_string(v->value.str_val);
}

static KL_Variable* 
lookup_global_variable_from_env(KL_State* L, 
    KL_LocalEnv* env, const char* name)
{
  KL_GlobalVariableRef* global;

  if (NULL == env)
    return NULL;

  for (global = env->global_variable; 
      NULL != global->next; global = global->next) {
    if (0 == strcmp(global->variable->name, name))
      return global->variable;
  }

  return NULL;
}

static KL_Value 
eval_identifier_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr)
{
  KL_Value val;
  KL_Variable* var;

  var = KL_lookup_local_variable(env, expr->expr.identifier);
  if (NULL != var) {
    val = var->value;
  }
  else {
    var = lookup_global_variable_from_env(L, env, expr->expr.identifier);
    if (NULL != var) {
      val = var->value;
    }
    else {
      //! error
    }
  }
  refer_if_string(&val);

  return val;
}

static KL_Value eval_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr);

static KL_Value 
eval_assign_expr(KL_State* L, 
    KL_LocalEnv* env, const char* identifier, KL_Expr* expr)
{
  KL_Value val;
  KL_Variable* left;

  val = eval_expr(L, env, expr);
  left = KL_lookup_local_variable(env, identifier);

  if (NULL == left) {
    left = lookup_global_variable_from_env(L, env, identifier);
  }

  if (NULL != left) {
    release_if_string(&left->value);
    left->value = val;
    release_if_string(&val);
  }
  else {
    if (NULL != env)
      KL_add_local_variable(env, identifier, &val);
    else 
      KL_add_global_variable(L, identifier, &val);

    refer_if_string(&val);
  }

  return val;
}

static KL_Boolean 
eval_bin_boolean(KL_State* L, int expr_type, 
    KL_Boolean left, KL_Boolean right, int lineno)
{
  KL_Boolean result;

  if (ET_EQ == expr_type) {
    result = (left == right);
  }
  else if (ET_NEQ == expr_type) {
    result = (left != right);
  }
  else {
    //! char* op_str = KL_get_oper_string(expr_type);
    //! runtime error 
  }

  return result;
}


static void 
eval_bin_int(KL_State* L, int expr_type, 
    int left, int right, KL_Value* result, int lineno)
{
  if (KL_is_math_oper(expr_type)) {
    result->val_type = VT_INT;
  }
  else if (KL_is_compare_oper(expr_type)) {
    result->val_type = VT_BOOL;
  }
  else {
    //! error
  }

  switch (expr_type) {
  case ET_BOOL:
  case ET_INT:
  case ET_REAL:
  case ET_STR:
  case ET_ID:
  case ET_ASSIGN:
    //! panic 
    break;
  case ET_ADD:
    result->value.int_val = left + right;
    break;
  case ET_SUB:
    result->value.int_val = left - right;
    break;
  case ET_MUL:
    result->value.int_val = left * right;
    break;
  case ET_DIV:
    result->value.int_val = left / right;
    break;
  case ET_MOD:
    result->value.int_val = left % right;
    break;
  case ET_AND:
  case ET_OR:
    //! panic
    break;
  case ET_EQ:
    result->value.bool_val = (left == right);
    break;
  case ET_NEQ:
    result->value.bool_val = (left != right);
    break;
  case ET_GT:
    result->value.bool_val = (left > right);
    break;
  case ET_GE:
    result->value.bool_val = (left >= right);
    break;
  case ET_LT:
    result->value.bool_val = (left < right);
    break;
  case ET_LE:
    result->value.bool_val = (left <= right);
    break;
  case ET_MINUS:
  case ET_FUNC_CALL:
  case ET_NIL:
  default:
    //! panic
    break;
  }
}

static void 
eval_bin_real(KL_State* L, int expr_type, 
    double left, double right, KL_Value* result, int lineno)
{
  if (KL_is_math_oper(expr_type)) {
    result->val_type = VT_REAL;
  }
  else if (KL_is_compare_oper(expr_type)) {
    result->val_type = VT_BOOL;
  }
  else {
    //! panic
  }

  switch (expr_type) {
  case ET_BOOL:
  case ET_INT:
  case ET_REAL:
  case ET_STR:
  case ET_ID:
  case ET_ASSIGN:
    //! panic
    break;
  case ET_ADD:
    result->value.real_val = left + right;
    break;
  case ET_SUB:
    result->value.real_val = left - right;
    break;
  case ET_MUL:
    result->value.real_val = left * right;
    break;
  case ET_DIV:
    result->value.real_val = left / right;
    break;
  case ET_MOD:
    result->value.real_val = fmod(left, right);
    break;
  case ET_AND:
  case ET_OR:
    //! panic
    break;
  case ET_EQ:
    result->value.int_val = (left == right);
    break;
  case ET_NEQ:
    result->value.int_val = (left != right);
    break;
  case ET_GT:
    result->value.int_val = (left > right);
    break;
  case ET_GE:
    result->value.int_val = (left >= right);
    break;
  case ET_LT:
    result->value.int_val = (left < right);
    break;
  case ET_LE:
    result->value.int_val = (left <= right);
    break;
  case ET_MINUS:
  case ET_FUNC_CALL:
  case ET_NIL:
  default:
    //! panic
    break;
  }
}

static KL_Boolean 
eval_compare_string(int expr_type, 
    KL_Value* left, KL_Value* right, int lineno)
{
  KL_Boolean result;
  int cmp = strcmp(left->value.str_val->string, 
      right->value.str_val->string);

  if (ET_EQ == expr_type) {
    result = (cmp == 0);
  }
  else if (ET_NEQ == expr_type) {
    result = (cmp != 0);
  }
  else if (ET_GT == expr_type) {
    result = (cmp > 0);
  }
  else if (ET_GE == expr_type) {
    result = (cmp >= 0);
  }
  else if (ET_LT == expr_type) {
    result = (cmp < 0);
  }
  else if (ET_LE == expr_type) {
    result = (cmp <= 0);
  }
  else {
    //! TODO:
    //! char* op_str = KL_get_oper_string(expr_type);
    //! KL_runtime_error
  }

  KL_release_string(left->value.str_val);
  KL_release_string(right->value.str_val);

  return result;
}

static KL_Boolean 
eval_bin_nil(KL_State* L, int expr_type, 
    KL_Value* left, KL_Value* right, int lineno)
{
  KL_Boolean result;

  if (ET_EQ == expr_type) {
    result = (VT_NIL == left->val_type && VT_NIL == right->val_type);
  }
  else if (ET_NEQ == expr_type) {
    result = !(VT_NIL == left->val_type && VT_NIL == right->val_type);
  }
  else {
    //! TODO:
    //! char* op_str = KL_get_oper_string(expr_type);
    //! runtime error
  }

  release_if_string(left);
  release_if_string(right);

  return result;
}

KL_Value 
KL_eval_bin_expr(KL_State* L, KL_LocalEnv* env, 
    int expr_type, KL_Expr* left, KL_Expr* right)
{
  KL_Value left_val;
  KL_Value right_val;
  KL_Value result;
}
