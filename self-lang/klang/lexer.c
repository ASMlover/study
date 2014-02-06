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



struct KL_Lexer {
  char       fname[BFILE];
  FILE*      stream;
  char       lexbuf[BSIZE];
  int        lineno;
  int        bsize;
  int        pos;
  KL_Boolean eof;
};


enum KL_LexState {
  LEX_STATE_BEGIN = 0, 
  LEX_STATE_FINISH, 

  LEX_STATE_IN_CINT, 
  LEX_STATE_IN_CREAL, 
  LEX_STATE_IN_CSTR, 
  LEX_STATE_IN_ID, 
  LEX_STATE_IN_ASSIGNEQ,  /* =/== */
  LEX_STATE_IN_NEGLTLE,   /* <>/</<= */
  LEX_STATE_IN_GTGE,      /* >/>= */
  LEX_STATE_IN_AND,       /* && */
  LEX_STATE_IN_OR,        /* || */
  LEX_STATE_IN_COMMENT,   /* # */
};


static const struct {
  const char* lexptr;
  int         token;
} kReserveds[] = {
  {"nil", TT_NIL}, 
  {"true", TT_TRUE}, 
  {"false", TT_FALSE}, 
  {"if", TT_IF}, 
  {"else", TT_ELSE}, 
  {"while", TT_WHILE}, 
  {"break", TT_BREAK}, 
  {"func", TT_FUNC}, 
  {"return", TT_RET}, 
};



static int 
get_char(KL_Lexer* lex)
{
  if (lex->pos >= lex->bsize) {
    if (NULL != fgets(lex->lexbuf, sizeof(lex->lexbuf), lex->stream)) {
      lex->pos = 0;
      lex->bsize = (int)strlen(lex->lexbuf);
    }
    else {
      lex->eof = BOOL_YES;
      return EOF;
    }
  }

  return lex->lexbuf[lex->pos++];
}

static void 
unget_char(KL_Lexer* lex)
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

  return TT_ID;
}





KL_Lexer* 
KL_lexer_create(const char* fname)
{
  KL_Lexer* lex = (KL_Lexer*)malloc(sizeof(*lex));
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
KL_lexer_release(KL_Lexer** lex)
{
  if (NULL != *lex) {
    if (NULL != (*lex)->stream) 
      fclose((*lex)->stream);

    free(*lex);
    *lex = NULL;
  }
}

int 
KL_lexer_token(KL_Lexer* lex, KL_Token* tok)
{
  int type        = TT_ERR;
  int i           = 0;
  int state       = LEX_STATE_BEGIN;
  KL_Boolean save = BOOL_NO;
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
      else if ('\'' == c) {
        save = BOOL_NO;
        state = LEX_STATE_IN_CSTR;
      }
      else if (isdigit(c) || '_' == c) {
        state = LEX_STATE_IN_ID;
      }
      else if ('=' == c) {
        state = LEX_STATE_IN_ASSIGNEQ;
      }
      else if ('>' == c) {
        state = LEX_STATE_IN_GTGE;
      }
      else if ('<' == c) {
        state = LEX_STATE_IN_NEGLTLE;
      }
      else if ('&' == c) {
        state = LEX_STATE_IN_AND;
      }
      else if ('|' == c) {
        state = LEX_STATE_IN_OR;
      }
      else if ('#' == c) {
        state = LEX_STATE_IN_COMMENT;
      }
      else {
        state = LEX_STATE_FINISH;
        switch (c) {
        case EOF:
          save = BOOL_NO;
          type = TT_EOF;
          break;
        case '+':
          type = TT_ADD;
          break;
        case '-':
          type = TT_SUB;
          break;
        case '*':
          type = TT_MUL;
          break;
        case '/':
          type = TT_DIV;
          break;
        case '%':
          type = TT_MOD;
          break;
        case '.':
          type = TT_DOT;
          break;
        case ',':
          type = TT_COMMA;
          break;
        case ';':
          type = TT_SEMI;
          break;
        case '(':
          type = TT_LPAREN;
          break;
        case ')':
          type = TT_RPAREN;
          break;
        case '[':
          type = TT_LBRACKET;
          break;
        case ']':
          type = TT_RBRACKET;
          break;
        case '{':
          type = TT_LBRACE;
          break;
        case '}':
          type = TT_RBRACE;
          break;
        default:
          save = BOOL_NO;
          type = TT_ERR;
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
          type = TT_CINT;
        }
      }
      break;
    case LEX_STATE_IN_CREAL:
      if (!isdigit(c)) {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TT_CREAL;
      }
      break;
    case LEX_STATE_IN_CSTR:
      if ('\'' == c) {
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TT_CSTR;
      }
      break;
    case LEX_STATE_IN_ID:
      if (!isalnum(c) && '_' != c) {
        unget_char(lex);
        save = BOOL_NO;
        state = LEX_STATE_FINISH;
        type = TT_ID;
      }
      break;
    case LEX_STATE_IN_ASSIGNEQ:
      if ('=' == c) {
        type = TT_EQ;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        type = TT_ASSIGN;
      }
      state = LEX_STATE_FINISH;
      break;
    case LEX_STATE_IN_NEGLTLE:
      if ('>' == c) {
        type = TT_NEQ;
      }
      else if ('=' == c) {
        type = TT_LE;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        type = TT_LT;
      }
      state = LEX_STATE_FINISH;
      break;
    case LEX_STATE_IN_GTGE:
      if ('=' == c) {
        type = TT_GE;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        type = TT_GT;
      }
      state = LEX_STATE_FINISH;
      break;
    case LEX_STATE_IN_AND:
      if ('&' == c) {
        type = TT_ADD;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        type = TT_ERR;
      }
      state = LEX_STATE_FINISH;
      break;
    case LEX_STATE_IN_OR:
      if ('|' == c) {
        type = TT_OR;
      }
      else {
        unget_char(lex);
        save = BOOL_NO;
        type = TT_ERR;
      }
      state = LEX_STATE_FINISH;
      break;
    case LEX_STATE_IN_COMMENT:
      save = BOOL_NO;
      if (EOF == c) {
        state = LEX_STATE_FINISH;
        type = TT_ERR;
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
      type = TT_ERR;
      break;
    }

    if (save && i < BSIZE)
      tok->name[i++] = (char)c;

    if (LEX_STATE_FINISH == state) {
      tok->name[i] = 0;
      tok->type = type;
      tok->line.lineno = lex->lineno;
      strcpy(tok->line.fname, lex->fname);
      if (TT_ID == type)
        tok->type = type = lookup_reserved(tok->name);
    }
  }

  return type;
}
