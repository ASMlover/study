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
  skip_whitespace();

  start_pos_ = current_pos_;
  if (is_tail())
    return make_token(TokenType::TOKEN_EOF);

  char c = advance();
  if (is_alpha(c))
    return make_identifier();
  if (is_digit(c))
    return make_number();

#define _MKTK(c, k)             case c: return make_token(TokenType::TOKEN_##k)
#define _MKTK2(c1, c2, k1, k2)  case c1: return make_token(match(c2) ? TokenType::TOKEN_##k1 : TokenType::TOKEN_##k2)
  switch (c) {
  _MKTK('(', LEFT_PAREN);
  _MKTK(')', RIGHT_PAREN);
  _MKTK('{', LEFT_BRACE);
  _MKTK('}', RIGHT_BRACE);
  _MKTK(';', SEMICOLON);
  _MKTK(',', COMMA);
  _MKTK('.', DOT);
  _MKTK('-', MINUS);
  _MKTK('+', PLUS);
  _MKTK('/', SLASH);
  _MKTK('*', STAR);
  _MKTK2('!', '=', BANG_EQUAL, BANG);
  _MKTK2('=', '=', EQUAL_EQUAL, EQUAL);
  _MKTK2('<', '=', LESS_EQUAL, LESS);
  _MKTK2('>', '=', GREATER_EQUAL, GREATER);
  case '"': return make_string();
  }
#undef _MKTK2
#undef _MKTK

  return make_error("unexpected character");
}

void Scanenr::skip_whitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '\n': ++lineno_; advance(); break;
    case '/':
      {
        if (peek(1) == '/') {
          while (!is_tail() && peek() != '\n')
            advance();
        }
        else {
          return;
        }
      } break;
    default: return;
    }
  }
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
#define _MKCHAR(x, y) case x: c = y; advance(); break
  str_t literal;
  while (!is_tail() && peek() != '"') {
    char c = peek();
    switch (c) {
    case '\n': ++lineno_; break;
    case '\\':
      switch (peek()) {
      _MKCHAR('"', '"');
      _MKCHAR('\\', '\\');
      _MKCHAR('%', '%');
      _MKCHAR('0', '\0');
      _MKCHAR('a', '\a');
      _MKCHAR('b', '\b');
      _MKCHAR('f', '\f');
      _MKCHAR('n', '\n');
      _MKCHAR('r', '\r');
      _MKCHAR('t', '\t');
      _MKCHAR('v', '\v');
      }
      break;
    }
    literal.push_back(c);
    advance();
  }
#undef _MKCHAR

  if (is_tail())
    return make_error("unterminated string");

  advance();
  return make_token(TokenType::TOKEN_STRING, literal);
}

}
