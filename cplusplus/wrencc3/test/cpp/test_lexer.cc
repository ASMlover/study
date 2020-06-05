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
#include "../../harness.hh"
#include "../../lexer.hh"

WRENCC_TEST(Lexer, wrencc::FakeTester) {
#define _CHECK(Kind) WRENCC_CHECK_EQ(lex.next_token().kind(), wrencc::TokenKind::Kind)

  {
    wrencc::Lexer lex("class A {}");
    _CHECK(KW_CLASS);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_LBRACE);
    _CHECK(TK_RBRACE);
    _CHECK(TK_EOF);
  }

  {
    wrencc::Lexer lex("var a = 56\nvar b = 56\nprint(a + b)");
    _CHECK(KW_VAR);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_EQ);
    _CHECK(TK_NUMERIC);
    _CHECK(TK_NL);
    _CHECK(KW_VAR);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_EQ);
    _CHECK(TK_NUMERIC);
    _CHECK(TK_NL);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_LPAREN);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_PLUS);
    _CHECK(TK_INDENTIFIER);
    _CHECK(TK_RPAREN);
    _CHECK(TK_EOF);
  }

#undef _CHECK
}
