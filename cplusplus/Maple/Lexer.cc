// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Lexer.hh"

namespace ms {


std::vector<Token> Lexer::scan_tokens() noexcept {
  tokens_.clear();
  while (!is_at_end()) {
    start_pos_ = current_pos_;
    scan_token();
  }
  tokens_.emplace_back(TokenType::TK_EOF, "", lineno_);
  return std::move(tokens_);
}

void Lexer::scan_token() noexcept {
  skip_whitespace();

  start_pos_ = current_pos_;
  if (is_tail()) {
    add_token(TokenType::TK_EOF);
    return;
  }

  char c = advance();
  if (is_alpha(c)) {
    identifier();
    return;
  }
  if (is_digit(c)) {
    number();
    return;
  }

  switch (c) {
  case '(': add_token(TokenType::TK_LPAREN); break;
  case ')': add_token(TokenType::TK_RPAREN); break;
  case '{': add_token(TokenType::TK_LBRACE); break;
  case '}': add_token(TokenType::TK_RBRACE); break;
  case ',': add_token(TokenType::TK_COMMA); break;
  case '.': add_token(TokenType::TK_DOT); break;
  case '-': add_token(TokenType::TK_MINUS); break;
  case '+': add_token(TokenType::TK_PLUS); break;
  case ';': add_token(TokenType::TK_SEMICOLON); break;
  case '*': add_token(TokenType::TK_STAR); break;
  case '!': add_token(match('=') ? TokenType::TK_BANG_EQUAL : TokenType::TK_BANG); break;
  case '=': add_token(match('=') ? TokenType::TK_EQUAL_EQUAL : TokenType::TK_EQUAL); break;
  case '<': add_token(match('=') ? TokenType::TK_LESS_EQUAL : TokenType::TK_LESS); break;
  case '>': add_token(match('=') ? TokenType::TK_GREATER_EQUAL : TokenType::TK_GREATER); break;

  case '/':
    if (match('/')) {
      while (peek() != '\n' && !is_at_end())
        advance();
    }
    else {
      add_token(TokenType::TK_SLASH);
    }
    break;
  case '"': string(); break;
  default: error_token(std::format("Unexpected character `{}`", c)); break;
  }
}

void Lexer::skip_whitespace() noexcept {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '\n': ++lineno_; advance(); break;
    case '/':
      {
        if (peek_next() == '/') {
          while (peek() != '\n' && !is_at_end())
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

void Lexer::string() noexcept {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n')
      ++lineno_;
    advance();
  }

  if (is_at_end()) {
    error_token("Unterminated string.");
    return;
  }

  advance();
  str_t literal = gen_literal(start_pos_ + 1, current_pos_ - 1);
  add_token(TokenType::TK_STRING, literal);
}

void Lexer::number() noexcept {
  while (is_digit(peek()))
    advance();

  if (peek() == '.' && is_digit(peek_next())) {
    advance();

    while (is_digit(peek()))
      advance();
  }
  add_token(TokenType::TK_NUMBER);
}

void Lexer::identifier() noexcept {
  while (is_alnum(peek()))
    advance();

  str_t literal = gen_literal(start_pos_, current_pos_);
  add_token(get_keyword_type(literal), literal);
}

}
