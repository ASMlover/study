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
#include "harness.hh"
#include "token.hh"

TADPOLE_TEST(TadpoleToken) {
  using TK = tadpole::TokenKind;
#define NEWTK2(k, s)    tadpole::Token(k, s, 0)
#define NEWTK3(k, s, n) tadpole::Token(k, s, n)
#define TEST_EQ(a, b)   TADPOLE_CHECK_EQ(a, b)
#define TEST_TK(k, s) {\
  auto t = NEWTK2(k, s);\
  TEST_EQ(t.kind(), k);\
  TEST_EQ(t.literal(), s);\
  TEST_EQ(t.lineno(), 0);\
}
#define TEST_STR(s, n) {\
  auto t = NEWTK3(TK::TK_STRING, s, n);\
  TEST_EQ(t.kind(), TK::TK_STRING);\
  TEST_EQ(t.as_string(), s);\
  TEST_EQ(t.lineno(), n);\
}

  TEST_TK(TK::TK_LPAREN, "(")
  TEST_TK(TK::TK_RPAREN, ")")
  TEST_TK(TK::TK_LBRACE, "{")
  TEST_TK(TK::TK_RBRACE, "}")
  TEST_TK(TK::TK_COMMA, ",")
  TEST_TK(TK::TK_MINUS, "-")
  TEST_TK(TK::TK_PLUS, "+")
  TEST_TK(TK::TK_SEMI, ";")
  TEST_TK(TK::TK_SLASH, "/")
  TEST_TK(TK::TK_STAR, "*")
  TEST_TK(TK::TK_EQ, "=")

  TEST_TK(TK::KW_FALSE, "false")
  TEST_TK(TK::KW_FN, "fn")
  TEST_TK(TK::KW_NIL, "nil")
  TEST_TK(TK::KW_TRUE, "true")
  TEST_TK(TK::KW_VAR, "var")

  TEST_TK(TK::TK_EOF, "")
  TEST_TK(TK::TK_ERR, "")

  // test for TK_IDENTIFIER
  TEST_TK(TK::TK_IDENTIFIER, "foo")
  TEST_TK(TK::TK_IDENTIFIER, "bar")
  TEST_TK(TK::TK_IDENTIFIER, "_Count")
  TEST_TK(TK::TK_IDENTIFIER, "_Foo1")
  TEST_TK(TK::TK_IDENTIFIER, "_Bar1")

  // test for TK_NUMERIC
  {
    auto t = NEWTK2(TK::TK_NUMERIC, "100");
    TEST_EQ(t.kind(), TK::TK_NUMERIC);
    TEST_EQ(t.as_numeric(), 100);
    TEST_EQ(t.lineno(), 0);

    auto t2 = NEWTK2(TK::TK_NUMERIC, "3.14");
    TEST_EQ(t2.kind(), TK::TK_NUMERIC);
    TEST_EQ(t2.as_numeric(), 3.14);
    TEST_EQ(t2.lineno(), 0);
  }

  // test for TK_STRING
  TEST_STR("Hello", 5)
  TEST_STR("This is a test string", 56)
  TEST_STR("xxx", 3)

#undef TEST_STR
#undef TEST_EQ
#undef NEWTK3
#undef NEWTK2
}
