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




struct Lexer {
  char  fname[BFILE];
  FILE* stream;
  char  lexbuf[BSIZE];
  int   lineno;
  int   bsize;
  int   pos;
  int   eof;
};


enum LexState {
  LEX_STATE_BEGIN = 0, 
  LEX_STATE_FINISH, 

  LEX_STATE_IN_CINT, 
  LEX_STATE_IN_CREAL, 
  LEX_STATE_IN_ID, 
  LEX_STATE_IN_ASSIGNEQ,  /* =/== */ 
  LEX_STATE_IN_NEQLTLE,   /* <>/</<= */
  LEX_STATE_IN_GTGE,      /* >/>= */
  LEX_STATE_IN_COMMENT,   /* # */
};




static const struct {
  const char* lexptr;
  int         token;
} kReserveds[] = {
  {"nil", TOKEN_TYPE_NIL}, 
  {"true", TOKEN_TYPE_TRUE}, 
  {"false", TOKEN_TYPE_FALSE}, 
  {"and", TOKEN_TYPE_AND}, 
  {"or", TOKEN_TYPE_OR}, 
  {"not", TOKEN_TYPE_NOT}, 
  {"if", TOKEN_TYPE_IF}, 
  {"else", TOKEN_TYPE_ELSE}, 
  {"for", TOKEN_TYPE_FOR}, 
  {"break", TOKEN_TYPE_BREAK}, 
  {"func", TOKEN_TYPE_FUNC}, 
  {"return", TOKEN_TYPE_RET}, 
};





static int 
get_char(struct Lexer* lex)
{
  if (lex->pos >= lex->bsize) {
    if (NULL != fgets(lex->lexbuf, sizeof(lex->lexbuf), lex->stream)) {
      lex->pos = 0;
      lex->bsize = (int)strlen(lex->lexbuf);

      return lex->lexbuf[lex->pos++];
    }
    else {
      lex->eof = BOOL_NO;
      return EOF;
    }
  }

  return lex->lexbuf[lex->pos++];
}

static void 
unget_char(struct Lexer* lex)
{
  if (!lex->eof)
    --lex->pos;
}


static int 
lookup_reserved(const char* s)
{
  int i, count = countof(kReserveds);
  for (i = 0; i < count; ++i) {
    if (0 == strcmp(kReserveds[i].lexptr, s))
      return kReserveds[i].token;
  }

  return TOKEN_TYPE_ID;
}








struct Lexer* 
lexer_create(const char* fname)
{
  struct Lexer* lex = (struct Lexer*)malloc(sizeof(*lex));
  if (NULL == lex)
    return NULL;

  do {
    lex->stream = fopen(fname, "r");
    if (NULL == lex->stream)
      break;

    strcpy(lex->fname, fname);
    lex->lineno = 1;
    lex->bsize  = 0;
    lex->pos    = 0;
    lex->eof    = BOOL_NO;

    return lex;
  } while (0);

  if (NULL != lex)
    free(lex);

  return NULL;
}

void 
lexer_release(struct Lexer** lex)
{
  if (NULL != *lex) {
    if (NULL !=  (*lex)->stream)
      fclose((*lex)->stream);

    free(*lex);
    *lex = NULL;
  }
}

