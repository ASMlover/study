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
  case ',': add_token(TOKEN_COMMA); break;
  case ';': add_token(TOKEN_SEMICOLON); break;
  case '?': add_token(TOKEN_QUESTION); break;
  case ':': add_token(TOKEN_COLON); break;
  case '{':
    // TODO: solve initializer list or just left bracket
    break;
  case '.':
    // TODO: solve ... or dot
    break;
  case '+':
    // TODO: solve +=, ++
    break;
  }
}
