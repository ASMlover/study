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
  KL_Boolean result = BOOL_NO;

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
  KL_Boolean result = BOOL_NO;

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
  KL_Boolean result = BOOL_NO;

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


static KL_String* 
chain_string(KL_State* L, KL_String* left, KL_String* right)
{
  char* str;
  KL_String* ret;

  int len = (int)strlen(left->string) + (int)strlen(right->string);
  str = (char*)KL_malloc(len + 1);
  sprintf(str, "%s%s", left->string, right->string);
  ret = KL_create_string(L, str);
  KL_release_string(left);
  KL_release_string(right);

  return ret;
}

KL_Value 
KL_eval_bin_expr(KL_State* L, KL_LocalEnv* env, 
    int expr_type, KL_Expr* left, KL_Expr* right)
{
  KL_Value left_val;
  KL_Value right_val;
  KL_Value result;

  left_val = eval_expr(L, env, left);
  right_val = eval_expr(L, env, right);

  if (VT_INT == left_val.val_type && VT_INT == right_val.val_type) {
    eval_bin_int(L, expr_type, left_val.value.int_val, 
        right_val.value.int_val, &result, left->lineno);
  }
  else if (VT_REAL == left_val.val_type && VT_REAL == right_val.val_type) {
    eval_bin_real(L, expr_type, left_val.value.real_val, 
        right_val.value.real_val, &result, left->lineno);
  }
  else if (VT_INT == left_val.val_type && VT_REAL == right_val.val_type) {
    left_val.value.real_val = left_val.value.int_val;
    eval_bin_real(L, expr_type, left_val.value.real_val, 
        right_val.value.real_val, &result, left->lineno);
  }
  else if (VT_REAL == left_val.val_type && VT_INT == right_val.val_type) {
    right_val.value.real_val = right_val.value.int_val;
    eval_bin_real(L, expr_type, left_val.value.real_val, 
        right_val.value.real_val, &result, left->lineno);
  }
  else if (VT_BOOL == left_val.val_type && VT_BOOL == right_val.val_type) {
    result.val_type = VT_BOOL;
    result.value.bool_val = eval_bin_boolean(L, expr_type, 
          left_val.value.bool_val, right_val.value.bool_val, left->lineno);
  }
  else if (VT_STR == left_val.val_type && ET_ADD == expr_type) {
    char buf[KL_BUFSIZ];
    KL_String* right_str = NULL;

    if (VT_INT == right_val.val_type) {
      sprintf(buf, "%d", right_val.value.int_val);
      right_str = KL_create_string(L, KL_strdup(buf));
    }
    else if (VT_REAL == right_val.val_type) {
      sprintf(buf, "%f",right_val.value.real_val);
      right_str = KL_create_string(L, KL_strdup(buf));
    }
    else if (VT_BOOL == right_val.val_type) {
      if (right_val.value.bool_val) 
        right_str = KL_create_string(L, KL_strdup("true"));
      else 
        right_str = KL_create_string(L, KL_strdup("false"));
    }
    else if (VT_STR == right_val.val_type) {
      right_str = right_val.value.str_val;
    }
    else if (VT_NIL == right_val.val_type) {
      right_str = KL_create_string(L, KL_strdup("nil"));
    }

    result.val_type = VT_STR;
    result.value.str_val = chain_string(L, 
        left_val.value.str_val, right_str);
  }
  else if (VT_STR == left_val.val_type && VT_STR == right_val.val_type) {
    result.val_type = VT_BOOL;
    result.value.bool_val 
      = eval_compare_string(expr_type, &left_val, &right_val, left->lineno);
  }
  else if (VT_NIL == left_val.val_type || VT_NIL == right_val.val_type) {
    result.val_type = VT_BOOL;
    result.value.bool_val 
      = eval_bin_nil(L, expr_type, &left_val, &right_val, left->lineno);
  }
  else {
    //! TODO:
    //! char* op_str = KL_get_oper_string(expr_type);
    //! runtime error 
  }

  return result;
}


static KL_Value 
eval_logical_expr(KL_State* L, 
    KL_LocalEnv* env, int expr_type, KL_Expr* left, KL_Expr* right)
{
  KL_Value left_val;
  KL_Value right_val;
  KL_Value result;

  result.val_type = VT_BOOL;
  left_val = eval_expr(L, env, left);

  if (VT_BOOL != left_val.val_type) {
    //! runtime error
  }

  if (ET_AND == expr_type) {
    if (!left_val.value.bool_val) {
      result.value.bool_val = BOOL_NO;
      return result;
    }
  }
  else if (ET_OR == expr_type) {
    if (left_val.value.bool_val) {
      result.value.bool_val = BOOL_YES;
      return result;
    }
  }
  else {
    //! panic
  }

  right_val = eval_expr(L, env, right);

  if (VT_BOOL != right_val.val_type) {
    //! runtime error
  }
  result.value.bool_val = right_val.value.bool_val;

  return result;
}

KL_Value 
KL_eval_minus_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* operand)
{
  KL_Value operand_val;
  KL_Value result;

  operand_val = eval_expr(L, env, operand);
  if (VT_INT == operand_val.val_type) {
    result.val_type = VT_INT;
    result.value.int_val = -operand_val.value.int_val;
  }
  else if (VT_BOOL == operand_val.val_type) {
    result.val_type = VT_REAL;
    result.value.real_val = -operand_val.value.real_val;
  }
  else {
    //! runtime error
  }

  return result;
}


