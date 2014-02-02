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


static KL_String* 
alloc_string(KL_State* L, const char* str, KL_Boolean is_literal)
{
  KL_String* ret = (KL_String*)KL_malloc(sizeof(*ret));
  ret->ref_count = 0;
  ret->is_literal = is_literal;
  ret->string = (char*)str;

  return ret;
}

KL_String* 
KL_literal_to_string(KL_State* L, const char* str)
{
  KL_String* ret = alloc_string(L, str, BOOL_YES);
  ret->ref_count = 1;

  return ret;
}

void 
KL_refer_string(KL_String* str)
{
  ++str->ref_count;
}

void 
KL_release_string(KL_String* str)
{
  --str->ref_count;

  if (0 == str->ref_count) {
    if (!str->is_literal)
      KL_free(str->string);

    KL_free(str);
  }
}



KL_String* 
KL_create_string(KL_State* L, const char* str)
{
  KL_String* ret = alloc_string(L, str, BOOL_NO);
  ret->ref_count = 1;

  return ret;
}
