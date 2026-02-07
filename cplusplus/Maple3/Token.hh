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

#include "Common.hh"

namespace ms {

enum class TokenType : i8_t {
  TOKENTYPE_BEG = -1,

#undef TOKDEF
#define TOKDEF(k, s) k,
#include "TokenTypes.hh"
#undef TOKDEF

  TOKENTYPE_END,
};

cstr_t get_type_name(TokenType type) noexcept;
TokenType get_keyword_type(const str_t& keyword) noexcept;

class Token final : public Copyable {
  TokenType type_{TokenType::TK_ERR};
  str_t literal_{};
  int lineno_{};
  int colno_{};
  int length_{};
public:
  Token() noexcept {}

  Token(TokenType type, const str_t& literal, int lineno = 0, int colno = 0, int length = 0) noexcept
    : type_{type}, literal_{literal}, lineno_{lineno}, colno_{colno}, length_{length} {
  }

  Token(const Token& r) noexcept
    : type_{r.type_}, literal_{r.literal_}, lineno_{r.lineno_}, colno_{r.colno_}, length_{r.length_} {
  }

  Token(Token&& r) noexcept
    : type_{std::move(r.type_)}, literal_{std::move(r.literal_)}, lineno_{std::move(r.lineno_)},
      colno_{std::move(r.colno_)}, length_{std::move(r.length_)} {
  }

  inline Token& operator=(const Token& r) noexcept {
    if (this != &r) {
      type_ = r.type_;
      literal_ = r.literal_;
      lineno_ = r.lineno_;
      colno_ = r.colno_;
      length_ = r.length_;
    }
    return *this;
  }

  inline Token& operator=(Token&& r) noexcept {
    if (this != &r) {
      type_ = std::move(r.type_);
      literal_ = std::move(r.literal_);
      lineno_ = std::move(r.lineno_);
      colno_ = std::move(r.colno_);
      length_ = std::move(r.length_);
    }
    return *this;
  }

  inline bool operator==(const Token& r) const noexcept {
    return this == &r ? true : literal_ == r.literal_;
  }

  inline bool operator!=(const Token& r) const noexcept {
    return !(*this == r);
  }

  inline bool is_equal_to(const Token& r) const noexcept { return *this == r; }
  inline bool is_equal_to(const str_t& s) const noexcept { return literal_ == s; }

  inline TokenType type() const noexcept { return type_; }
  inline const str_t& literal() const noexcept { return literal_; }
  inline int lineno() const noexcept { return lineno_; }
  inline int colno() const noexcept { return colno_; }
  inline int length() const noexcept { return length_; }
  inline double as_number() const noexcept { return std::atof(literal_.c_str()); }
  inline const str_t& as_string() const noexcept { return literal_; }
  inline cstr_t as_cstring() const noexcept { return literal_.c_str(); }

  str_t stringify() const;

  static Token from_literal(const str_t& literal) noexcept {
    return Token{TokenType::TK_STRING, literal};
  }

  static Token from_details(TokenType type, const str_t& literal, int lineno, int colno, int length) noexcept {
    return Token{type, literal, lineno, colno, length};
  }
};

inline std::ostream& operator<<(std::ostream& out, const Token& t) noexcept {
  return out << t.stringify();
}

}
