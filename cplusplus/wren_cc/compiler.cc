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
#include "lexer.hh"
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
  Block* block_{};
  int codes_count_{};

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
    Value constant = make_value(tok.as_numeric());

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
    expression();
    consume(TokenKind::TK_NL);
  }

  void expression(void) {
    call();
  }

  void call(void) {
    primary();
    if (match(TokenKind::TK_DOT)) {
      consume(TokenKind::TK_IDENTIFIER);
      int symbol = vm_.get_symbol(prev_.literal());
      std::cout << "symbol: " << symbol << std::endl;

      // compile the method call
      emit_bytes(Code::CALL, symbol);
    }
  }

  void primary(void) {
    if (match(TokenKind::TK_NUMERIC))
      numeric(prev_);
  }
public:
  Compiler(VM& vm, Lexer& lex) noexcept
    : vm_(vm)
    , lex_(lex) {
    block_ = new Block();
  }

  ~Compiler(void) {
    // FIXME: fixed allocated Block
    //
    // if (block_ != nullptr)
    //   delete block_;
  }

  Block* run_compiler(void) {
    advance();
    do {
      statement();
    } while (!match(TokenKind::TK_EOF));
    emit_byte(Code::END);

    return had_error_ ? nullptr : block_;
  }
};

Block* compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(vm, lex);

  return c.run_compiler();
}

}
