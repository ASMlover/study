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
#include "lexer.h"


static const char* kTokenTypes[] = {
  "TOKEN_TYPE_ERR", 
  "TOKEN_TYPE_EOF",         /* EOF */

  "TOKEN_TYPE_CINT",        /* const int */
  "TOKEN_TYPE_CREAL",       /* const real */

  "TOKEN_TYPE_ID",          /* id */

  "TOKEN_TYPE_ASSIGN",      /* = */
  "TOKEN_TYPE_EQ",          /* == */
  "TOKEN_TYPE_NEQ",         /* <> */
  "TOKEN_TYPE_LT",          /* < */
  "TOKEN_TYPE_LE",          /* <= */
  "TOKEN_TYPE_GT",          /* > */
  "TOKEN_TYPE_GE",          /* >= */
  "TOKEN_TYPE_ADD",         /* + */
  "TOKEN_TYPE_SUB",         /* - */
  "TOKEN_TYPE_MUL",         /* * */
  "TOKEN_TYPE_DIV",         /* / */
  "TOKEN_TYPE_MOD",         /* % */

  "TOKEN_TYPE_DOT",         /* . */
  "TOKEN_TYPE_COMMA",       /* , */
  "TOKEN_TYPE_SEMI",        /* ; */
  "TOKEN_TYPE_LPAREN",      /* ( */
  "TOKEN_TYPE_RPAREN",      /* ) */
  "TOKEN_TYPE_LBRACKET",    /* [ */
  "TOKEN_TYPE_RBRACKET",    /* ] */
  "TOKEN_TYPE_LBRACE",      /* { */
  "TOKEN_TYPE_RBRACE",      /* } */

  "TOKEN_TYPE_NIL",         /* nil */
  "TOKEN_TYPE_TRUE",        /* true */
  "TOKEN_TYPE_FALSE",       /* false */
  "TOKEN_TYPE_AND",         /* and */
  "TOKEN_TYPE_OR",          /* or */
  "TOKEN_TYPE_NOT",         /* not */
  "TOKEN_TYPE_IF",          /* if */
  "TOKEN_TYPE_ELSE",        /* else */
  "TOKEN_TYPE_FOR",         /* for */
  "TOKEN_TYPE_BREAK",       /* break */
  "TOKEN_TYPE_FUNC",        /* func */
  "TOKEN_TYPE_RET",         /* return */

  "TOKEN_TYPE_COMMENT",     /* # */
};


int 
main(int argc, char* argv[])
{
  struct Lexer* lex;
  struct Token t;

  if (argc < 2) {
    fprintf(stderr, "USAGE: lexer source-file\n");
    return 1;
  }
  
  lex = lexer_create(argv[1]);
  while (TOKEN_TYPE_EOF != lexer_token(lex, &t)) {
    fprintf(stdout, "        %s:%4d - %s:%s\n", t.line.fname, 
        t.line.lineno, kTokenTypes[t.type], t.name);
  }
  lexer_release(&lex);

  return 0;
}
