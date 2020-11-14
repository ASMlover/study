// Copyright (c) 2020 ASMlover. All rights reserved.
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

#include "common.hh"
#include "token.hh"
#include "function_object.hh"
#include "compiler_internal.hh"
#include "fun_compiler.hh"

namespace tadpole {

class VM;
class Lexer;
class FunCompiler;

class GlobalParser final : private UnCopyable {
  VM& vm_;
  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool had_error_{};
  bool panic_mode_{};

  FunCompiler* curr_compiler_{};

  inline void error_at_current(const str_t& msg) noexcept { error_at(curr_, msg); }
  inline void error(const str_t& msg) noexcept { error_at(prev_, msg); }
  inline Chunk* curr_chunk() const noexcept { return  curr_compiler_->fn()->chunk(); }
  inline bool check(TokenKind kind) const noexcept { return curr_.kind() == kind; }

  void error_at(const Token& tok, const str_t& msg) noexcept;
};

}
