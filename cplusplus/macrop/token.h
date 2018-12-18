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

#include <cstdlib>
#include <string>
#include <sstream>
#include "token_type.h"

enum NumberType {
  NUMBER_DEFAULT,
  NUMBER_8SYSTEM,
  NUMBER_16SYSTEM,
};

class Token {
  TokenType type_;
  std::string lexeme_;
  int line_;
  NumberType ntype_{};
  bool is_wide_{};
public:
  Token(TokenType type, const std::string& lexeme, int line)
    : type_(type)
    , lexeme_(lexeme)
    , line_(line) {
  }

  void set_number_type(NumberType type) {
    ntype_ = type;
  }

  NumberType get_number_type(void) const {
    return ntype_;
  }

  void set_wide(void) {
    is_wide_ = true;
  }

  bool is_wide(void) const {
    return is_wide_;
  }

  TokenType get_type(void) const {
    return type_;
  }

  std::string get_lexeme(void) const {
    return lexeme_;
  }

  int get_line(void) const {
    return line_;
  }

  std::string as_string(void) const {
    return lexeme_;
  }

  char as_char(void) const {
    return lexeme_[0];
  }

  int as_int(void) const {
    return std::atoi(lexeme_.c_str());
  }

  long as_long(void) const {
    return std::atol(lexeme_.c_str());
  }

  long long as_long_long(void) const {
    return std::atoll(lexeme_.c_str());
  }

  float as_float(void) const {
    return static_cast<float>(std::atof(lexeme_.c_str()));
  }

  double as_double(void) const {
    return std::atof(lexeme_.c_str());
  }

  std::string repr(void) const {
    std::stringstream ss;
    ss << token_type_as_string(type_) << "|" << lexeme_ << "|" << line_;
    return ss.str();
  }
};
