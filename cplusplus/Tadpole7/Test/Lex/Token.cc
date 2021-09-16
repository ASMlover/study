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
#include <Common/Harness.hh>
#include <Lex/Token.hh>

TADPOLE_TEST(TadpoleToken) {
  using TK = Tadpole::Lex::TokenKind;
#define NEWTK2(k, s)    Tadpole::Lex::Token(k, s, 0)
#define NEWTK3(k, s, l) Tadpole::Lex::Token(k, s, l)
#define TESTEQ(a, b)    TADPOLE_CHECK_EQ(a, b)
#define TESTTK(k, s)    do {\
  auto t = NEWTK2(k, s);\
  TESTEQ(t.kind(), k);\
  TESTEQ(t.literal(), s);\
  TESTEQ(t.lineno(),  0);\
} while (false)
#define TESTID(id)      TESTTK(TK::TK_IDENTIFIER, id)
#define TESTNUM(n)      do {\
  auto t = NEWTK2(TK::TK_NUMERIC, #n);\
  TESTEQ(t.kind(), TK::TK_NUMERIC);\
  TESTEQ(t.as_numeric(), n);\
  TESTEQ(t.lineno(), 0);\
} while (false)
#define TESTSTR(s, l)   do {\
  auto t = NEWTK3(TK::TK_STRING, s, l);\
  TESTEQ(t.kind(), TK::TK_STRING);\
  TESTEQ(t.as_string(), s);\
  TESTEQ(a.lineno(), l);\
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

  {
    // test for IDENTIFIER
    TESTID("foo");
    TESTID("FOO");
    TESTID("foo_");
    TESTID("FOO_");
    TESTID("foo__");
    TESTID("FOO__");
    TESTID("foo1");
    TESTID("FOO1");
    TESTID("foo1_");
    TESTID("FOO1_");
    TESTID("foo1__");
    TESTID("FOO1__");
    TESTID("foo_1");
    TESTID("FOO_1");
    TESTID("foo__1");
    TESTID("FOO__1");
    TESTID("foo_1_");
    TESTID("FOO_1_");
    TESTID("foo_1__");
    TESTID("FOO_1__");
    TESTID("foo__1_");
    TESTID("FOO__1_");
    TESTID("foo__1__");
    TESTID("FOO__1__");
    TESTID("_foo");
    TESTID("_FOO");
    TESTID("_foo_");
    TESTID("_FOO_");
    TESTID("_foo__");
    TESTID("_FOO__");
    TESTID("_foo1");
    TESTID("_FOO1");
    TESTID("_foo1__");
    TESTID("_FOO1__");
    TESTID("_foo_1");
    TESTID("_FOO_1");
    TESTID("_foo__1");
    TESTID("_FOO__1");
    TESTID("_foo_1_");
    TESTID("_FOO_1_");
    TESTID("_foo_1__");
    TESTID("_FOO_1__");
    TESTID("_foo__1_");
    TESTID("_FOO__1_");
    TESTID("_foo__1__");
    TESTID("_FOO__1__");
    TESTID("__foo");
    TESTID("__FOO");
    TESTID("__foo_");
    TESTID("__FOO_");
    TESTID("__foo__");
    TESTID("__FOO__");
    TESTID("__foo1");
    TESTID("__FOO1");
    TESTID("__foo1__");
    TESTID("__FOO1__");
    TESTID("__foo_1");
    TESTID("__FOO_1");
    TESTID("__foo__1");
    TESTID("__FOO__1");
    TESTID("__foo_1_");
    TESTID("__FOO_1_");
    TESTID("__foo_1__");
    TESTID("__FOO_1__");
    TESTID("__foo__1_");
    TESTID("__FOO__1_");
    TESTID("__foo__1__");
    TESTID("__FOO__1__");
    TESTID("_");
    TESTID("__");
    TESTID("___");
    TESTID("____");
    TESTID("_1");
    TESTID("_1_");
    TESTID("_1__");
    TESTID("__1");
    TESTID("__1_");
    TESTID("__1__");
  }

  {
    // test for NUMERIC
    TESTNUM(100);
    TESTNUM(-100);
    TESTNUM(+100);
    TESTNUM(1.2345);
    TESTNUM(-1.2345);
    TESTNUM(+1.2345);
    TESTNUM(100.0);
    TESTNUM(-100.0);
    TESTNUM(+100.0);
  }

#undef TESTSTR
#undef TESTNUM
#undef TESTID
#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}
