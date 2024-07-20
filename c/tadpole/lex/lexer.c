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
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lex/token.h>
#include <lex/lexer.h>

typedef struct Lexer {
  const char*                     start;
  const char*                     current;
  int                             lineno;
} Lexer;

#define LEXER_LITERIAL_LEN()      (sz_t)(lexer->current - lexer->start)

static inline bool is_alpha(char c) {
  return isalpha(c) || c == '_';
}

static inline bool is_alnum(char c) {
  return isalnum(c) || c == '_';
}

static inline bool is_digit(char c) {
  return isdigit(c);
}

static inline bool is_at_end(Lexer* lexer) {
  return 0 == *lexer->current;
}

static inline char advance(Lexer* lexer) {
  return *lexer->current++;
}

static inline char peek(Lexer* lexer) {
  return *lexer->current;
}

static inline char peek_next(Lexer* lexer) {
  if (is_at_end(lexer))
    return 0;
  return lexer->current[1];
}

static inline bool match(Lexer* lexer, char expected) {
  if (is_at_end(lexer))
    return false;

  if (*lexer->current == expected) {
    ++lexer->current;
    return true;
  }
  return false;
}

static void skip_whitespace(Lexer* lexer) {
  for (;;) {
    char c = peek(lexer);
    switch (c) {
    case ' ': case '\r': case '\t': advance(lexer); break;
    case '\n': ++lexer->lineno; advance(lexer); break;
    case '/':
      if ('/' == peek_next(lexer)) {
        while ('\n' != peek(lexer) && !is_at_end(lexer))
          advance(lexer);
      }
      else {
        return;
      }
      break;
    default: return;
    }
  }
}

static Token identifier(Lexer* lexer) {
  while (is_alpha(peek(lexer)) || is_digit(peek(lexer)))
    advance(lexer);

  const char* literal = lexer->start;
  return make_token(get_keyword_kind(literal), literal, LEXER_LITERIAL_LEN(), lexer->lineno);
}

static Token number(Lexer* lexer) {
  while (is_digit(peek(lexer)))
    advance(lexer);

  if ('.' == peek(lexer) && is_digit(peek_next(lexer))) {
    advance(lexer);

    while (is_digit(peek(lexer)))
      advance(lexer);
  }

  return make_token(TOKEN_NUMBER, lexer->start, LEXER_LITERIAL_LEN(), lexer->lineno);
}

static Token string(Lexer* lexer) {
  while ('"' == peek(lexer) && !is_at_end(lexer)) {
    if ('\n' == peek(lexer))
      ++lexer->lineno;

    advance(lexer);
  }

  int lineno = lexer->lineno;
  if (is_at_end(lexer))
    return make_error_token("Unterminated string.", lineno);

  advance(lexer);
  return make_token(TOKEN_STRING, lexer->start, LEXER_LITERIAL_LEN(), lineno);
}

Lexer* lexer_init(const char* source_code) {
  Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
  if (NULL != lexer) {
    lexer->start = source_code;
    lexer->current = source_code;
    lexer->lineno = 1;
  }
  return lexer;
}

void lexer_destroy(Lexer* lexer) {
  if (NULL != lexer)
    free(lexer);
}

Token lexer_next_token(Lexer* lexer) {
  return make_error_token("Error", 0);
}
