// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "common.hh"
#include "harness.hh"
#include "token.hh"

TADPOLE_TEST(TadpoleToken) {
  using TK  = tadpole::TokenKind;
#define NEWTK2(k, s)    tadpole::Token(k, s, 0)
#define NEWTK3(k, s, n) tadpole::Token(k, s, n)
#define TESTEQ(a, b)    TADPOLE_CHECK_EQ(a, b)
#define TEST_TK(k, s) do {\
  auto t = NEWTK2(k, s);\
  TESTEQ(t.kind(), k);\
  TESTEQ(t.literal(), s);\
  TESTEQ(t.lineno(), 0);\
} while (false)
#define TEST_STR(s, n) do {\
  auto t = NEWTK3(TK::TK_STRING, s, n);\
  TESTEQ(t.kind(), TK::TK_STRING);\
  TESTEQ(t.as_string(), s);\
  TESTEQ(t.lineno(), n);\
} while (false)

  TEST_TK(TK::TK_LPAREN, "(");
  TEST_TK(TK::TK_RPAREN, ")");
  TEST_TK(TK::TK_LBRACE, "{");
  TEST_TK(TK::TK_RBRACE, "}");
  TEST_TK(TK::TK_COMMA, ",");
  TEST_TK(TK::TK_MINUS, "-");
  TEST_TK(TK::TK_PLUS, "+");
  TEST_TK(TK::TK_SEMI, ";");
  TEST_TK(TK::TK_SLASH, "/");
  TEST_TK(TK::TK_STAR, "*");
  TEST_TK(TK::TK_EQ, "=");

  TEST_TK(TK::KW_FALSE, "false");
  TEST_TK(TK::KW_FN, "fn");
  TEST_TK(TK::KW_NIL, "nil");
  TEST_TK(TK::KW_TRUE, "true");
  TEST_TK(TK::KW_VAR, "var");

  TEST_TK(TK::TK_EOF, "EOF");
  TEST_TK(TK::TK_ERR, "ERR");

  {
    // test for IDENTIFIER
    TEST_TK(TK::TK_IDENTIFIER, "foo");
    TEST_TK(TK::TK_IDENTIFIER, "_foo");
    TEST_TK(TK::TK_IDENTIFIER, "__foo1");
    TEST_TK(TK::TK_IDENTIFIER, "bar");
    TEST_TK(TK::TK_IDENTIFIER, "_bar");
    TEST_TK(TK::TK_IDENTIFIER, "__bar1");
    TEST_TK(TK::TK_IDENTIFIER, "Count");
    TEST_TK(TK::TK_IDENTIFIER, "_Count");
    TEST_TK(TK::TK_IDENTIFIER, "Count_");
    TEST_TK(TK::TK_IDENTIFIER, "__init__");
    TEST_TK(TK::TK_IDENTIFIER, "__magic__");
  }

  {
    // test for NUMERIC
    auto t = NEWTK2(TK::TK_NUMERIC, "100");
    TESTEQ(t.kind(), TK::TK_NUMERIC);
    TESTEQ(t.as_numeric(), 100);
    TESTEQ(t.lineno(), 0);

    auto t2 = NEWTK2(TK::TK_NUMERIC, "3.14");
    TESTEQ(t2.kind(), TK::TK_NUMERIC);
    TESTEQ(t2.as_numeric(), 3.14);
    TESTEQ(t2.lineno(), 0);

    auto t3 = NEWTK2(TK::TK_NUMERIC, "-45");
    TESTEQ(t3.kind(), TK::TK_NUMERIC);
    TESTEQ(t3.as_numeric(), -45);
    TESTEQ(t3.lineno(), 0);

    auto t4 = NEWTK2(TK::TK_NUMERIC, "-67.89");
    TESTEQ(t4.kind(), TK::TK_NUMERIC);
    TESTEQ(t4.as_numeric(), -67.89);
    TESTEQ(t4.lineno(), 0);
  }

  {
    // test for STRING
    TEST_STR("Hello", 5);
    TEST_STR("XXX", 3);
    TEST_STR("World", 5);
    TEST_STR("This is String testing", 33);
  }

#undef TEST_STR
#undef TEST_TK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}
