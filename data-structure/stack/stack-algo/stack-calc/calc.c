/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stack.h"


static char __buf[128];

static int
priority_level_cmp(int a, int b) 
{
  if (('+' == (a) || '-' == (a))
    && ('*' == (b) || '/' == (b)))
    return -1;
  else if (('+' == (a) || '-' == (a))
    && ('+' == (b) || '-' == (b)))
    return 0;
  else if (('*' == (a) || '/' == (a))
    && ('*' == (b) || '/' == (b)))
    return 0;
  else 
    return 1;
}


static int 
switch_to_postexp(const char* buf, int buf_len)
{
  int i_src, i_dest = 0;
  int ch;
  stack_t s;

  if (NULL == buf || buf_len <= 0) {
    fprintf(stderr, "expression error ...");
    exit(0);
  }

  s = stack_create();
  for (i_src = 0; i_src < buf_len; ++i_src) {
    ch = buf[i_src];

    if (isdigit(ch)) 
      __buf[i_dest++] = ch;
    else if (isspace(ch)) {
    }
    else if (')' == ch) {
      element_t e;
      while (!stack_empty(s)) {
        e = stack_pop(s);

        if ('(' == e)
          break;
        else 
          __buf[i_dest++] = e;
      }
    }
    else if (!stack_empty(s) 
      && (-1 == priority_level_cmp(ch, stack_peek(s)))) {
      while (!stack_empty(s))
        __buf[i_dest++] = stack_pop(s);
      stack_push(s, ch);
    }
    else 
      stack_push(s, ch);
  }
  while (!stack_empty(s)) 
    __buf[i_dest++] = stack_pop(s);
  stack_delete(&s);
  
  return i_dest;
}

int 
get_calc_result(const char* buf, int buf_len)
{
  int i, ch, calc_ret = 0;
  stack_t s;
  if (NULL == buf || buf_len <= 0) {
    fprintf(stderr, "expression error ...");
    exit(0);
  }

  s = stack_create();
  for (i = 0; i < buf_len; ++i) {
    ch = buf[i];
    
    if (isdigit(ch))
      stack_push(s, ch);
    else if (isspace(ch)) {
    }
    else {
      element_t op2 = stack_pop(s) - '0';
      element_t op1 = stack_pop(s) - '0';
      switch (ch) {
      case '+':
        calc_ret = op1 + op2;
        break;
      case '-':
        calc_ret = op1 - op2;
        break;
      case '*':
        calc_ret = op1 * op2;
        break;
      case '/':
        calc_ret = op1 / op2;
        break;
      }
      stack_push(s, calc_ret + '0');
    }
  }
  calc_ret = stack_pop(s) - '0';
  stack_delete(&s);

  return calc_ret;
}

int 
calc(const char* buf, int buf_len)
{
  int exp_len = switch_to_postexp(buf, buf_len);
  int result  = get_calc_result(__buf, exp_len);

  return result;
}
