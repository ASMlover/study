// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include "lexer.hh"

namespace loxcc {

Token Lexer::next_token(void) {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  if (isalpha(c))
    return make_identifier();
  if (isdigit(c))
    return make_numeric();

  switch (c) {
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case ',': return make_token(TokenKind::TK_COMMA);
  case '.': return make_token(TokenKind::TK_DOT);
  case '-': return make_token(TokenKind::TK_MINUS);
  case '+': return make_token(TokenKind::TK_PLUS);
  case ';': return make_token(TokenKind::TK_SEMI);
  case '/': return make_token(TokenKind::TK_SLASH);
  case '*': return make_token(TokenKind::TK_STAR);
  case '!': return make_token(match('=') ? TokenKind::TK_BANGEQ : TokenKind::TK_BANG);
  case '=': return make_token(match('=') ? TokenKind::TK_EQEQ : TokenKind::TK_EQ);
  case '>': return make_token(match('=') ? TokenKind::TK_GTEQ : TokenKind::TK_GT);
  case '<': return make_token(match('=') ? TokenKind::TK_LTEQ : TokenKind::TK_LT);
  case '"': return make_string();
  }
  return make_error("unexpected charactor");
}

void Lexer::skip_whitespace(void) {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '\n': ++lineno_; advance(); break;
    case '/':
      if (peek_next() == '/') {
        while (!is_end() && peek() != '\n')
          advance();
      }
      else {
        return;
      }
      break;
    default: return;
    }
  }
}

Token Lexer::make_identifier(void) {
  while (isalnum(peek()))
    advance();

  str_t literal = gen_literal(begpos_, curpos_);
  return make_token(get_keyword_kind(literal), literal);
}

Token Lexer::make_numeric(void) {
  while (isdigit(peek()))
    advance();

  while (peek() == '.' && isdigit(peek_next())) {
    advance();
    while (isdigit(peek()))
      advance();
  }

  if (isalpha(peek()))
    return make_error("invalid numeric or identifier");
  return make_token(TokenKind::TK_NUMERIC);
}

Token Lexer::make_string(void) {
  str_t literal;
  while (!is_end() && peek() != '"') {
    char c = peek();
    switch (c) {
    case '\n': ++lineno_; break;
    case '\\':
      switch (peek_next()) {
      case '"': c = '"'; advance(); break;
      case '\\': c = '\\'; advance(); break;
      case '%': c = '%'; advance(); break;
      case '0': c = '\0'; advance(); break;
      case 'a': c = '\a'; advance(); break;
      case 'b': c = '\b'; advance(); break;
      case 'f': c = '\f'; advance(); break;
      case 'n': c = '\n'; advance(); break;
      case 'r': c = '\r'; advance(); break;
      case 't': c = '\t'; advance(); break;
      case 'v': c = '\v'; advance(); break;
      }
      break;
    }
    literal.push_back(c);
    advance();
  }

  if (is_end())
    return make_error("unterminated string");

  advance(); // closng the string "
  return make_token(TokenKind::TK_STRING, literal);
}

}
