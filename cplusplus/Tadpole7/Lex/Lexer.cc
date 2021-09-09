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
#include <Lex/Lexer.hh>

namespace Tadpole::Lex {

Token Lexer::next_token() {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_tail())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  if (is_alpha(c))
    return make_identifier();
  if (is_digit(c))
    return make_numeric();

#define _MKTK(c, k) case c: return make_token(TokenKind::TK_##k)
  switch (c) {
  _MKTK('(', LPAREN);
  _MKTK(')', RPAREN);
  _MKTK('{', LBRACE);
  _MKTK('}', RBRACE);
  _MKTK(',', COMMA);
  _MKTK('-', MINUS);
  _MKTK('+', PLUS);
  _MKTK(';', SEMI);
  _MKTK('/', SLASH);
  _MKTK('*', STAR);
  _MKTK('=', EQ);
  case '"': return make_string();
  }
#undef _MKTK

  return make_error("unexpected character");
}

void Lexer::skip_whitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '\n': ++lineno_; advance(); break;
    case '/':
      {
        if (peek(1) == '/') {
          while (peek() != '\n')
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

Token Lexer::make_token(TokenKind kind) {
  return  Token{kind, gen_literal(begpos_, curpos_), lineno_};
}

Token Lexer::make_token(TokenKind kind, const str_t& literal) {
  return Token{kind, literal, lineno_};
}

Token Lexer::make_error(const str_t& message) {
  return Token{TokenKind::TK_ERR, message, lineno_};
}

Token Lexer::make_identifier() {
  while (is_alnum(peek()))
    advance();

  str_t literal = gen_literal(begpos_, curpos_);
  return make_token(get_keyword_kind(literal), literal);
}

Token Lexer::make_numeric() {
  while (is_digit(peek()))
    advance();
  if (peek() == '.' && is_digit(peek(1))) {
    advance();
    while (is_digit(peek()))
      advance();
  }
  return make_token(TokenKind::TK_NUMERIC);
}

Token Lexer::make_string() {
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
  return make_token(TokenKind::TK_STRING, literal);
}

}
