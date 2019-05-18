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
#include "common.hh"
#include "token.hh"

namespace loxcc::interpret {

class ErrorReport final : private UnCopyable {
  bool had_error_{};

  void report(int lineno, const str_t& where, const str_t& message);
public:
  inline bool had_error(void) const { return had_error_; }
  inline void reset_error(void) { had_error_ = false; }

  inline void error(int lineno, const str_t& message) {
    report(lineno, "", message);
  }

  inline void error(const Token& tok, const str_t& message) {
    report(tok.lineno(), tok.literal(), message);
  }
};

class RuntimeError final : public Copyable, public std::exception {
  Token token_;
  str_t message_;

  inline const char* what(void) const throw() { return message_.c_str(); }
public:
  RuntimeError(const Token& tok, const str_t& msg) noexcept
    : token_(tok), message_(msg) {
  }

  inline const Token& token(void) const { return token_; }
  inline const str_t& message(void) const { return message_; }
};

}
