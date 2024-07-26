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
#include "token.h"

static const char* kTokenStrings[] = {
#undef TOKENDEF
#define TOKENDEF(k, s)  #k,
#include "token_defs.h"
#undef TOKENDEF

  NULL,
};

const char* tokenTypeAsString(TokenType type) {
  if (type >= TOKEN_LEFT_PAREN && type < COUNTER_OF_TOKEN)
    return kTokenStrings[type];
  return "<UNKNOWN>";
}

Token makeToken(TokenType type, const char* literal, int length, int lineno) {
  Token token = {.type = type, .start = literal, .length = length, .lineno = lineno};
  return token;
}

Token errorToken(const char* message, int lineno) {
  Token token = {.type = TOKEN_ERROR, .start = message, .length = (int)strlen(message), .lineno = lineno};
  return token;
}

Token syntheticToken(const char* text) {
  Token token = {.type = TOKEN_STRING, .start = text, .length = (int)strlen(text), .lineno = 0};
  return token;
}
