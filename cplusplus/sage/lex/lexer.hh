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
#include <vector>
#include "../common/common.hh"
#include "token.hh"

namespace sage {

class ErrorReport;

class Lexer : private UnCopyable {
  ErrorReport& err_report_;

  const std::string& source_bytes_;
  std::string fname_;
  std::size_t begpos_{};
  std::size_t curpos_{};
  int lineno_{1};
  std::vector<Token> tokens_;

  bool is_alpha(char c) const;
  bool is_alnum(char c) const;
  std::string gen_literal(std::size_t begpos, std::size_t endpos) const;
  bool is_end(void) const;
  char advance(void);
  bool match(char expected);
  char peek(void) const;
  char peek_next(void) const;

  void next_token(void);
  void make_token(TokenKind kind);
  void make_token(TokenKind kind, const std::string& literal);
  void skip_comment(void);
  void make_string(void);
  void make_numeric(void);
  void make_identifier(void);
public:
  Lexer(ErrorReport& err_report,
      const std::string& source_bytes, const std::string& fname = "");

  std::vector<Token>& parse_tokens(void);
};

}
