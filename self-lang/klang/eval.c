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
