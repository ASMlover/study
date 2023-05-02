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
#include <iostream>
#include "../harness.hh"
#include "../scanner.hh"

CLOX_TEST(Scanner) {
  using TTy = clox::TokenType;

#define TESTEQ(t)     CLOX_CHECK_EQ(scanner.scan_token().type(), t)
#define DUMPSCANNER() do {\
  for (;;) {\
    auto t = scanner.scan_token();\
    std::cout << t << std::endl;\
    if (t.type() == TTy::TOKEN_EOF)\
      break;\
  }\
} while (false)

  {
    clox::str_t s = "print(\"Hello, world!\")";
    clox::Scanenr scanner(s);
    DUMPSCANNER();
  }

  {
    clox::str_t s = "var a = 1.234";
    clox::Scanenr scanner(s);
    TESTEQ(TTy::KEYWORD_VAR);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_EQUAL);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_SEMICOLON);
    TESTEQ(TTy::TOKEN_EOF);
  }

  {
    clox::str_t s = "print(\"33 + 45.6 = \", 33 + 45.6);";
    clox::Scanenr scanner(s);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_STRING);
    TESTEQ(TTy::TOKEN_COMMA);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_PLUS);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_SEMICOLON);
    TESTEQ(TTy::TOKEN_EOF);
  }

  {
    clox::str_t s = "fun show_message(msg) { print(msg); }";
    clox::Scanenr scanner(s);
    TESTEQ(TTy::KEYWORD_FUN);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_LEFT_BRACE);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_SEMICOLON);
    TESTEQ(TTy::TOKEN_RIGHT_BRACE);
    TESTEQ(TTy::TOKEN_EOF);
  }

  {
    clox::str_t s = "fun do_calc() { print(22 + 34.5 * 11 / 87.34 - (54 + 0.89)); }";
    clox::Scanenr scanner(s);
    TESTEQ(TTy::KEYWORD_FUN);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_LEFT_BRACE);
    TESTEQ(TTy::TOKEN_IDENTIFIER);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_PLUS);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_STAR);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_SLASH);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_MINUS);
    TESTEQ(TTy::TOKEN_LEFT_PAREN);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_PLUS);
    TESTEQ(TTy::TOKEN_NUMBER);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_RIGHT_PAREN);
    TESTEQ(TTy::TOKEN_SEMICOLON);
    TESTEQ(TTy::TOKEN_RIGHT_BRACE);
    TESTEQ(TTy::TOKEN_EOF);
  }

#undef DUMPSCANNER
#undef TESTEQ
}
