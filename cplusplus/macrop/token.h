// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "token_kinds.h"

class Token {
  TokenKind kind_{TokenKind::UNKNOWN};
  std::string lexeme_;
  int line_{1};
public:
  bool is(TokenKind k) const { return kind_ == k; }
  bool is_not(TokenKind k) const { return kind_ != k; }
  bool is_literal(void) const {
    return kind_ >= TokenKind::INTCONST && kind_ <= TokenKind::U32STRINGLITERAL;
  }

  TokenKind get_kind(void) const { return kind_; }
  void set_kind(TokenKind k) { kind_ = k; }
  std::string get_lexeme(void) const { return lexeme_; }
  void set_lexeme(const std::string& s) { lexeme_ = s; }
  int get_line(void) const { return line_; }
  void set_line(int l) { line_ = l; }
  const char* get_name(void) const { return get_token_name(kind_); }
};
