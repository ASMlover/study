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
#pragma once

#include <vector>
#include "common.hh"
#include "token.hh"

namespace loxpp {

class ErrorReporter;

class Scanner final : private UnCopyable {
  ErrorReporter& error_repoter_;
  const str_t& source_bytes_;
  str_t filename_;
  std::vector<Token> tokens_;

  sz_t start_pos_{};
  sz_t current_pos_{};
  int lineno_{1};

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const noexcept {
    return source_bytes_.substr(begpos, endpos - begpos);
  }

  inline bool is_at_end() const noexcept { return current_pos_ >= source_bytes_.size(); }
  inline char advance() noexcept { return source_bytes_[current_pos_++]; }

  inline void add_token(TokenType type) noexcept {
    str_t literal = gen_literal(start_pos_, current_pos_);
    tokens_.push_back(Token::make_from_details(type, literal, lineno_));
  }

  void scan_token() noexcept;
public:
  Scanner(ErrorReporter& error_repoter, const str_t& source_bytes, const str_t& filename = "") noexcept;

  std::vector<Token>& scan_tokens() noexcept;
};

}
