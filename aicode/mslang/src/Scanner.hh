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

#include "Token.hh"

namespace ms {

struct ScannerState {
  cstr_t start;
  cstr_t current;
  cstr_t line_start;
  int line;
  TokenType prev_type;
};

class Scanner {
  static constexpr int kMAX_INTERPOLATION_NESTING = 8;

  cstr_t start_{};
  cstr_t current_{};
  cstr_t line_start_{};
  int line_{1};
  int interp_braces_[kMAX_INTERPOLATION_NESTING]{};
  int interp_count_{0};
  TokenType prev_type_{TokenType::TOKEN_EOF};

  bool is_at_end() const noexcept;
  char advance() noexcept;
  char peek() const noexcept;
  char peek_next() const noexcept;
  bool match(char expected) noexcept;

  void skip_whitespace() noexcept;
  Token make_token(TokenType type) const noexcept;
  Token error_token(cstr_t message) const noexcept;
  Token scan_string() noexcept;
  Token scan_string_continuation() noexcept;
  Token scan_number() noexcept;
  Token scan_identifier() noexcept;
  TokenType identifier_type() const noexcept;
  TokenType check_keyword(int start, int length, cstr_t rest, TokenType type) const noexcept;
  static bool is_asi_trigger(TokenType type) noexcept;
public:
  Scanner() noexcept = default;
  explicit Scanner(strv_t source) noexcept;

  void init(strv_t source) noexcept;
  Token scan_token() noexcept;

  ScannerState save_state() const noexcept;
  void restore_state(const ScannerState& state) noexcept;
};

} // namespace ms
