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
#include "Errors.hh"
#include "Scanner.hh"

namespace ms {

Scanner::Scanner(ErrorReporter& error_reporter, const str_t& source_bytes, const str_t& filename) noexcept
  : error_reporter_(error_reporter), source_bytes_{source_bytes}, filename_{filename} {
}

std::vector<Token>& Scanner::scan_tokens() noexcept {
  tokens_.clear();

  while (!is_at_end()) {
    start_pos_ = current_pos_;
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
  }
}

void Scanner::string() noexcept {
  while (!is_at_end() && peek() != '"') {
    if (peek() == '\n')
      ++lineno_;

    advance();
  }

  if (is_at_end()) {
    error_reporter_.error("", lineno_, "Unterminated string.");
    return;
  }

  // the closing ".
  advance();

  add_token(TokenType::TK_STRING, gen_literal(start_pos_ + 1, current_pos_ - 1));
}

void Scanner::number() noexcept {}
void Scanner::identifier() noexcept {}

}
