// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <cctype>
#include <iostream>
#include "scanner.h"

Scanner::Scanner(const std::string& source)
  : source_bytes_(source) {
}

std::vector<Token> Scanner::scan_tokens(void) {
  while (!is_eof()) {
    start_ = current_;
    scan_token();
  }

  tokens_.push_back(Token(TOKEN_EOF, "", line_));
  return tokens_;
}

std::string Scanner::current_lexeme(std::size_t begpos, std::size_t endpos) {
  return source_bytes_.substr(begpos, endpos - begpos);
}

bool Scanner::is_eof(void) const {
  return current_ >= source_bytes_.size();
}

bool Scanner::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Scanner::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

char Scanner::advance(void) {
  return source_bytes_[current_++];
}

bool Scanner::match(char expected) {
  if (is_eof())
    return false;
  if (source_bytes_[current_] != expected)
    return false;

  ++current_;
  return true;
}

char Scanner::peek(void) const {
  if (current_ >= source_bytes_.size())
    return 0;
  return source_bytes_[current_];
}

char Scanner::peek_next(void) const {
  if (current_ + 1 >= source_bytes_.size())
    return 0;
  return source_bytes_[current_ + 1];
}

void Scanner::add_token(TokenType type) {
  auto lexeme = current_lexeme(start_, current_);
  tokens_.push_back(Token(type, lexeme, line_));
}

void Scanner::scan_token(void) {
  char c = advance();
  switch (c) {
  case '(': add_token(TOKEN_LPAREN); break;
  case ')': add_token(TOKEN_RPAREN); break;
  case '[': add_token(TOKEN_LBRACKET); break;
  case ']': add_token(TOKEN_RBRACKET); break;
  case '{': add_token(TOKEN_LBRACE); break;
  case '}': add_token(TOKEN_RBRACE); break;
  case ',': add_token(TOKEN_COMMA); break;
  case ';': add_token(TOKEN_SEMICOLON); break;
  case '?': add_token(TOKEN_QUESTION); break;
  case ':': add_token(TOKEN_COLON); break;
  case '.': resolve_dot_start(); break;
  case '+':
    add_token(match('=') ?
        TOKEN_PLUS_EQUAL : (match('+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS));
    break;
  case '-':
    if (match('='))
      add_token(TOKEN_MINUS_EQUAL);
    else if (match('-'))
      add_token(TOKEN_MINUS_MINUS);
    else if (match('>'))
      add_token(TOKEN_MINUS_GREATER);
    else
      add_token(TOKEN_MINUS);
    break;
  case '*': add_token(match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR); break;
  case '/': resolve_slash(); break;
  case '~': add_token(TOKEN_COMPLEMENT); break;
  case '%': add_token(match('=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO); break;
  case '#': resolve_macro(); break;
  case '&':
    add_token(match('=') ?
        TOKEN_AND_EQUAL : (match('&') ? TOKEN_LOGIC_AND : TOKEN_AND));
    break;
  case '|':
    add_token(match('=') ?
        TOKEN_OR_EQUAL : (match('|') ? TOKEN_LOGIC_OR : TOKEN_OR));
    break;
  case '^': add_token(match('=') ? TOKEN_XOR_EQUAL : TOKEN_XOR); break;
  case '!': add_token(match('=') ? TOKEN_NOT_EQUAL : TOKEN_LOGIC_NOT); break;
  case '=': add_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL); break;
  case '>': resolve_greater_start(); break;
  case '<': resolve_less_start(); break;
  case ' ':
  case '\t':
  case '\r':
    break;
  case '\n':
    ++line_;
    break;
  case '\'': resolve_char(); break;
  case '"': resolve_string(); break;
  default:
    if (std::isdigit(c))
      resolve_number();
    else if (is_alpha(c))
      resolve_identifier();
    else
      std::cerr << "unexpected character ..." << std::endl;
  }
}

void Scanner::resolve_slash(void) {
  if (match('=')) {
    add_token(TOKEN_SLASH_EQUAL);
  }
  else if (match('/')) {
    while (!is_eof() && peek() != '\n')
      advance();
  }
  else if (match('*')) {
    // TODO: block comments
  }
  else {
    add_token(TOKEN_SLASH);
  }
}

void Scanner::resolve_char(bool wchar) {
  // TODO: s-char
}

void Scanner::resolve_string(bool wstr) {
  // TODO: s-char-sequence
}

void Scanner::resolve_number(void) {
  // TODO: integer-number or floating-point-number
}

void Scanner::resolve_macro(void) {
  // TODO: ## or precompile-macros
}

void Scanner::resolve_identifier(char begchar) {
  while (is_alnum(peek()))
    advance();

  if (begchar == 'L') {
    if (peek() == '\'') {
      resolve_char(true);
      return;
    }
    else if (peek() == '"') {
      resolve_string(true);
      return;
    }
  }

  auto lexeme = current_lexeme(start_, current_).c_str();
  auto type = TOKEN_IDENTIFILER;
  if (is_keyword(lexeme))
    type = get_keyword_type(lexeme);
  add_token(type);
}

void Scanner::resolve_dot_start(void) {
  if (std::isdigit(peek())) {
    resolve_number(); // TODO: need pass dot ?
    return;
  }

  if (peek() == '.') {
    if (peek_next() == '.')
      add_token(TOKEN_VARARGS);
    else
      std::cerr << "unexpected character ..." << std::endl;
  }
  else {
    add_token(TOKEN_DOT);
  }
}

void Scanner::resolve_greater_start(void) {
  // TODO: solve > about operators, >>, >>=, >=
}

void Scanner::resolve_less_start(void) {
  // TODO: solve < abotu operators, <<, <<=, <=
}
