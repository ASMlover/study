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
#include "memory.h"

#define MAKE_TOKEN(t)   makeToken((t), scanner->start, (int)(scanner->current - scanner->start), scanner->lineno)

struct Scanner {
  const char*           start;
  const char*           current;
  int                   lineno;
};

static inline bool isAlpha(char c) {
  return isalpha(c) || '_' == c;
}

static inline bool isAlnum(char c) {
  return isalnum(c) || '_' == c;
}

static inline bool isDigit(char c) {
  return isdigit(c);
}

static inline bool isAtEnd(Scanner* scanner) {
  return 0 == *scanner->current;
}

static inline char advance(Scanner* scanner) {
  return *scanner->current++;
}

static inline char peek(Scanner* scanner) {
  return *scanner->current;
}

static inline char peekNext(Scanner* scanner) {
  if (isAtEnd(scanner))
    return 0;
  return scanner->current[1];
}

static bool match(Scanner* scanner, char expected) {
  if (isAtEnd(scanner))
    return false;

  if (*scanner->current == expected) {
    ++scanner->current;
    return true;
  }
  return false;
}

static void skipWhitespace(Scanner* scanner) {
  for (;;) {
    char c = peek(scanner);
    switch (c) {
    case ' ': case '\r': case '\t': advance(scanner); break;
    case '\n': ++scanner->lineno; advance(scanner); break;
    case '/':
      if ('/' == peekNext(scanner)) {
        while ('\n' != peek(scanner) && !isAtEnd(scanner))
          advance(scanner);
      }
      else {
        return;
      }
      break;
    default: return;
    }
  }
}

static TokenType checkKeyword(Scanner* scanner, int start, int length, const char* rest, TokenType type) {
  if (scanner->current - scanner->start == start + length
      && 0 == memcmp(scanner->start + start, rest, length))
    return type;
  return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Scanner* scanner) {
  switch (scanner->start[0]) {
  case 'a': return checkKeyword(scanner, 1, 2, "nd", KEYWORD_AND);
  case 'c': return checkKeyword(scanner, 1, 4, "lass", KEYWORD_CLASS);
  case 'e': return checkKeyword(scanner, 1, 3, "lse", KEYWORD_ELSE);
  case 'f':
    if (scanner->current - scanner->start > 1) {
      switch (scanner->start[1]) {
      case 'a': return checkKeyword(scanner, 2, 3, "lse", KEYWORD_FALSE);
      case 'o': return checkKeyword(scanner, 2, 1, "r", KEYWORD_FOR);
      case 'u': return checkKeyword(scanner, 2, 1, "n", KEYWORD_FUN);
      }
    }
    break;
  case 'i': return checkKeyword(scanner, 1, 1, "f", KEYWORD_IF);
  case 'n': return checkKeyword(scanner, 1, 2, "il", KEYWORD_NIL);
  case 'o': return checkKeyword(scanner, 1, 1, "r", KEYWORD_OR);
  case 'p': return checkKeyword(scanner, 1, 4, "rint", KEYWORD_PRINT);
  case 'r': return checkKeyword(scanner, 1, 5, "eturn", KEYWORD_RETURN);
  case 's': return checkKeyword(scanner, 1, 4, "uper", KEYWORD_SUPER);
  case 't':
    if (scanner->current - scanner->start > 1) {
      switch (scanner->start[1]) {
      case 'h': return checkKeyword(scanner, 2, 2, "is", KEYWORD_THIS);
      case 'r': return checkKeyword(scanner, 2, 2, "ue", KEYWORD_TRUE);
      }
    }
    break;
  case 'v': return checkKeyword(scanner, 1, 2, "ar", KEYWORD_VAR);
  case 'w': return checkKeyword(scanner, 1, 4, "hile", KEYWORD_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner* scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner)))
    advance(scanner);

  return MAKE_TOKEN(identifierType(scanner));
}

static Token number(Scanner* scanner) {
  while (isDigit(peek(scanner)))
    advance(scanner);

  if ('.' == peek(scanner) && isDigit(peekNext(scanner))) {
    advance(scanner);

    while (isDigit(peek(scanner)))
      advance(scanner);
  }
  return MAKE_TOKEN(TOKEN_NUMBER);
}

static Token string(Scanner* scanner) {
  while ('"' != peek(scanner) && !isAtEnd(scanner)) {
    if ('\n' == peek(scanner))
      ++scanner->lineno;

    advance(scanner);
  }

  if (isAtEnd(scanner))
    return errorToken("Unterminated string.", scanner->lineno);

  advance(scanner);
  return MAKE_TOKEN(TOKEN_STRING);
}

Scanner* allocScanner(const char* sourceCode) {
  Scanner* scanner = ALLOCATE(Scanner, 1);
  if (NULL != scanner)
    initScanner(scanner, sourceCode);
  return scanner;
}

void deallocScanner(Scanner* scanner) {
  FREE(Scanner, scanner);
}

void initScanner(Scanner* scanner, const char* sourceCode) {
  scanner->start = sourceCode;
  scanner->current = sourceCode;
  scanner->lineno = 1;
}

void destroyScanner(Scanner* scanner) {
  scanner->start = NULL;
  scanner->current = NULL;
  scanner->lineno = 0;
}

Token scanToken(Scanner* scanner) {
  skipWhitespace(scanner);
  scanner->start = scanner->current;

  if (isAtEnd(scanner))
    return MAKE_TOKEN(TOKEN_EOF);

  char c = advance(scanner);
  if (isAlpha(c))
    return identifier(scanner);
  if (isDigit(c))
    return number(scanner);

  switch (c) {
  case '(': return MAKE_TOKEN(TOKEN_LEFT_PAREN);
  case ')': return MAKE_TOKEN(TOKEN_RIGHT_PAREN);
  case '{': return MAKE_TOKEN(TOKEN_LEFT_BRACE);
  case '}': return MAKE_TOKEN(TOKEN_RIGHT_BRACE);
  case ';': return MAKE_TOKEN(TOKEN_SEMICOLON);
  case ',': return MAKE_TOKEN(TOKEN_COMMA);
  case '.': return MAKE_TOKEN(TOKEN_DOT);
  case '-': return MAKE_TOKEN(TOKEN_MINUS);
  case '+': return MAKE_TOKEN(TOKEN_PLUS);
  case '/': return MAKE_TOKEN(TOKEN_SLASH);
  case '*': return MAKE_TOKEN(TOKEN_STAR);
  case '!': return MAKE_TOKEN(match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=': return MAKE_TOKEN(match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<': return MAKE_TOKEN(match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>': return MAKE_TOKEN(match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
  case '"': return string(scanner);
  default: break;
  }

  return errorToken("Unexpected character.", scanner->lineno);
}
