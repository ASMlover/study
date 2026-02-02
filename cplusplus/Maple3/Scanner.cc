// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include "Scanner.hh"

namespace ms {

Scanner::Scanner(const str_t& source_code, const str_t& source_fname) noexcept
  : source_code_{source_code}, source_fname_{source_fname} {
}

std::vector<Token> Scanner::scan_tokens() noexcept {
  tokens_.clear();

  for (; !is_at_end(); ) {
    start_ = current_;
    scan_token();
  }
  tokens_.emplace_back(TokenType::TK_EOF, "", lineno_);

  return tokens_;
}

void Scanner::scan_token() noexcept {
  char c = advance();
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
    else if (match('*')) {
      bool closed = false;
      while (!is_at_end()) {
        if (peek() == '\n')
          ++lineno_;
        if (peek() == '*' && peek_next() == '/') {
          advance();
          advance();
          closed = true;
          break;
        }
        advance();
      }
      if (!closed)
        add_token(TokenType::TK_ERR, "Unterminated block comment.");
    }
    else {
      add_token(TokenType::TK_SLASH);
    }
    break;
  case ' ':
  case '\r':
  case '\t':
    break;
  case '\n':
    ++lineno_;
    break;
  case '"':
    string();
    break;
  default:
    if (is_digit(c)) {
      number();
    }
    else if (is_alpha(c)) {
      identifier();
    }
    else {
      add_token(TokenType::TK_ERR, from_fmt("Unexpected character: `%c`", c));
    }
    break;
  }
}

void Scanner::string() noexcept {
  str_t literal;
  bool closed = false;

  while (!is_at_end()) {
    char c = advance();
    if (c == '"') {
      closed = true;
      break;
    }

    if (c == '\n')
      ++lineno_;

    if (c == '\\') {
      if (is_at_end()) {
        add_token(TokenType::TK_ERR, "Unterminated escape sequence.");
        return;
      }
      char esc = advance();
      switch (esc) {
      case 'n': literal.push_back('\n'); break;
      case 'r': literal.push_back('\r'); break;
      case 't': literal.push_back('\t'); break;
      case '\\': literal.push_back('\\'); break;
      case '"': literal.push_back('"'); break;
      default: literal.push_back(esc); break;
      }
    }
    else {
      literal.push_back(c);
    }
  }

  if (!closed) {
    add_token(TokenType::TK_ERR, "Unterminated string.");
    return;
  }

  add_token(TokenType::TK_STRING, literal);
}

void Scanner::number() noexcept {
  while (is_digit(peek()))
    advance();

  if (peek() == '.' && is_digit(peek_next())) {
    advance();
    while (is_digit(peek()))
      advance();
  }

  add_token(TokenType::TK_NUMBER);
}

void Scanner::identifier() noexcept {
  while (is_alnum(peek()))
    advance();

  str_t text = gen_literal(start_, current_);
  add_token(get_keyword_type(text), text);
}

}
