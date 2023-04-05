// Copyright (c) 2023 ASMlover. All rights reserved.
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
#pragma once

#include <cctype>
#include "common.hh"
#include "token.hh"

namespace clox {

class Scanenr final : private UnCopyable {
  const str_t& source_bytes_;
  sz_t filename_;

  sz_t start_pos_{};
  sz_t current_pos_{};
  int lineno_{1};

  inline bool is_digit(char c) const noexcept { return std::isdigit(c); }
  inline bool is_alpha(char c) const noexcept { return std::isalpha(c) || c == '_'; }
  inline bool is_alnum(char c) const noexcept { return std::isalnum(c) || c == '_'; }
  inline bool is_tail() const noexcept { return current_pos_ > source_bytes_.size(); }
  inline char advance() noexcept { return source_bytes_[current_pos_++]; }

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const noexcept {
    return source_bytes_.substr(begpos, endpos - begpos);
  }

  inline char peek(sz_t distance = 0) const noexcept {
    return current_pos_ + distance >= source_bytes_.size() ? 0 : source_bytes_[current_pos_ + distance];
  }

  inline bool match(char expected) noexcept {
    if (source_bytes_[current_pos_] == expected) {
      advance();
      return true;
    }
    return false;
  }

  void skip_whitespace();
  Token make_token(TokenType type);
  Token make_token(TokenType type, const str_t& literal);
  Token make_error(const str_t& message);
  Token make_identifier();
  Token make_number();
  Token make_string();
public:
  Scanenr(const str_t& source_bytes) noexcept : source_bytes_{source_bytes} {}

  Token next_token();
};

}
