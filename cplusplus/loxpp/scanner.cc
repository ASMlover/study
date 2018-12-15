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
#include <iostream>
#include "scanner.h"

bool Scanner::is_at_end(void) const {
  return current_ >= source_.size();
}

char Scanner::advance(void) {
  return source_[current_++];
}

bool Scanner::match(char expected) {
  if (is_at_end())
    return false;
  if (source_[current_] != expected)
    return false;

  ++current_;
  return true;
}

char Scanner::peek(void) {
  return source_[current_];
}

void Scanner::add_string(void) {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n')
      ++line_;
    advance();
  }

  // unterminated string
  if (is_at_end()) {
    std::cerr << "unterminated string: " << line_ << std::endl;
    return;
  }

  // the closing "
  advance();

  // trim the surround quotes
  auto lexeme = source_.substr(start_ + 1, current_ - 1);
  tokens_.push_back(Token(TOKEN_STRING, lexeme, line_));
}

void Scanner::add_token(TokenType type) {
  auto lexeme = source_.substr(start_, current_);
  tokens_.push_back(Token(type, lexeme, line_));
}

void Scanner::scan_token(void) {
  char c = advance();
  switch (c) {
  case '(': add_token(TOKEN_LEFT_PAREN); break;
  case ')': add_token(TOKEN_RIGHT_PAREN); break;
  case '{': add_token(TOKEN_LEFT_BRACE); break;
  case '}': add_token(TOKEN_RIGHT_BRACE); break;
  case ',': add_token(TOKEN_COMMA); break;
  case ';': add_token(TOKEN_SEMICOLON); break;
  case '.': add_token(TOKEN_DOT); break;
  case '+': add_token(TOKEN_PLUS); break;
  case '-': add_token(TOKEN_MINUS); break;
  case '*': add_token(TOKEN_STAR); break;
  case '!': add_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG); break;
  case '=': add_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL); break;
  case '<': add_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS); break;
  case '>': add_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER); break;
  case '/':
    if (match('/')) {
      // a comment goes until the end of the line
      while (peek() != '\n' && !is_at_end())
        advance();
    }
    else {
      add_token(TOKEN_SLASH);
    }
    break;
  case ' ':
  case '\r':
  case '\t':
    // ignore whitespace
    break;
  case '\n': ++line_; break;
  case '"': add_string(); break;
  default:
    std::cerr << "unexpected character, at: " << line_ << std::endl;
    break;
  }
}

std::vector<Token> Scanner::scan_tokens(void) {
  while (!is_at_end()) {
    // TODO:
    start_ = current_;
    scan_token();
  }

  tokens_.push_back(Token(TOKEN_EOF, "", line_));
  return tokens_;
}