int 
lexer_token(struct Lexer* lex, struct Token* token)
{
  int type  = TOKEN_TYPE_ERR;
  int i     = 0;
  int state = LEX_STATE_BEGIN;
  int save = BOOL_NO;
  int c;

  while (LEX_STATE_FINISH != state) {
    c = get_char(lex);
    save = BOOL_YES;

    switch (state) {
    case LEX_STATE_BEGIN:
      if (' ' == c || '\t' == c) {
        save = BOOL_NO;
      }
      else if ('\n' == c) {
        save = BOOL_NO;
        ++lex->lineno;
      }
      else if (isdigit(c)) {
        state = LEX_STATE_IN_CINT;
      }
      else if (isalpha(c) || '_' == c) {
        state = LEX_STATE_IN_ID;
      }
      else if ('=' == c) {
        state = LEX_STATE_IN_ASSIGNEQ;
      }
      else if ('<' == c) {
        state = LEX_STATE_IN_NEQLTLE;
      }
      else if ('>' == c) {
        state = LEX_STATE_IN_GTGE;
      }
      else if ('#' == c) {
        save = BOOL_NO;
        state = LEX_STATE_IN_COMMENT;
      }
      else {
        state = LEX_STATE_FINISH;
        switch (c) {
        case EOF:
          save = BOOL_NO;
          type = TOKEN_TYPE_EOF;
          break;
        case '+':
          type = TOKEN_TYPE_ADD;
          break;
        case '-':
          type = TOKEN_TYPE_SUB;
          break;
        case '*':
          type = TOKEN_TYPE_MUL;
          break;
        case '/':
          type = TOKEN_TYPE_DIV;
          break;
        case '%':
          type = TOKEN_TYPE_MOD;
          break;
        case '.':
          type = TOKEN_TYPE_DOT;
          break;
        case ',':
          type = TOKEN_TYPE_COMMA;
          break;
        case ';':
          type = TOKEN_TYPE_SEMI;
          break;
        case '(':
          type = TOKEN_TYPE_LPAREN;
          break;
        case ')':
          type = TOKEN_TYPE_RPAREN;
          break;
        case '[':
          type = TOKEN_TYPE_LBRACKET;
          break;
        case ']':
          type = TOKEN_TYPE_RBRACKET;
          break;
        case '{':
          type = TOKEN_TYPE_LBRACE;
          break;
        case '}':
          type = TOKEN_TYPE_RBRACE;
          break;
        default:
          save = BOOL_NO;
          type = TOKEN_TYPE_ERR;
          break;
        }
      }
      break;
    case LEX_STATE_IN_CINT:
      if ('.' == c) {
        state = LEX_STATE_IN_CREAL;
      }
      else {
        if (!isdigit(c)) {
          unget_char(lex);
          save = BOOL_NO;
          state = LEX_STATE_FINISH;
          type = TOKEN_TYPE_CINT;
        }
      }
      break;
    case LEX_STATE_IN_CREAL:
      if (!isdigit(c)) {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_CREAL;
      }
      break;
    case LEX_STATE_IN_ID:
      if (!isalnum(c) && '_' != c) {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_ID;
      }
      break;
    case LEX_STATE_IN_ASSIGNEQ:
      if ('=' == c) {
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_EQ;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_ASSIGN;
      }
      break;
    case LEX_STATE_IN_NEQLTLE:
      if ('>' == c) {
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_NEQ;
      }
      else if ('=' == c) {
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_LE;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_LT;
      }
      break;
    case LEX_STATE_IN_GTGE:
      if ('=' == c) {
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_GE;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_GT;
      }
      break;
    case LEX_STATE_IN_COMMENT:
      save = BOOL_NO;
      if (EOF == c) {
        state = LEX_STATE_FINISH;
        type = TOKEN_TYPE_EOF;
      }
      else if ('\n' == c) {
        ++lex->lineno;
        state = LEX_STATE_BEGIN;
      }
      break;
    case LEX_STATE_FINISH:
    default:
      save = BOOL_NO;
      state = LEX_STATE_FINISH;
      type = TOKEN_TYPE_ERR;
      break;
    }

    if (save && i < BSIZE) 
      token->name[i++] = (char)c;

    if (LEX_STATE_FINISH == state) {
      token->name[i] = 0;
      token->type = type;
      token->line.lineno = lex->lineno;
      strcpy(token->line.fname, lex->fname);
      if (TOKEN_TYPE_ID == type)
        token->type = type = lookup_reserved(token->name);
    }
  }

  return type;
}
