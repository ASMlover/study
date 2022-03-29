// Copyright (c) 2022 ASMlover. All rights reserved.
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
#include "errors.hh"
#include "scanner.hh"

namespace loxpp {

Scanner::Scanner(ErrorReporter& error_repoter, const str_t& source_bytes, const str_t& filename) noexcept
  : error_repoter_(error_repoter)
  , source_bytes_(source_bytes)
  , filename_(filename) {
}

std::vector<Token>& Scanner::scan_tokens() noexcept {
  while (!is_at_end()) {
    // TODO:

    start_pos_ = current_pos_;
    scan_token();
  }

  tokens_.push_back(Token::make_from_details(TokenType::TK_EOF, "", lineno_));
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
  case ';': add_token(TokenType::TK_SEMI); break;
  case '*': add_token(TokenType::TK_STAR); break;
  case '!': add_token(match('=') ? TokenType::TK_NE : TokenType::TK_NOT); break;
  case '=': add_token(match('=') ? TokenType::TK_EQEQ : TokenType::TK_EQ); break;
  case '<': add_token(match('=') ? TokenType::TK_LE : TokenType::TK_LT); break;
  case '>': add_token(match('=') ? TokenType::TK_GE : TokenType::TK_GT); break;
  case '/':
    if (match('/')) {
      // a comment goes until the end of the line.
      while (!is_at_end() && peek() != '\n')
        advance();
    }
    else {
      add_token(TokenType::TK_SLASH);
    }
    break;
  case ' ':
  case '\r':
  case '\t':
    // Ignore whitespace
    break;
  case '\n': ++lineno_; break;
  case '"': string(); break;
  default:
    error_repoter_.error("", lineno_, "Unexpected character.");
    break;
  }
}

void Scanner::string() noexcept {
  while (!is_at_end() && peek() != '"') {
    if (peek() == '\n')
      ++lineno_;
    advance();
  }

  if (is_at_end()) {
    error_repoter_.error("", lineno_, "Unterminated string.");
    return;
  }

  // The closing ".
  advance();

  str_t literal = gen_literal(start_pos_ + 1, current_pos_ - 1);
  add_token(TokenType::TK_STRING, literal);
}

}
