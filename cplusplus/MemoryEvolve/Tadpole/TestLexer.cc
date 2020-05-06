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
#include <Core/MEvolve.hh>
#include <Tadpole/Lexer.hh>

_MEVO_TEST(TadpoleLexer, _mevo::FakeTester) {
  auto token_repr = [](const _mevo::tadpole::Token& tok) {
    std::fprintf(stdout, "%-16s|%4d|%s\n",
      _mevo::tadpole::get_token_name(tok.kind()),
      tok.lineno(),
      tok.literal().c_str());
  };

  _mevo::str_t source_bytes =
    "// test case for Lexer\n"
    "var a = 34;\n"
    "var b = 56.43;\n"
    "var c = (a + b) * a / b;\n"
    "print(\"calculate value is: \", c);\n"
    ;
  _mevo::tadpole::Lexer lex(source_bytes);
  for (;;) {
    auto tok = lex.next_token();
    token_repr(tok);

    if (tok.kind() == _mevo::tadpole::TokenKind::TK_EOF)
      break;
  }
}