static KL_LocalEnv* 
alloc_local_env(void)
{
  KL_LocalEnv* ret = (KL_LocalEnv*)KL_malloc(sizeof(*ret));
  ret->variable = NULL;
  ret->global_variable = NULL;

  return ret;
}

static void 
dispose_local_env(KL_State* L, KL_LocalEnv* env)
{
  KL_Variable* tmp;
  KL_GlobalVariableRef* ref;
  while (NULL != env->variable) {
    tmp = env->variable;
    if (VT_STR == env->variable->value.val_type)
      KL_release_string(env->variable->value.value.str_val);

    env->variable = tmp->next;
    KL_free(tmp);
  }

  while (NULL != env->global_variable) {
    ref = env->global_variable;
    env->global_variable = ref->next;
    KL_free(ref);
  }

  KL_free(env);
}

static KL_Value 
call_native_func(KL_State* L, 
    KL_LocalEnv* env, KL_Expr* expr, KL_NativeFuncType proc)
{
  KL_Value    value;
  int         nargs;
  KL_ArgList* arg_list;
  KL_Value*   args;
  int         i;

  for (nargs = 0, arg_list = expr->expr.func_call_expr.argument;
      NULL != arg_list; arg_list = arg_list->next) 
    ++nargs;

  args = (KL_Value*)KL_malloc(sizeof(*args));

  for (arg_list = expr->expr.func_call_expr.argument, i = 0; 
      NULL != arg_list; arg_list = arg_list->next, ++i)
    args[i] = eval_expr(L, env, arg_list->expr);
  value = proc(L, nargs, args);

  for (i = 0; i < nargs; ++i) 
    release_if_string(&args[i]);
  KL_free(args);

  return value;
}

static KL_Value 
call_defined_func(KL_State* L, 
    KL_LocalEnv* env, KL_Expr* expr, KL_Function* func)
{
  KL_Value      value;
  KL_StmtResult result;
  KL_ArgList*   arg_list;
  KL_ParamList* param_list;
  KL_LocalEnv*  local_env;

  local_env = alloc_local_env();

  for (arg_list = expr->expr.func_call_expr.argument, 
      param_list = func->func.define.param; 
      NULL != arg_list; 
      arg_list = arg_list->next, param_list = param_list->next) {
    KL_Value arg_val;

    if (NULL == param_list) {
      //! TODO:
      //! runtime error
    }
    arg_val = eval_expr(L, env, arg_list->expr);
    KL_add_local_variable(local_env, param_list->name, &arg_val);
  }

  if (NULL != param_list) {
    //! runtime error
  }

  result = KL_exec_stmt_list(L, local_env, 
      func->func.define.block->stmt_list);

  if (SRT_RETURN == result.type)
    value = result.stmt_result.value;
  else 
    value.val_type = VT_NIL;

  dispose_local_env(L, local_env);

  return value;
}

static KL_Value 
eval_func_call_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr)
{
  KL_Value value;
  KL_Function* func;

  char* identifier = expr->expr.func_call_expr.identifier;
  func = KL_lookup_func(identifier);
  if (NULL == func) {
    //! TODO:
    //! runtime error
  }

  switch (func->func_type) {
  case FT_DEFINE:
    value = call_defined_func(L, env, expr, func);
    break;
  case FT_NATIVE:
    value = call_native_func(L, env, expr, func->func.native.proc);
    break;
  default:
    //! panic
    break;
  }

  return value;
}

static KL_Value 
eval_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr)
{
  KL_Value v;

  switch (expr->expr_type) {
  case ET_BOOL:
    v = eval_boolean_expr(expr->expr.bool_val);
    break;
  case ET_INT:
    v = eval_int_expr(expr->expr.int_val);
    break;
  case ET_REAL:
    v = eval_real_expr(expr->expr.real_val);
    break;
  case ET_STR:
    v = eval_str_expr(L, expr->expr.str_val);
    break;
  case ET_ID:
    v = eval_identifier_expr(L, env, expr);
    break;
  case ET_ASSIGN:
    v = eval_assign_expr(L, env, 
        expr->expr.assign_expr.variable, 
        expr->expr.assign_expr.operand);
    break;
  case ET_ADD:
  case ET_SUB:
  case ET_MUL:
  case ET_DIV:
  case ET_MOD:
  case ET_EQ:
  case ET_NEQ:
  case ET_GT:
  case ET_GE:
  case ET_LT:
  case ET_LE:
    v = KL_eval_bin_expr(L, env, expr->expr_type, 
        expr->expr.binary_expr.left, expr->expr.binary_expr.right);
    break;
  case ET_AND:
  case ET_OR:
    v = eval_logical_expr(L, env, expr->expr_type, 
        expr->expr.binary_expr.left, expr->expr.binary_expr.right);
    break;
  case ET_MINUS:
    v = KL_eval_minus_expr(L, env, expr->expr.minus_expr);
    break;
  case ET_FUNC_CALL:
    v = eval_func_call_expr(L, env, expr);
    break;
  case ET_NIL:
    v = eval_nil_expr();
    break;
  default:
    //! panic
    break;
  }

  return v;
}

KL_Value 
KL_eval_expr(KL_State* L, KL_LocalEnv* env, KL_Expr* expr)
{
  return eval_expr(L, env, expr);
}
