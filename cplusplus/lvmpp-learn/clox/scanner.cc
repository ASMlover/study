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
#include "scanner.hh"

namespace clox {

Token Scanenr::next_token() {
  return make_error("unexpected character");
}

void Scanenr::skip_whitespace() {
}

Token Scanenr::make_token(TokenType type) {
  return Token::from_details(type, gen_literal(start_pos_, current_pos_), lineno_);
}

Token Scanenr::make_token(TokenType type, const str_t& literal) {
  return Token::from_details(type, literal, lineno_);
}

Token Scanenr::make_error(const str_t& message) {
  return Token::from_details(TokenType::TOKEN_ERROR, message, lineno_);
}

Token Scanenr::make_identifier() {
  while (is_alnum(peek()))
    advance();

  str_t literal = gen_literal(start_pos_, current_pos_);
  return make_token(get_keyword_type(literal), literal);
}

Token Scanenr::make_number() {
  while (is_digit(peek()))
    advance();
  if (peek() == '.' && is_digit(peek(1))) {
    advance();
    while (is_digit(peek()))
      advance();
  }
  return make_token(TokenType::TOKEN_NUMBER);
}

Token Scanenr::make_string() {
  return make_token(TokenType::TOKEN_STRING);
}

}
