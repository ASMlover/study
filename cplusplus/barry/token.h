/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __BARRY_TOKEN_HEADER_H__
#define __BARRY_TOKEN_HEADER_H__

enum TokenType {
  TOKEN_NONE,
  TOKEN_LPAREN,   /* ( */
  TOKEN_RPAREN,   /* ) */
  TOKEN_LBRACE,   /* { */
  TOKEN_RBRACE,   /* } */
  TOKEN_LBRACKET, /* [ */
  TOKEN_RBRACKET, /* ] */
  TOKEN_SEMI,     /* ; */
  TOKEN_PLUS,     /* + */
  TOKEN_MINUS,    /* - */
  TOKEN_MULTIPLY, /* * */
  TOKEN_DIVIDE,   /* / */
  TOKEN_LT,       /* < */
  TOKEN_GT,       /* > */
  TOKEN_EQ,       /* == */
  TOKEN_NE,       /* != */
  TOKEN_COMMA,    /* , */
  TOKEN_NEGATE,   /* ~ */
  TOKEN_PIPE,     /* | */
  TOKEN_TILDE,    /* ~ */
  TOKEN_BITOR,    /* | */
  TOKEN_RSHIFT,   /* >> */
  TOKEN_LSHIFT,   /* << */
  TOKEN_OR,       /* || */
  TOKEN_AND,      /* && */
  TOKEN_ASSIGN,   /* = */
  TOKEN_ID,       /* [a-zA-Z_0-9] */
  TOKEN_STR,      /* "string" */
  TOKEN_NUMBER,   /* 0-9.0-9 */
  TOKEN_DEF,      /* def <name> */
  TOKEN_DO,       /* do */
  TOKEN_END,      /* end */
  TOKEN_IF,       /* if */
  TOKEN_ELSE,     /* else */
  TOKEN_FOR,      /* for */
  TOKEN_WHILE,    /* while */
  TOKEN_BREAK,    /* break */
  TOKEN_CONTINUE, /* continue */
  TOKEN_TRUE,     /* true */
  TOKEN_FALSE,    /* false */
  TOKEN_NIL,      /* nil */
};

typedef struct BarryToken {
  int type;
  int colno;
  int lineno;
  char* name;
  struct {
    char* string;
    float number;
  } as;
} BarryToken;

#endif  /* __BARRY_TOKEN_HEADER_H__ */
