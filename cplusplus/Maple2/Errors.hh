// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Common.hh"
#include "Token.hh"

namespace ms {

class ErrorReporter final : private UnCopyable {
  bool had_error_{};

  void report(const str_t& fname, int lineno, const str_t& where, const str_t& message) noexcept;
public:
  inline bool had_error() const noexcept { return had_error_; }

  void error(const str_t& fname, int lineno, const str_t& message) noexcept;
  void error(const Token& token, const str_t& message) noexcept;
};

class RuntimeError final : public Copyable, public std::exception {
  const Token token_;
  str_t message_;

  virtual cstr_t what() const noexcept override {
    return message_.c_str();
  }
public:
  RuntimeError(const Token& token, const str_t& message) noexcept
    : token_{token}, message_{message} {
  }

  inline const Token& token() const noexcept { return token_; }
  inline const str_t& message() const noexcept { return message_; }
};

}
