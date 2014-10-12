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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Token {
  BEGIN = 0,
  END,
  READ,
  WRITE,
  ID,
  INTLITERAL,
  LPAREN,
  RPAREN,
  SEMICOLON,
  COMMA,
  ASSIGNOP,
  ADDOP,
  SUBOP,
  SCANEOF,
} Token;

typedef struct Keyword {
  Token token;
  char* name;
} Keyword;

static Keyword reserveds[] = {
  {BEGIN, "begin"},
  {END, "end"},
  {READ, "read"},
  {WRITE, "write"},
};

static char token_buffer[128];
static int  token_index = 0;

static void clear_buffer(void) {
  memset(token_buffer, 0, sizeof(token_buffer));
  token_index = 0;
}

static void buffer_char(int in_char) {
  token_buffer[token_index++] = (char)in_char;
}

static Token check_reserved(void) {
  int count = sizeof(reserveds) / sizeof(*reserveds);
  int i;

  for (i = 0; i < count; ++i) {
    if (0 == strcmp(token_buffer, reserveds[i].name))
      return reserveds[i].token;
  }

  return ID;
}

Token lexer_scanner(void) {
  int in_char, c;

  clear_buffer();
  if (feof(stdin))
    return SCANEOF;

  while (EOF != (in_char = getchar())) {
    if (isspace(in_char)) {
      continue;
    }
    else if (isalpha(in_char)) {
      buffer_char(in_char);
      for (c = getchar(); isalnum(c) || '_' == c; c = getchar())
        buffer_char(c);
      ungetc(c, stdin);
      return check_reserved();
    }
    else if (isdigit(in_char)) {
      buffer_char(in_char);
      for (c = getchar(); isdigit(c); c = getchar())
        buffer_char(c);
      ungetc(c, stdin);
      return INTLITERAL;
    }
    else if ('(' == in_char) {
      return LPAREN;
    }
    else if (')' == in_char) {
      return RPAREN;
    }
    else if (';' == in_char) {
      return SEMICOLON;
    }
    else if (',' == in_char) {
      return COMMA;
    }
    else if ('+' == in_char) {
      return ADDOP;
    }
    else if ('=' == in_char) {
      return ASSIGNOP;
    }
    else if ('-' == in_char) {
      return SUBOP;
    }
    else if ('#' == in_char) {
      do {
        in_char = getchar();
      } while ('\n' != in_char);
    }
    else {
      fprintf(stderr, "lexer error : %c\n", in_char);
      abort();
    }
  }

  return SCANEOF;
}

int main(int argc, char* argv[]) {
  return 0;
}
