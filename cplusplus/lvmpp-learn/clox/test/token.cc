// Copyright (c) 2023 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include "../harness.hh"
#include "../token.hh"

CLOX_TEST(Token) {
  using TTy = clox::TokenType;

#define NEWTK2(k, s)    clox::Token(k, s, 0)
#define NEWTK3(k, s, l) clox::Token(k, s, l)
#define TESTEQ(a, b)    CLOX_CHECK_EQ(a, b)
#define TESTTK(k, s) do {\
  auto t = NEWTK2(k, s);\
  TESTEQ(t.type(), k);\
  TESTEQ(t.literal(), s);\
  TESTEQ(t.lineno(), 0);\
} while (false)

  TESTTK(TTy::TOKEN_LEFT_PAREN, "(");
  TESTTK(TTy::TOKEN_RIGHT_PAREN, ")");
  TESTTK(TTy::TOKEN_LEFT_BRACE, "{");
  TESTTK(TTy::TOKEN_RIGHT_BRACE, "}");
  TESTTK(TTy::TOKEN_COMMA, ",");
  TESTTK(TTy::TOKEN_DOT, ".");
  TESTTK(TTy::TOKEN_MINUS, "-");
  TESTTK(TTy::TOKEN_PLUS, "+");
  TESTTK(TTy::TOKEN_SEMICOLON, ";");
  TESTTK(TTy::TOKEN_SLASH, "/");
  TESTTK(TTy::TOKEN_STAR, "*");

  TESTTK(TTy::TOKEN_BANG, "!");
  TESTTK(TTy::TOKEN_BANG_EQUAL, "!=");
  TESTTK(TTy::TOKEN_EQUAL, "=");
  TESTTK(TTy::TOKEN_EQUAL_EQUAL, "==");
  TESTTK(TTy::TOKEN_GREATER, ">");
  TESTTK(TTy::TOKEN_GREATER_EQUAL, ">=");
  TESTTK(TTy::TOKEN_LESS, "<");
  TESTTK(TTy::TOKEN_LESS_EQUAL, "<=");

  TESTTK(TTy::KEYWORD_AND, "and");
  TESTTK(TTy::KEYWORD_CLASS, "class");
  TESTTK(TTy::KEYWORD_ELSE, "else");
  TESTTK(TTy::KEYWORD_FALSE, "false");
  TESTTK(TTy::KEYWORD_FOR, "for");
  TESTTK(TTy::KEYWORD_FUN, "fun");
  TESTTK(TTy::KEYWORD_IF, "if");
  TESTTK(TTy::KEYWORD_NIL, "nil");
  TESTTK(TTy::KEYWORD_OR, "or");
  TESTTK(TTy::KEYWORD_PRINT, "print");
  TESTTK(TTy::KEYWORD_RETURN, "return");
  TESTTK(TTy::KEYWORD_SUPER, "super");
  TESTTK(TTy::KEYWORD_THIS, "this");
  TESTTK(TTy::KEYWORD_TRUE, "true");
  TESTTK(TTy::KEYWORD_VAR, "var");
  TESTTK(TTy::KEYWORD_WHILE, "while");

#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}
