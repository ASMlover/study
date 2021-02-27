// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
#include "../common/harness.hh"
#include "lexer.hh"

TADPOLE_TEST(TadpoleLexer) {
  using TK = tadpole::TokenKind;
#define TESTEQ(k) TADPOLE_CHECK_EQ(lex.next_token().kind(), k)
#define DUMPLEX() do {\
    for (;;) {\
      auto t = lex.next_token();\
      std::fprintf(stdout, "%-24s | %-32s | %d\n",\
          tadpole::get_kind_name(t.kind()),\
          t.as_cstring(),\
          t.lineno());\
      if (t.kind() == TK::TK_EOF)\
        break;\
    }\
  } while (false)

  {
    std::string s =
      "print(\"Welcome to Tadpole !!!\");\nvar v = 34 * 56 / 22.45 + (88.76 - 27.4);";
    tadpole::Lexer lex(s);
    DUMPLEX();
  }

  {
    std::string s = "var a = 56;";
    tadpole::Lexer lex(s);
    TESTEQ(TK::KW_VAR);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_EQ);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "print(\"23 + 45 =\", 23 + 45);";
    tadpole::Lexer lex(s);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_STRING);
    TESTEQ(TK::TK_COMMA);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_PLUS);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "fn show_msg(msg) { print(msg); }";
    tadpole::Lexer lex(s);
    TESTEQ(TK::KW_FN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_LBRACE);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_RBRACE);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "fn calc(msg) { print(12 + 45 * 78.23 / 89.11 - 0.67); }";
    tadpole::Lexer lex(s);
    TESTEQ(TK::KW_FN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_LBRACE);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_PLUS);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_STAR);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_SLASH);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_MINUS);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_RBRACE);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "var a = nil; print(a, true, false, nil);";
    tadpole::Lexer lex(s);
    TESTEQ(TK::KW_VAR);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_EQ);
    TESTEQ(TK::KW_NIL);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_COMMA);
    TESTEQ(TK::KW_TRUE);
    TESTEQ(TK::TK_COMMA);
    TESTEQ(TK::KW_FALSE);
    TESTEQ(TK::TK_COMMA);
    TESTEQ(TK::KW_NIL);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_EOF);
  }

#undef DUMPLEX
#undef TESTEQ
}
