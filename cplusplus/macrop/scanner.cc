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
#include <cstdlib>
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
    if (std::isdigit(c)) {
      resolve_number();
    }
    else if (is_alpha(c)) {
      resolve_identifier();
    }
    else {
      std::cerr << "unexpected character: " << c << std::endl;
      std::abort();
    }
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
    while (!is_eof() && !(peek() == '*' && peek_next() == '/')) {
      if (peek() == '\n')
        ++line_;
      advance();
    }

    if (peek() == '*' && peek_next() == '/') {
      advance();
    }
    else {
      std::cerr << "invalid block comments" << std::endl;
      std::abort();
    }
  }
  else {
    add_token(TOKEN_SLASH);
  }
}

void Scanner::resolve_char(bool wchar) {
  if (peek_next() == '\'') {
    advance();

    auto lexeme = current_lexeme(start_ + 1, current_ - 1);
    Token t(wchar ? TOKEN_WCHAR_CONST : TOKEN_CHAR_CONST, lexeme, line_);
    t.set_wide();
    tokens_.push_back(t);
  }
  else {
    std::cerr << "invalid char token ..." << std::endl;
    std::abort();
  }
}

void Scanner::resolve_string(bool wstr) {
  while (!is_eof() && peek() != '"') {
    if (peek() == '\n')
      ++line_;
    advance();
  }

  if (is_eof()) {
    std::cerr << "invalid string at line: " << line_ << std::endl;
    std::abort();
  }
  advance();

  auto lexeme = current_lexeme(start_ + 1, current_ - 1);
  Token t(wstr ? TOKEN_WSTR_CONST : TOKEN_STR_CONST, lexeme, line_);
  t.set_wide();
  tokens_.push_back(t);
}

void Scanner::resolve_number(bool real) {
  auto floating_fn = [this]{
    if (peek() == 'f' || peek() == 'F') {
      advance();
      add_token(TOKEN_DOUBLE_CONST);
    }
    else if (peek() == 'l' || peek() == 'L') {
      advance();
      add_token(TOKEN_LONGDOUBLE_CONST);
    }
    add_token(TOKEN_FLOAT_CONST);
  };

  while (std::isdigit(peek()))
    advance();

  if (peek() == '.') {
    if (real) {
      std::cerr << "invalid floating-point number ..." << std::endl;
      std::abort();
    }

    if (std::isdigit(peek_next())) {
      advance();
      while (std::isdigit(peek()))
        advance();
    }

    floating_fn();
    return;
  }

  if (real) {
    floating_fn();
  }
  else {
    if (peek() == 'u' || peek() == 'U') {
      advance();
      if (peek() == 'l' || peek() == 'L') {
        advance();
        if (peek() == 'l' || peek() == 'L') {
          advance();
          add_token(TOKEN_ULONGLONG_CONST);
        }
        else {
          add_token(TOKEN_ULONG_CONST);
        }
      }
      else {
        add_token(TOKEN_UINT_CONST);
      }
    }
    else if (peek() == 'l' || peek() == 'L') {
      advance();
      if (peek() == 'l' || peek() == 'L') {
        advance();
        add_token(TOKEN_LONGLONG_CONST);
      }
      else {
        add_token(TOKEN_LONG_CONST);
      }
    }
    else {
      add_token(TOKEN_INT_CONST);
    }
  }
}

void Scanner::resolve_macro(void) {
  if (peek() == '#') {
    if (peek_next() == '#') {
      std::cerr << "invalid macro joint ..." << std::endl;
      std::abort();
    }
    else {
      advance();
      add_token(TOKEN_MACRO_JOINT);
    }
  }
  else {
    while (peek() == ' ' || peek() == '\t')
      advance();

    if (is_alpha(peek())) {
      start_ = current_;

      advance();
      while (is_alnum(peek()))
        advance();

      auto lexeme = (std::string("#") + current_lexeme(start_, current_));
      if (is_macro_keyword(lexeme.c_str())) {
        auto type = get_macro_keyword_type(lexeme.c_str());
        Token t(type, lexeme, line_);
        tokens_.push_back(t);
      }
      else {
        std::cerr << "invlaid macro identifier ..." << std::endl;
        std::abort();
      }
    }
  }
}

void Scanner::resolve_identifier(char begchar) {
  while (is_alnum(peek()))
    advance();

  if (begchar == 'L') {
    if (peek() == '\'') {
      advance();
      resolve_char(true);
      return;
    }
    else if (peek() == '"') {
      advance();
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
    resolve_number(true);
    return;
  }

  if (peek() == '.') {
    if (peek_next() == '.') {
      add_token(TOKEN_VARARGS);
    }
    else {
      std::cerr << "unexpected character: " << peek_next() << std::endl;
    }
  }
  else {
    add_token(TOKEN_DOT);
  }
}

void Scanner::resolve_greater_start(void) {
  if (match('='))
    add_token(TOKEN_GREATER_EQUAL);
  else if (match('>'))
    add_token(match('=') ? TOKEN_RSHIFT_EQUAL : TOKEN_RSHIFT);
  else
    add_token(TOKEN_GREATER);
}

void Scanner::resolve_less_start(void) {
  if (match('='))
    add_token(TOKEN_LESS_EQUAL);
  else if (match('<'))
    add_token(match('=') ? TOKEN_LSHIFT_EQUAL : TOKEN_LSHIFT);
  else
    add_token(TOKEN_LESS);
}
