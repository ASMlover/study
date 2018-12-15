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
#include <unordered_map>
#include "token.h"

class Scanner {
  std::string source_;
  std::size_t start_{};
  std::size_t current_{};
  int line_{1};

  std::vector<Token> tokens_;
  std::unordered_map<std::string, TokenType> keywords_;

  double str2number(const std::string& s);
  bool is_alpha(char c) const;
  bool is_alnum(char c) const;
  std::string get_lexeme(const std::string& s, std::size_t beg, std::size_t end);

  bool is_at_end(void) const;
  char advance(void);
  bool match(char expected);
  char peek(void) const;
  char peek_next(void) const;
  void add_string(void);
  void add_number(void);
  void add_identifier(void);
  void add_token(TokenType type);
  void scan_token(void);
public:
  Scanner(const std::string& source);

  std::vector<Token> scan_tokens(void);
};
