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
#include <ctype.h>
#include "token.h"



static char* _s_line;
static int   _s_line_pos;


enum LexicalStatus {
  LEXICAL_STATUS_INIT = 0, 
  LEXICAL_STATUS_REAL_PART, 
  LEXICAL_STATUS_DOT_PART, 
  LEXICAL_STATUS_FRAC_PART, 
};


void 
set_line(const char* line)
{
  _s_line = (char*)line;
  _s_line_pos = 0;
}

void 
get_token(Token* token)
{
  int out_pos = 0;
  int status = LEXICAL_STATUS_INIT;
  char c;

  token->type = TOKEN_TYPE_INVAL;
  while ('\0' != _s_line[_s_line_pos]) {
    c = _s_line[_s_line_pos];
    if ((LEXICAL_STATUS_REAL_PART == status 
          || LEXICAL_STATUS_FRAC_PART == status)
        && !isdigit(c) && '.' != c) {
      token->type = TOKEN_TYPE_NUMBER;
      sscanf(token->name, "%lf", &token->value);
      return;
    }

    if (isspace(c)) {
      if ('\n' == c) {
        token->type = TOKEN_TYPE_CR;
        return;
      }
      ++_s_line_pos;
      continue;
    }

    if (out_pos >= MAX_TOKEN_SIZE - 1) {
      fprintf(stderr, "token too long.\n");
      exit(1);
    }
    token->name[out_pos] = _s_line[_s_line_pos];
    ++_s_line_pos;
    token->name[++out_pos] = '\0';

    if ('+' == c) {
      token->type = TOKEN_TYPE_ADD;
      return;
    }
    else if ('-' == c) {
      token->type = TOKEN_TYPE_SUB;
      return;
    }
    else if ('*' == c) {
      token->type = TOKEN_TYPE_MUL;
      return;
    }
    else if ('/' == c) {
      token->type = TOKEN_TYPE_DIV;
      return;
    } 
    else if ('(' == c) {
      token->type = TOKEN_TYPE_LPAREN;
      return;
    }
    else if (')' == c) {
      token->type = TOKEN_TYPE_RPAREN;
      return;
    }
    else if (isdigit(c)) {
      if (LEXICAL_STATUS_INIT == status) 
        status = LEXICAL_STATUS_REAL_PART;
      else if (LEXICAL_STATUS_DOT_PART == status)
        status = LEXICAL_STATUS_FRAC_PART;
    }
    else if ('.' == c) {
      if (LEXICAL_STATUS_REAL_PART == status) {
        status = LEXICAL_STATUS_DOT_PART;
      }
      else {
        fprintf(stderr, "syntax error.\n");
        exit(1);
      }
    }
    else {
      fprintf(stderr, "bad character (%c)\n", c);
      exit(1);
    }
  }
}
