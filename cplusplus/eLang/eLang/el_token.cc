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

namespace el {

std::ostream& operator<<(std::ostream& cout, const Token& token) {
  switch (token.Type()) {
  case TokenType::TOKEN_LEFT_PAREN:
    cout << "left paren"; break;
  case TokenType::TOKEN_RIGHT_PAREN:
    cout << "right paren"; break;
  case TokenType::TOKEN_LEFT_BRACKET:
    cout << "left bracket"; break;
  case TokenType::TOKEN_RIGHT_BRACKET:
    cout << "right bracket"; break;
  case TokenType::TOKEN_LEFT_BRACE:
    cout << "left brace"; break;
  case TokenType::TOKEN_RIGHT_BRACE:
    cout << "right brace"; break;
  case TokenType::TOKEN_AT:
    cout << "at"; break;
  case TokenType::TOKEN_DOT:
    cout << "dot"; break;
  case TokenType::TOKEN_HASH:
    cout << "hash"; break;
  case TokenType::TOKEN_PIPE:
    cout << "pipe"; break;
  case TokenType::TOKEN_SEMICOLON:
    cout << "semicolon"; break;
  case TokenType::TOKEN_ARROW:
    cout << "<-"; break;
  case TokenType::TOKEN_LONG_ARROW:
    cout << "<--"; break;
  case TokenType::TOKEN_SELF:
    cout << "self"; break;
  case TokenType::TOKEN_UNDEFINED:
    cout << "undefined"; break;
  case TokenType::TOKEN_RETURN:
    cout << "return"; break;
  case TokenType::TOKEN_NUMBER:
    cout << "number " << token.Number(); break;
  case TokenType::TOKEN_STRING:
    cout << "string '" << token.Text() << "'"; break;
  case TokenType::TOKEN_NAME:
    cout << "name '" << token.Text() << "'"; break;
  case TokenType::TOKEN_OPERATOR:
    cout << "operator '" << token.Text() << "'"; break;
  case TokenType::TOKEN_KEYWORD:
    cout << "keyword '" << token.Text() << "'"; break;
  case TokenType::TOKEN_LINE:
    cout << "newline"; break;
  case TokenType::TOKEN_IGNORE_LINE:
    cout << "ignore line"; break;
  case TokenType::TOKEN_EOF:
    cout << "eof"; break;
  default:
    cout << token.Text(); break;
  }

  return cout;
}

}
