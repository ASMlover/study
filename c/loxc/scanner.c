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
  case 'n': return checkKeyword(1, 2, "il", KEYWORD_NIL);
  case 'o': return checkKeyword(1, 1, "r", KEYWORD_OR);
  case 'p': return checkKeyword(1, 4, "rint", KEYWORD_PRINT);
  case 'r': return checkKeyword(1, 5, "eturn", KEYWORD_RETURN);
  case 's': return checkKeyword(1, 4, "uper", KEYWORD_SUPER);
  case 't':
    if (scanner.current - scanner.start > 1) {
      switch (scanner.start[1]) {
      case 'h': return checkKeyword(2, 2, "is", KEYWORD_THIS);
      case 'r': return checkKeyword(2, 2, "ue", KEYWORD_TRUE);
      }
    }
    break;
  case 'v': return checkKeyword(1, 2, "ar", KEYWORD_VAR);
  case 'w': return checkKeyword(1, 4, "hile", KEYWORD_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek()))
    advance();
  return makeToken(identifierType());
}

static Token number() {
  while (isDigit(peek()))
    advance();

  if (peek() == '.' && isDigit(peekNext())) {
    advance();

    while (isDigit(peek()))
      advance();
  }
  return makeToken(TOKEN_NUMBER);
}

static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      ++scanner.lineno;

    advance();
  }

  if (isAtEnd())
    return errorToken("Unterminated string.");

  advance();
  return makeToken(TOKEN_STRING);
}

void initScanner(const char* sourceCode) {
  scanner.start = sourceCode;
  scanner.current = sourceCode;
  scanner.lineno = 1;
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();
  if (isAlpha(c))
    return identifier();
  if (isDigit(c))
    return number();

  switch (c) {
  case '(': return makeToken(TOKEN_LEFT_PAREN);
  case ')': return makeToken(TOKEN_RIGHT_PAREN);
  case '{': return makeToken(TOKEN_LEFT_BRACE);
  case '}': return makeToken(TOKEN_RIGHT_BRACE);
  case ';': return makeToken(TOKEN_SEMICOLON);
  case ',': return makeToken(TOKEN_COMMA);
  case '.': return makeToken(TOKEN_DOT);
  case '-': return makeToken(TOKEN_MINUS);
  case '+': return makeToken(TOKEN_PLUS);
  case '/': return makeToken(TOKEN_SLASH);
  case '*': return makeToken(TOKEN_STAR);
  case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"': return string();
  default: break;
  }

  return errorToken("Unexpected character.");
}
