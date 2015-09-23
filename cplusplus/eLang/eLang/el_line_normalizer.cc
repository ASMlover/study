// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "el_base.h"
#include "el_token.h"
#include "el_line_normalizer.h"

namespace el {

bool LineNormalizer::IsInfinite(void) const {
  return lexer_.IsInfinite();
}

Ref<Token> LineNormalizer::ReadToken(void) {
  Ref<Token> r;

  while (r.IsNil()) {
    r = lexer_.ReadToken();

    switch (r->Type()) {
    case TokenType::TOKEN_LINE:
      if (eat_new_lines_)
        r.Clear();
      else
        eat_new_lines_ = true;
      break;
    case TokenType::TOKEN_IGNORE_LINE:
      r.Clear();
      eat_new_lines_ = true;
      break;
    case TokenType::TOKEN_LEFT_PAREN:
    case TokenType::TOKEN_LEFT_BRACKET:
    case TokenType::TOKEN_LEFT_BRACE:
    case TokenType::TOKEN_PIPE:
    case TokenType::TOKEN_SEMICOLON:
    case TokenType::TOKEN_ARROW:
    case TokenType::TOKEN_LONG_ARROW:
    case TokenType::TOKEN_OPERATOR:
    case TokenType::TOKEN_KEYWORD:
      eat_new_lines_ = true;
      break;
    default:
      eat_new_lines_ = true;
      break;
    }
  }

  return r;
}

}
