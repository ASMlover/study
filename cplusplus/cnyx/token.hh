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

#include "common.hh"

namespace nyx {

enum class TokenKind {
#undef TOKDEF
#define TOKDEF(k, s) k,
#include "kinds_def.hh"
#undef TOKDEF
  NUM_KINDS
};

const char* get_token_name(TokenKind kind);
TokenKind get_keyword_kind(const char* key);

class Token : public Copyable {
  TokenKind kind_{TokenKind::TK_ERROR};
  str_t literal_;
  int lineno_{1};
public:
  Token(void) noexcept {}
  Token(TokenKind kind, const str_t& literal, int lineno) noexcept
    : kind_(kind), literal_(literal), lineno_(lineno) {
  }
  Token(const Token& r) noexcept
    : kind_(r.kind_), literal_(r.literal_), lineno_(r.lineno_) {
  }
  Token(Token&& r) noexcept
    : kind_(std::move(r.kind_))
    , literal_(std::move(r.literal_))
    , lineno_(std::move(r.lineno_)) {
  }

  Token& operator=(const Token& r) noexcept {
    if (this != &r) {
      kind_ = r.kind_;
      literal_ = r.literal_;
      lineno_ = r.lineno_;
    }
    return *this;
  }

  Token& operator=(Token&& r) noexcept {
    if (this != &r) {
      kind_ = std::move(r.kind_);
      literal_ = std::move(r.literal_);
      lineno_ = std::move(r.lineno_);
    }
    return *this;
  }

  inline bool is_equal(const Token& r) const { return literal_ == r.literal_; }
  inline TokenKind get_kind(void) const { return kind_; }
  inline str_t get_literal(void) const { return literal_; }
  inline int get_lineno(void) const { return lineno_; }
  inline double as_numeric(void) const { return std::atof(literal_.c_str()); }
  inline str_t as_string(void) const { return literal_; }

  str_t stringify(void) const;

  static Token custom_token(const str_t& literal = "") {
    return Token{TokenKind::TK_STRINGLITERAL, literal, 0};
  }
};

std::ostream& operator<<(std::ostream& out, const Token& tok);

}
