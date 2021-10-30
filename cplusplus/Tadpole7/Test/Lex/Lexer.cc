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
#include <Lex/Lexer.hh>

TADPOLE_TEST(TadpoleLexer) {
  using TK = Tadpole::Lex::TokenKind;

#define TESTEQ(k) TADPOLE_CHECK_EQ(lex.next_token().kind(), k)
#define DUMPLEX() do {\
  for (;;) {\
    auto t = lex.next_token();\
    std::fprintf(stdout, "%-24s | %-32s | %d\n",\
        Tadpole::Lex::get_kind_name(t.kind()),\
        t.as_cstring(),\
        t.lineno());\
    if (t.kind() == TK::TK_EOF)\
      break;\
  }\
} while (false)

  {
    std::string s =
      "print(\"Hello, world !\")";
    Tadpole::Lex::Lexer lex(s);
    DUMPLEX();
  }

  {
    std::string s = "var a = 1.234;";
    Tadpole::Lex::Lexer lex(s);
    TESTEQ(TK::KW_VAR);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_EQ);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "print(\"33 + 56.7 =\", 33 + 56.7);";
    Tadpole::Lex::Lexer lex(s);
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
    std::string s = "fn show_message(msg) { print(msg); }";
    Tadpole::Lex::Lexer lex(s);
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
    std::string s = "fn do_calc() { print(22 + 33.5 * 8.12 / 9.13 - (99 + 88.17)); }";
    Tadpole::Lex::Lexer lex(s);
    TESTEQ(TK::KW_FN);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_LPAREN);
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
    TESTEQ(TK::TK_LPAREN);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_PLUS);
    TESTEQ(TK::TK_NUMERIC);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_RPAREN);
    TESTEQ(TK::TK_SEMI);
    TESTEQ(TK::TK_RBRACE);
    TESTEQ(TK::TK_EOF);
  }

  {
    std::string s = "var a = nil; print(a, true, false, nil);";
    Tadpole::Lex::Lexer lex(s);
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

  {
    std::string s = "var s =\"ERROR LEXER";
    Tadpole::Lex::Lexer lex(s);
    TESTEQ(TK::KW_VAR);
    TESTEQ(TK::TK_IDENTIFIER);
    TESTEQ(TK::TK_EQ);
    TESTEQ(TK::TK_ERR);
    TESTEQ(TK::TK_EOF);
  }

#undef DUMPLEX
#undef TESTEQ

}
