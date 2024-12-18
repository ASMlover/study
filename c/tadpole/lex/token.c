/*
 * Copyright (c) 2024 ASMlover. All rights reserved.
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
#include <string.h>
#include <lex/token.h>

static const char* kNames[] = {
#undef TOKENDEF
#define TOKENDEF(k, s)            #k,
#include <lex/kinds_def.h>
#undef TOKENDEF

  NULL,
};

static TokenKind check_keyword(const char* text, sz_t start, sz_t length, const char* rest, TokenKind kind) {
  if (0 == memcmp(text + start, rest, length))
    return kind;
  return TOKEN_IDENTIFIER;
}

const char* get_kind_name(TokenKind kind) {
  if (kind >= TOKEN_LPAREN && kind < COUNT_OF_TOKEN)
    return kNames[kind];
  return "<UNKNOWN>";
}

TokenKind get_keyword_kind(const char* text) {
  if (NULL == text)
    return TOKEN_IDENTIFIER;

  switch (text[0]) {
  case 'a': return check_keyword(text, 1, 2, "nd", KEYWORD_AND);
  case 'c': return check_keyword(text, 1, 4, "lass", KEYWORD_CLASS);
  case 'e': return check_keyword(text, 1, 3, "lse", KEYWORD_ELSE);
  case 'f':
    switch (text[1]) {
    case 'a': return check_keyword(text, 2, 3, "lse", KEYWORD_FALSE);
    case 'o': return check_keyword(text, 2, 1, "r", KEYWORD_FOR);
    case 'n': return KEYWORD_FN;
    }
    break;
  case 'i': return check_keyword(text, 1, 1, "f", KEYWORD_IF);
  case 'n': return check_keyword(text, 1, 2, "il", KEYWORD_NIL);
  case 'o': return check_keyword(text, 1, 1, "r", KEYWORD_OR);
  case 'p': return check_keyword(text, 1, 4, "rint", KEYWORD_PRINT);
  case 'r': return check_keyword(text, 1, 5, "return", KEYWORD_RETURN);
  case 's': return check_keyword(text, 1, 4, "uper", KEYWORD_SUPER);
  case 't':
    switch (text[1]) {
    case 'h': return check_keyword(text, 2, 2, "is", KEYWORD_THIS);
    case 'r': return check_keyword(text, 2, 2, "ue", KEYWORD_TRUE);
    }
    break;
  case 'v': return check_keyword(text, 1, 2, "ar", KEYWORD_VAR);
  case 'w': return check_keyword(text, 1, 4, "hile", KEYWORD_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

Token make_token(TokenKind kind, const char* literal, sz_t length, int lineno) {
  Token token;
  token.kind = kind;
  token.start = literal;
  token.length = length;
  token.lineno = lineno;

  return token;
}

Token make_error_token(const char* message, int lineno) {
  Token token;
  token.kind = TOKEN_ERR;
  token.start = message;
  token.length = strlen(message);
  token.lineno = lineno;

  return token;
}

Token make_synthetic_token(const char* text) {
  Token token;
  token.start = text;
  token.length = strlen(text);
  return token;
}
