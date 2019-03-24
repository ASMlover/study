// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <string>
#include "common.hh"
#include "token.hh"

namespace nyx {

class Lexer : private UnCopyable {
  const std::string& source_bytes_;
  std::size_t begpos_{};
  std::size_t curpos_{};
  int lineno_{1};

  inline bool is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
  }

  inline bool is_alnum(char c) const {
    return std::isalnum(c) || c == '_';
  }

  inline std::string gen_literal(std::size_t begpos, std::size_t endpos) const {
    return source_bytes_.substr(begpos, endpos - begpos);
  }

  inline bool is_end(void) const {
    return curpos_ >= source_bytes_.size();
  }

  inline char peek(void) const {
    return curpos_ >= source_bytes_.size() ? 0 : source_bytes_[curpos_];
  }

  inline char peek_next(void) const {
    return curpos_ + 1 >= source_bytes_.size() ? 0 : source_bytes_[curpos_ + 1];
  }

  inline char advance(void) {
    return source_bytes_[curpos_++];
  }

  bool match(char expected);
  void skip_whitespace(void);
  Token make_token(TokenKind kind) const;
  Token make_token(TokenKind kind, const std::string& literal) const;
  Token error_token(const std::string& message = "") const;
  Token make_numeric(void);
  Token make_string(void);
  Token make_identifier(void);
public:
  Lexer(const std::string& source_bytes)
    : source_bytes_(source_bytes) {
  }

  Token next_token(void);
};

}
