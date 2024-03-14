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
#include <string.h>
#include "scanner.h"

typedef struct {
  const char* start;
  const char* current;
  int lineno;
} Scanner;

static const char* kTokenStrings[] = {
#undef TOKENDEF
#define TOKENDEF(k, s) #k,
#include "token_defs.h"
#undef TOKENDEF

  NULL,
};

Scanner scanner;

const char* tokenTypeAsString(TokenType type) {
  if (type >= TOKEN_LEFT_PAREN && type < COUNTER_OF_TOKEN)
    return kTokenStrings[type];
  return "<UNKNOWN>";
}

static bool isAlpha(char c) {
  return isalpha(c) || c == '_';
}

static bool isAlnum(char c) {
  return isalnum(c) || c == '_';
}

static bool isDigit(char c) {
  return isdigit(c);
}

static bool isAtEnd() {
  return *scanner.current == 0;
}

static char advance() {
  return *scanner.current++;
}

static char peek() {
  return *scanner.current;
}

static char peekNext() {
  if (isAtEnd())
    return 0;
  return scanner.current[1];
}

static bool match(char expected) {
  if (isAtEnd())
    return false;

  if (*scanner.current == expected) {
    ++scanner.current;
    return true;
  }
  return false;
}

static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.lineno = scanner.lineno;

  return token;
}

static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.lineno = scanner.lineno;

  return token;
}

static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '\n': ++scanner.lineno; advance(); break;
    case '/':
      if (peekNext() == '/') {
        while (peek() != '\n' && !isAtEnd())
          advance();
      }
      else {
        return;
      }
      break;
    default: return;
    }
  }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length
      && memcmp(scanner.start + start, rest, length) == 0)
    return type;
  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (scanner.start[0]) {
  case 'a': return checkKeyword(1, 2, "nd", KEYWORD_AND);
  case 'c': return checkKeyword(1, 4, "lass", KEYWORD_CLASS);
  case 'e': return checkKeyword(1, 3, "lse", KEYWORD_ELSE);
  case 'f':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'a': return checkKeyword(2, 3, "lse", KEYWORD_FALSE);
      case 'o': return checkKeyword(2, 1, "r", KEYWORD_FOR);
      case 'u': return checkKeyword(2, 1, "n", KEYWORD_FUN);
      }
    }
    break;
  case 'i': return checkKeyword(1, 1, "f", KEYWORD_IF);
  }
  return TOKEN_IDENTIFIER;
}

void initScanner(const char* sourceCode) {
  scanner.start = sourceCode;
  scanner.current = sourceCode;
  scanner.lineno = 1;
}

Token scanToken() {
  return errorToken("Unexpected character.");
}
