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

#include <string>

namespace lox {

enum TokenKind {
#define TOKDEF(k, s) k,
#include "kind_defs.h"
#undef TOKDEF
  NUM_TOKENS
};

const char* get_token_name(TokenKind kind);
bool contains_in_keywords(TokenKind kind);
TokenKind get_keyword_kind(const char* key);

class Token {
  TokenKind kind_{TokenKind::TK_UNKNOWN};
  std::string literal_;
  int lineno_{1};
public:
  Token(void) {
  }

  Token(TokenKind kind, const std::string& literal, int lineno)
    : kind_(kind), literal_(literal), lineno_(lineno) {
  }

  bool literal_equal(const Token& r) const {
    return literal_ == r.literal_;
  }

  TokenKind get_kind(void) const { return kind_; }
  const std::string& get_literal(void) const { return literal_; }
  int get_lineno(void) const { return lineno_; }

  std::string stringify(void) const;
  double as_numeric(void) const;
  std::string as_string(void) const;
};

std::ostream& operator<<(std::ostream& out, const Token& tok);

}
