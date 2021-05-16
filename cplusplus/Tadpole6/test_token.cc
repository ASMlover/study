// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include "harness.hh"
#include "token.hh"

TADPOLE_TEST(TadpoleToken) {
  using TK = tadpole::TokenKind;
#define NEWTK2(k, s)    tadpole::Token(k, s, 0)
#define NEWTK3(k, s, l) tadpole::Token(k, s, l)
#define TESTEQ(a, b)    TADPOLE_CHECK_EQ(a, b)
#define TESTTK(k, s)    do {\
  auto t = NEWTK2(k, s);\
  TESTEQ(t.kind(), k);\
  TESTEQ(t.literal(), s);\
  TESTEQ(t.lineno(), 0);\
} while (false)
#define TESTNUM(n)      do {\
  auto t = NEWTK2(TK::TK_NUMERIC, #n);\
  TESTEQ(t.kind(), TK::TK_NUMERIC);\
  TESTEQ(t.as_numeric(), n);\
  TESTEQ(t.lineno(), 0);\
} while (false)
#define TESTSTR(s, l)   do {\
  auto t = NEWTK3(TK::TK_STRING, s, l);\
  TESTEQ(t.kind(), TK::TK_STRING);\
  TESTEQ(t.literal(), s);\
  TESTEQ(t.lineno(), l);\
} while (false)

  TESTTK(TK::TK_LPAREN, "(");
  TESTTK(TK::TK_RPAREN, ")");
  TESTTK(TK::TK_LBRACE, "{");
  TESTTK(TK::TK_RBRACE, "}");
  TESTTK(TK::TK_COMMA, ",");
  TESTTK(TK::TK_MINUS, "-");
  TESTTK(TK::TK_PLUS, "+");
  TESTTK(TK::TK_SEMI, ";");
  TESTTK(TK::TK_SLASH, "/");
  TESTTK(TK::TK_STAR, "*");
  TESTTK(TK::TK_EQ, "=");

  TESTTK(TK::KW_FALSE, "false");
  TESTTK(TK::KW_FN, "fn");
  TESTTK(TK::KW_NIL, "nil");
  TESTTK(TK::KW_TRUE, "true");
  TESTTK(TK::KW_VAR, "var");

  TESTTK(TK::TK_EOF, "EOF");
  TESTTK(TK::TK_ERR, "ERR");

#undef TESTSTR
#undef TESTNUM
#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}
