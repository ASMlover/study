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

#include <exception>
#include <string>
#include "common.hh"
#include "../lex/token.hh"

namespace sage {

class ErrorReport : private UnCopyable {
  bool had_error_{};

  void report(const std::string& fname,
      int lineno, const std::string& where, const std::string& message);
public:
  bool had_error(void) const { return had_error_; }
  void reset_error(void) { had_error_ = false; }

  void error(const std::string& fname, int lineno, const std::string& message);
  void error(const Token& tok, const std::string& message);
};

class RuntimeError : public Copyable, public std::exception {
  const Token token_;
  std::string message_;

  const char* what(void) const throw() { return message_.c_str(); }
public:
  RuntimeError(const Token& tok, const std::string& msg)
    : token_(tok), message_(msg) {
  }

  const Token get_token(void) const { return token_; }
  std::string get_message(void) const { return message_; }

  static Token virtual_token(const std::string& literal);
};

}
