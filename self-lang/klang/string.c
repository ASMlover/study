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

#define KL_STRING_ALLOC_SZ  (256)


static char* s_str_literal_buffer = NULL;
static int   s_str_literal_buffer_size = 0;
static int   s_str_literal_buffer_alloc_sz = 0;


char* 
KL_create_identifier(const char* str)
{
  char* new_str = (char*)KL_util_malloc(strlen(str) + 1);
  strcpy(new_str, str);

  return new_str;
}



void 
KL_open_string_literal(void)
{
  s_str_literal_buffer_size = 0;
}

char* 
KL_close_string_literal(void)
{
  char* new_str = KL_util_malloc(s_str_literal_buffer_size + 1);
  memcpy(new_str, s_str_literal_buffer, s_str_literal_buffer_size);
  new_str[s_str_literal_buffer_size] = 0;

  return new_str;
}

void 
KL_add_string_literal(int letter)
{
  if (s_str_literal_buffer_size ==s_str_literal_buffer_alloc_sz) {
    s_str_literal_buffer_alloc_sz += KL_STRING_ALLOC_SZ;
    s_str_literal_buffer
      = (char*)KL_realloc(s_str_literal_buffer, 
          s_str_literal_buffer_alloc_sz);
  }

  s_str_literal_buffer[s_str_literal_buffer_size] = (char)letter;
  ++s_str_literal_buffer_size;
}

void 
KL_reset_string_literal_buffer(void)
{
  KL_free(s_str_literal_buffer);
  s_str_literal_buffer = NULL;
  s_str_literal_buffer_size = 0;
  s_str_literal_buffer_alloc_sz = 0;
}
