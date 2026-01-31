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
#pragma once

#include <cctype>
#include <vector>
#include "Common.hh"
#include "Token.hh"

namespace ms {

class Scanner final : private UnCopyable {
  str_t source_code_;
  str_t source_fname_;
  std::vector<Token> tokens_;

  sz_t start_{};
  sz_t current_{};
  int lineno_{1};

  inline bool is_digit(char c) const noexcept { return std::isdigit(c); }
  inline bool is_alpha(char c) const noexcept { return std::isalpha(c) || c == '_'; }
  inline bool is_alnum(char c) const noexcept { return std::isalnum(c) || c == '_'; }

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const noexcept { return source_code_.substr(begpos, endpos - begpos); }

  inline bool is_at_end() const noexcept { return current_ >= source_code_.size(); }
  inline char advance() noexcept { return source_code_[current_++]; }
  inline char peek() const noexcept { return is_at_end() ? '\0' : source_code_[current_]; }
  inline char peek_next() const noexcept { return current_ + 1 >= source_code_.size() ? '\0' : source_code_[current_ + 1]; }

  inline bool match(char expected) noexcept {
    if (is_at_end() || source_code_[current_] != expected)
      return false;

    ++current_;
    return true;
  }

  inline void add_token(TokenType type, const str_t& literal) noexcept {
    tokens_.emplace_back(type, literal, lineno_);
  }

  inline void add_token(TokenType type) noexcept {
    add_token(type, gen_literal(start_, current_));
  }
};

}
