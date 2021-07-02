// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include "value.hh"
#include "function_object.hh"
#include "chunk.hh"
#include "compiler_helper.hh"
#include "fn_compiler.hh"

namespace tadpole {

class Lexer;
class TadpoleVM;

class GlobalParser final : public IObjectIterator {
  static constexpr int kMaxArgs = 8;

  TadpoleVM& vm_;
  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool had_error_{};
  bool panic_mode_{};

  FnCompiler* curr_compiler_{};

  inline void error_at_current(const str_t& msg) noexcept { error_at(curr_, msg); }
  inline void error(const str_t& msg) noexcept { error_at(prev_, msg); }
  inline Chunk* curr_chunk() const noexcept { return curr_compiler_->fn()->chunk(); }
  inline bool check(TokenKind kind) const noexcept { return curr_.kind() == kind; }

  template <typename T> inline void emit_byte(T byte) noexcept {
    curr_chunk()->write(byte, prev_.lineno());
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) noexcept {
    emit_byte(b1);
    emit_byte(b2);
  }

  inline void emit_return() noexcept { emit_bytes(Code::NIL, Code::RETURN); }

  inline void emit_constant(const Value& v) noexcept {
    emit_bytes(Code::CONSTANT, curr_chunk()->add_constant(v));
  }

  const ParseRule& get_rule(TokenKind kind) const noexcept;

  void error_at(const Token& tok, const str_t& msg) noexcept;
  void advance();
  void consume(TokenKind kind, const str_t& msg);
  bool match(TokenKind kind);

  void init_compiler(FnCompiler* compiler, int scope_depth, FnType fn_type);
  FunctionObject* finish_compiler();
  void enter_scope();
  void leave_scope();
  u8_t identifier_constant(const Token& name) noexcept;
  u8_t parse_variable(const str_t& msg);
  void mark_initialized();
  void define_global(u8_t global);
  u8_t arguments();
  void named_variable(const Token& name, bool can_assign);
  void parse_precedence(Precedence precedence);
  void binary(bool can_assign);
  void call(bool can_assign);
  void grouping(bool can_assign);
  void literal(bool can_assign);
  void variable(bool can_assign);
  void numeric(bool can_assign);
  void string(bool can_assign);
  void block();
  void function(FnType fn_type);
  void synchronize();
  void expression();
  void declaration();
  void statement();
  void fn_decl();
  void var_decl();
  void expr_stmt();
public:
  GlobalParser(TadpoleVM& vm, Lexer& lex) noexcept;
  virtual ~GlobalParser() {}

  virtual void iter_objects(ObjectVisitor&& visitor) override;
  FunctionObject* compile();
};

}
