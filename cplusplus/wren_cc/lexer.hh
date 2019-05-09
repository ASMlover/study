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
#include "common.hh"
#include "token.hh"

namespace wrencc {

class Lexer : private UnCopyable {
  const str_t& source_bytes_;
  sz_t begpos_{};
  sz_t curpos_{};
  int lineno_{1};

  inline bool is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
  }

  inline bool is_alnum(char c) const {
    return std::isalnum(c) || c == '_';
  }

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const {
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

  inline bool match(char expected) {
    if (is_end() || source_bytes_[curpos_] != expected)
      return false;
    ++curpos_;
    return true;
  }

  void skip_whitespace(void);
  Token make_token(TokenKind kind, const str_t& literal);
  Token make_token(TokenKind kind);
  Token make_token(TokenKind kind, int lineno);
  Token make_error(const str_t& error_message);
  Token make_identifier(void);
  Token make_numeric(void);
  Token make_string(void);
  Token make_embed(void);
public:
  Lexer(const str_t& source_bytes)
    : source_bytes_(source_bytes) {
  }

  Token next_token(void);
};

}
