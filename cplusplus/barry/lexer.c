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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "token.h"
#include "lexer.h"

#define _LexerPeek(L) L->src[L->offset]

static unsigned char
_LexerNext(BarryLexer* L)
{
  unsigned char ch = L->src[L->offset];

  if ('\0' != ch) {
    ++L->offset;
    L->pch = L->ch;
    L->ch = ch;

    if ('\n' == ch || '\r' == ch) {
      L->colno = 1;
    }
    else {
      ++L->colno;
    }

    return ch;
  }

  return '\0';
}

static unsigned char
_LexerPrev(BarryLexer* L)
{
  int index = L->offset - 1;
  if (index < 0) {
    L->offset = 0;
    L->colno = 1;
    L->ch = L->src[0];
    L->pch = 0;
    return L->src[0];
  }
  else {
    --L->offset;
    if (L->lineno < 3) {
      L->colno = 1;
    }
    else {
      --L->colno;
    }

    L->ch = L->src[index];
    if (0 == index) {
      L->pch = 0;
    }
    else {
      L->pch = L->src[index - 1];
    }
    return L->src[index];
  }
}

static void
_LexerToken(BarryLexer* L, int type, const char* buf)
{
  L->last = L->curr;
  L->curr.type = type;
  L->curr.lineno = L->lineno;
  L->curr.colno = L->colno;
  L->curr.as.string = strdup(buf);

  switch (type) {
  case TOKEN_NUMBER:
    L->curr.as.number = atof(buf);
    break;
  }
}

static int
_LexerScanIdentifier(BarryLexer* L, unsigned char ch)
{
  unsigned char buf[BUFSIZ];
  int size = 0;
  int num = 0;
  int set = 0;
  int i = 0;

  do {
    buf[size++] = ch;
    ch = _LexerNext(L);
  } while (isalnum(ch) || '_' == ch || '.' == ch);

  _LexerPrev(L);
  if (0 == size)
    return -1;

  buf[size] = '\0';
  num = 1;
  for (; i < size; ++i) {
    if (!isdigit(buf[i]) && '.' != buf[i])
      num = 0;
  }

#define SET_TOKEN_IF(str, token)\
  if (0 == set && EQUAL((char*)str, (char*)buf)) {\
    set = 1;\
    _LexerToken(L, token, (char*)buf);\
  }

  if (num) {
    _LexerToken(L, TOKEN_NUMBER, (char*)buf);
  }
  else {
    SET_TOKEN_IF("if", TOKEN_IF);
    SET_TOKEN_IF("else", TOKEN_ELSE);
    SET_TOKEN_IF("end", TOKEN_END);
    SET_TOKEN_IF("do", TOKEN_DO);
    SET_TOKEN_IF("for", TOKEN_FOR);
    SET_TOKEN_IF("while", TOKEN_WHILE);
    SET_TOKEN_IF("break", TOKEN_BREAK);
    SET_TOKEN_IF("continue", TOKEN_CONTINUE);
    SET_TOKEN_IF(buf, TOKEN_ID);
  }

  return 0;
}

static int
_LexerScanString(BarryLexer* L, unsigned char ch)
{
  unsigned char quote = ch;
  unsigned char buf[BUFSIZ];
  int size = 0;
  int ignore = 0;

  if ('"' != ch && '\'' != ch)
    return -1;

  while (1) {
    ch = _LexerNext(L);
    ignore = 0;

    if ('\\' == ch && ('"' == _LexerPeek(L) || '\'' == _LexerPeek(L))) {
      buf[size++] = ch;
      ch = _LexerNext(L);
      ignore = 1;
    }

    if (quote == ch && 0 == ignore)
      break;
    if ('\n' == ch || '\r' == ch)
      return -1;

    buf[size++] = ch;
  }

  buf[size] = '\0';
  L->colno -= size;
  _LexerToken(L, TOKEN_STR, (char*)buf);

  return 0;
}

int
barry_LexerScan(BarryLexer* L)
{
  unsigned char ch = 0;

SCAN:
  ch = _LexerNext(L);
  switch (ch) {
  case '\0':
    return 1;
  case ' ':
  case '\t':
    goto SCAN;
  case '#':
    while ('\n' != ch && '\r' != ch) {
      ch = _LexerNext(L);
    }
    goto SCAN;
  case '\r':
  case '\n':
    ++L->lineno;
    L->colno = 1;
    goto SCAN;
  case '"':
  case '\'':
    return _LexerScanString(L, ch);
  case ',':
    return _LexerToken(L, TOKEN_COMMA, ","), 0;
  case '(':
    return _LexerToken(L, TOKEN_LPARAM, "("), 0;
  case ')':
    return _LexerToken(L, TOKEN_RPARAM, ")"), 0;
  case ';':
    return _LexerToken(L, TOKEN_SEMI, ";"), 0;
  case '+':
    return _LexerToken(L, TOKEN_PLUS, "+"), 0;
  case '-':
    return _LexerToken(L, TOKEN_MINUS, "-"), 0;
  case '*':
    return _LexerToken(L, TOKEN_MULTIPLY, "*"), 0;
  case '/':
    return _LexerToken(L, TOKEN_DIVIDE, "/"), 0;
  case '=':
    if ('=' == _LexerPeek(L))
      return _LexerToken(L, TOKEN_EQ, "=="), 0;
    else
      return _LexerToken(L, TOKEN_ASSIGN, "="), 0;
  case '!':
    return _LexerToken(L, TOKEN_NE, "!"), 0;
  case '|':
    if ('|' == _LexerPeek(L))
      return _LexerToken(L, TOKEN_OR, "||"), 0;
    else
      return _LexerToken(L, TOKEN_BITOR, "|"), 0;
  case '{':
    return _LexerToken(L, TOKEN_LBRACE, "{"), 0;
  case '}':
    return _LexerToken(L, TOKEN_RBRACE, "}"), 0;
  case '[':
    return _LexerToken(L, TOKEN_LBRACKET, "["), 0;
  case ']':
    return _LexerToken(L, TOKEN_RBRACKET, "]"), 0;
  default:
    return _LexerScanIdentifier(L, ch);
  }

  return 0;
}
