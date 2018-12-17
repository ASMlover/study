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
#include <vector>
#include "token.h"

class Scanner {
  std::string source_bytes_;
  std::size_t start_{};
  std::size_t current_{};
  int line_{1};

  std::vector<Token> tokens_;

  std::string current_lexeme(std::size_t begpos, std::size_t endpos);

  bool is_eof(void) const;
  bool is_alpha(char c) const;
  bool is_alnum(char c) const;
  char advance(void);
  bool match(char expected);
  char peek(void) const;
  char peek_next(void) const;
  void add_token(TokenType type);
  void scan_token(void);

  void resolve_slash(void);
  void resolve_char(bool wchar = false);
  void resolve_string(bool wstr = false);
  void resolve_number(bool real = false);
  void resolve_macro(void);
  void resolve_identifier(char begchar = 0);
  void resolve_dot_start(void);
  void resolve_greater_start(void);
  void resolve_less_start(void);
public:
  Scanner(const std::string& source);
  std::vector<Token> scan_tokens(void);
};
