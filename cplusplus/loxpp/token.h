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

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include "token_type.h"

class Token {
  TokenType type_;
  std::string lexeme_;
  int line_;
public:
  Token(TokenType type, const std::string& lexeme, int line)
    : type_(type)
    , lexeme_(lexeme)
    , line_(line) {
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

  double as_number(void) const {
    return std::atof(lexeme_.c_str());
  }

  std::string repr(void) const {
    std::stringstream ss;

    ss << std::left << std::setw(16) << type_as_string(type_) << ":"
      << std::right << std::setw(16) << lexeme_ << "|"
      << std::right << std::setw(4) << line_;
    return ss.str();
  }
};
