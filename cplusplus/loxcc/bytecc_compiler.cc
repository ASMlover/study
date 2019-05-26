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
#include <functional>
#include <iostream>
#include "lexer.hh"
#include "bytecc_chunk.hh"
#include "bytecc_chunk.hh"
#include "bytecc_vm.hh"
#include "bytecc_compiler.hh"

namespace loxcc::bytecc {

enum class Precedence {
  NONE,
  ASSIGNMENT, // =
  OR, // or
  AND, // and
  EQUALITY, // == !=
  COMPARISON, // > >= < <=
  TERM, // + -
  FACTOR, // * /
  UNARY, // ! -
  CALL, // . ()
  PRIMARY,
};

inline Precedence operator+(Precedence a, int b) {
  return Xt::as_type<Precedence>(Xt::as_type<int>(a) + b);
}

struct ParseRule {
  std::function<void (GlobalParser*, bool)> prefix;
  std::function<void (GlobalParser*, bool)> infix;
  Precedence precedence;
};

struct LocalVar {
  Token name;
  int depth{};
  bool is_upvalue{};

  LocalVar(const Token& arg_name,
      int arg_depth = 0, bool arg_upvalue = false) noexcept
    : name(arg_name), depth(arg_depth), is_upvalue(arg_upvalue) {
  }
};

struct Upvalue {
  u8_t index{};
  bool is_local{};

  Upvalue(u8_t arg_index = 0, bool arg_local = false) noexcept
    : index(arg_index), is_local(arg_local) {
  }
};

enum class FunctionType {
  FUNCTION,
  CTOR,
  METHOD,
  TOP_LEVEL,
};

struct Compiler {
  Compiler* enclosing{};

  FunctionObject* fn{};
  FunctionType type{};

  std::vector<LocalVar> locals;
  std::vector<Upvalue> upvalues;

  int scope_depth{};

  Compiler(void) noexcept {}
  Compiler(Compiler* arg_enclosing, FunctionObject* arg_fn,
      FunctionType arg_type, int arg_depth = 0) noexcept
    : enclosing(arg_enclosing)
    , fn(arg_fn)
    , type(arg_type)
    , scope_depth(arg_depth) {
  }
};

struct ClassCompiler {
  ClassCompiler* enclosing{};

  Token name;
  bool has_superclass{};

  ClassCompiler(void) noexcept {}
  ClassCompiler(ClassCompiler* arg_enclosing,
      const Token& arg_name, bool has_super = false) noexcept
    : enclosing(arg_enclosing), name(arg_name), has_superclass(has_super) {
  }
};

class GlobalParser final : private UnCopyable {
  VM& vm_;
  Lexer& lex_;

  Compiler* curr_compiler_{};
  ClassCompiler* curr_class_{};

  Token curr_;
  Token prev_;

  bool had_error_{};
  bool panic_mode_{};

  inline Chunk* curr_chunk(void) const { return curr_compiler_->fn->chunk(); }
public:
};

FunctionObject* GlobalCompiler::compile(VM& vm, const str_t& source_bytes) {
  // TODO:
  return nullptr;
}

void GlobalCompiler::mark_roots(VM& vm) {
  // TODO:
}

}
