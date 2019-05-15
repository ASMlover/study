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
#include <cstdarg>
#include <functional>
#include <iostream>
#include <vector>
#include "lexer.hh"
#include "vm.hh"
#include "compiler.hh"

namespace wrencc {

class Compiler;
using CompileFn = std::function<void (Compiler*, const Token&)>;

enum class Precedence {
  NONE,
  LOWEST,

  EQUALITY, // == !=
  COMPARISON, // < <= > >=
  BITWISE, // | &
  TERM, // + -
  FACTOR, // * / %
  CALL, // ()
};

struct InfixCompiler {
  CompileFn fn;
  Precedence precedence;
};

class Compiler : private UnCopyable {
  VM& vm_;

  Lexer& lex_;
  Token prev_;
  Token curr_;

  // the block being comipled
  BlockObject* block_{};
  int codes_count_{};

  SymbolTable locals_;

  bool had_error_{};

  void error(const char* format, ...) {
    had_error_ = true;
    std::cerr << "Compile ERROR on " << curr_ << " : ";

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    std::cerr << std::endl;
  }

  template <typename T> inline void emit_byte(T b) {
    block_->add_code(b);
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) {
    emit_byte(b1);
    emit_byte(b2);
  }

  inline void emit_constant(Value v) {
    u8_t b = block_->add_constant(v);
    emit_bytes(Code::CONSTANT, b);
  }

  void numeric(const Token& tok) {
    Value constant = NumericObject::make_numeric(tok.as_numeric());

    emit_constant(constant);
  }

  bool match(TokenKind expected) {
    if (curr_.kind() != expected)
      return false;

    advance();
    return true;
  }

  void consume(TokenKind expected) {
    advance();

    if (prev_.kind() != expected)
      error("expected %d, got %d", expected, prev_.kind());
  }

  void advance(void) {
    prev_ = curr_;
    curr_ = lex_.next_token();
  }

  void statement(void) {
    if (match(TokenKind::KW_CLASS)) {
      consume(TokenKind::TK_IDENTIFIER);

      int local = locals_.add(prev_.as_string());
      if (local == -1)
        error("local variable is already defined");

      // create the empty class
      emit_byte(Code::CLASS);
      // store it in it's name
      emit_bytes(Code::STORE_LOCAL, local);

      // compile the method definitions
      consume(TokenKind::TK_LBRACE);
      // TODO: function definieions
      consume(TokenKind::TK_RBRACE);

      return;
    }

    if (match(TokenKind::KW_VAR)) {
      consume(TokenKind::TK_IDENTIFIER);
      int local = locals_.add(prev_.as_string());

      if (local == -1)
        error("local variable is already defined");

      consume(TokenKind::TK_EQ);

      // compile the initializer
      expression();
      emit_bytes(Code::STORE_LOCAL, local);
      return;
    }

    expression();
  }

  void expression(void) {
    call();
  }

  void call(void) {
    primary();
    if (match(TokenKind::TK_DOT)) {
      consume(TokenKind::TK_IDENTIFIER);
      int symbol = vm_.symbols().ensure(prev_.as_string());

      // compile the method call
      emit_bytes(Code::CALL, symbol);
    }
  }

  void primary(void) {
    if (match(TokenKind::TK_IDENTIFIER)) {
      int local = locals_.get(prev_.as_string());
      if (local == -1)
        error("unkonwn variable");

      emit_bytes(Code::LOAD_LOCAL, local);
      return;
    }
    if (match(TokenKind::TK_NUMERIC)) {
      numeric(prev_);
      return;
    }
  }
public:
  Compiler(VM& vm, Lexer& lex) noexcept
    : vm_(vm)
    , lex_(lex) {
    block_ = BlockObject::make_block();
  }

  ~Compiler(void) {
    // FIXME: fixed deallocate BlockObject by GC
  }

  BlockObject* run_compiler(void) {
    advance();

    for (;;) {
      statement();
      consume(TokenKind::TK_NL);

      if (match(TokenKind::TK_EOF))
        break;
      // emit_byte(Code::POP);
    }
    emit_byte(Code::END);

    block_->set_num_locals(locals_.count());
    return had_error_ ? nullptr : block_;
  }
};

BlockObject* compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(vm, lex);

  return c.run_compiler();
}

}
