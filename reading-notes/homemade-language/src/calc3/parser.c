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
#include <stdio.h>
#include <stdlib.h>
#include "token.h"



#define MAX_LINE_SIZE   (1024)

static Token _s_ahead_token;
static int   _s_ahead_token_exist;



static void 
token_get(Token* token)
{
  if (_s_ahead_token_exist) {
    *token = _s_ahead_token;
    _s_ahead_token_exist = 0;
  } 
  else {
    get_token(token);
  }
}

static void 
token_unget(Token* token)
{
  _s_ahead_token = *token;
  _s_ahead_token_exist = 1;
}


static double parse_expression(void);

static double 
parse_primary_expression(void)
{
  Token token;
  double value = 0.0;
  int sub_flag = 0;

  token_get(&token);
  if (TOKEN_TYPE_SUB == token.type) 
    sub_flag = 1;
  else 
    token_unget(&token);

  token_get(&token);
  if (TOKEN_TYPE_NUMBER == token.type) {
    value = token.value;
  }
  else if (TOKEN_TYPE_LPAREN == token.type) {
    value = parse_expression();
    token_get(&token);
    if (TOKEN_TYPE_RPAREN != token.type) {
      fprintf(stderr, "missing ) syntax error.\n");
      exit(1);
    }
  }
  else {
    token_unget(&token);
  }

  if (sub_flag) 
    value = -value;

  return value;
}

static double 
parse_term(void)
{
  double v1, v2;
  Token token;

  v1 = parse_primary_expression();
  for (;;) {
    token_get(&token);
    if (TOKEN_TYPE_MUL != token.type && TOKEN_TYPE_DIV != token.type) {
      token_unget(&token);
      break;
    }
    
    v2 = parse_primary_expression();
    switch (token.type) {
    case TOKEN_TYPE_MUL:
      v1 *= v2;
      break;
    case TOKEN_TYPE_DIV:
      v1 /= v2;
      break;
    }
  }

  return v1;
}

static double 
parse_expression(void)
{
  double v1, v2;
  Token token;

  v1 = parse_term();
  for (;;) {
    token_get(&token);
    if (TOKEN_TYPE_ADD != token.type && TOKEN_TYPE_SUB != token.type) {
      token_unget(&token);
      break;
    }

    v2 = parse_term();
    switch (token.type) {
    case TOKEN_TYPE_ADD:
      v1 += v2;
      break;
    case TOKEN_TYPE_SUB:
      v1 -= v2;
      break;
    default:
      token_unget(&token);
      break;
    }
  }

  return v1;
}


double 
parse_line(void)
{
  double value;

  _s_ahead_token_exist = 0;
  value = parse_expression();

  return value;
}
