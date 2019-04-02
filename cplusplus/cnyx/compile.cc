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
#include <cassert>
#include <functional>
#include <iostream>
#include <vector>
#include "lexer.hh"
#include "object.hh"
#include "vm.hh"
#include "compile.hh"

namespace nyx {

enum Precedence {
  NONE,
  ASSIGNMENT, // =
  OR, // or
  AND, // and
  EQUALITY, // == !=
  COMPARISON, // < <= > >=
  TERM, // + -
  FACTOR, // * /
  UNARY, // + - !
  CALL, // . () []
  PRIMARY,
};

class Compiler;

struct ParseRule {
  std::function<void (Compiler*, bool)> prefix;
  std::function<void (Compiler*, bool)> infix;
  Precedence precedence;
};

class Compiler : private UnCopyable {
  Compiler* enclosing_{};

  VM& vm_;
  Lexer& lex_;
  Token curr_;
  Token prev_;
  bool had_error_{};

  FunctionObject* function_{};

  void error(const std::string& message) {
    std::cerr << message << std::endl;
    had_error_ = true;
  }

  void emit_byte(std::uint8_t byte) {
    function_->append_code(byte);
  }

  void emit_bytes(std::uint8_t byte1, std::uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  std::uint8_t add_constant(Value constant) {
    function_->append_constant(constant);
    return static_cast<std::uint8_t>(function_->constants_count() - 1);
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto numeric_fn = [](Compiler* p, bool b) { p->numeric(b); };
    static auto binary_fn = [](Compiler* p, bool b) { p->binary(b); };

    static ParseRule _rules[] = {
      nullptr, nullptr, Precedence::NONE, // TK_ERROR
      nullptr, nullptr, Precedence::NONE, // TK_EOF
      nullptr, nullptr, Precedence::NONE, // TK_IDENTIFIER
      nullptr, nullptr, Precedence::NONE, // TK_STRINGLITERAL
      numeric_fn, nullptr, Precedence::NONE, // TK_NUMERICCONST

      nullptr, nullptr, Precedence::NONE, // TK_LPAREN
      nullptr, nullptr, Precedence::NONE, // TK_RPAREN
      nullptr, nullptr, Precedence::NONE, // TK_LBRACE
      nullptr, nullptr, Precedence::NONE, // TK_RBRACE
      nullptr, nullptr, Precedence::NONE, // TK_COMMA
      nullptr, nullptr, Precedence::NONE, // TK_DOT
      nullptr, nullptr, Precedence::NONE, // TK_SEMI
      nullptr, nullptr, Precedence::NONE, // TK_BANG
      nullptr, nullptr, Precedence::NONE, // TK_BANGEQUAL
      nullptr, nullptr, Precedence::NONE, // TK_EQUAL
      nullptr, nullptr, Precedence::NONE, // TK_EQUALEQUAL
      nullptr, nullptr, Precedence::NONE, // TK_GREATER
      nullptr, nullptr, Precedence::NONE, // TK_GREATEREQUAL
      nullptr, nullptr, Precedence::NONE, // TK_LESS
      nullptr, nullptr, Precedence::NONE, // TK_LESSEQUAL
      nullptr, binary_fn, Precedence::TERM, // TK_PLUS
      nullptr, binary_fn, Precedence::TERM, // TK_MINUS
      nullptr, binary_fn, Precedence::FACTOR, // TK_STAR
      nullptr, binary_fn, Precedence::FACTOR, // TK_SLASH

      nullptr, nullptr, Precedence::NONE, // KW_AND
      nullptr, nullptr, Precedence::NONE, // KW_CLASS
      nullptr, nullptr, Precedence::NONE, // KW_ELSE
      nullptr, nullptr, Precedence::NONE, // KW_FALSE
      nullptr, nullptr, Precedence::NONE, // KW_FOR
      nullptr, nullptr, Precedence::NONE, // KW_FUN
      nullptr, nullptr, Precedence::NONE, // KW_IF
      nullptr, nullptr, Precedence::NONE, // KW_NIL
      nullptr, nullptr, Precedence::NONE, // KW_OR
      nullptr, nullptr, Precedence::NONE, // KW_PRINT
      nullptr, nullptr, Precedence::NONE, // KW_RETURN
      nullptr, nullptr, Precedence::NONE, // KW_SUPER
      nullptr, nullptr, Precedence::NONE, // KW_THIS
      nullptr, nullptr, Precedence::NONE, // KW_TRUE
      nullptr, nullptr, Precedence::NONE, // KW_VAR
      nullptr, nullptr, Precedence::NONE, // KW_WHILE
    };
    return _rules[EnumUtil<TokenKind>::as_int(kind)];
  }

  void parse_precedence(Precedence precedence, bool can_assign) {
    advance();

    auto prefix = get_rule(prev_.get_kind()).prefix;
    if (!prefix) {
      // compiler error
      error("expected expression");
      return;
    }
    prefix(this, can_assign);

    while (precedence <= get_rule(curr_.get_kind()).precedence) {
      advance();
      auto infix = get_rule(prev_.get_kind()).infix;
      if (infix)
        infix(this, can_assign);
    }
  }

  void numeric(bool can_assign) {
    double value = prev_.as_numeric();
    std::uint8_t constant = add_constant(NumericObject::create(vm_, value));
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  void binary(bool can_assign) {
    auto oper_kind = prev_.get_kind();
    auto& rule = get_rule(oper_kind);

    // compile the right-hand operand
    parse_precedence(EnumUtil<Precedence>::as_enum(rule.precedence + 1), false);

    switch (oper_kind) {
    case TokenKind::TK_PLUS: emit_byte(OpCode::OP_ADD); break;
    case TokenKind::TK_MINUS: emit_byte(OpCode::OP_SUB); break;
    case TokenKind::TK_STAR: emit_byte(OpCode::OP_MUL); break;
    case TokenKind::TK_SLASH: emit_byte(OpCode::OP_DIV); break;
    default: assert(false); break; // unreachable
    }
  }
public:
  Compiler(VM& vm, Lexer& lex) : vm_(vm), lex_(lex) {
    function_ = FunctionObject::create(vm_);
  }

  inline bool had_error(void) const { return had_error_; }
  inline FunctionObject* get_function(void) const { return function_; }
  inline Compiler* get_enclosing(void) const { return enclosing_; }

  void gray_function(void) {
    vm_.gray_value(function_);
  }

  void finish_compiler(void) {
    emit_byte(OpCode::OP_RETURN);
  }

  void advance(void) {
    prev_ = curr_;
    curr_ = lex_.next_token();
  }

  void consume(TokenKind kind, const std::string& message) {
    if (curr_.get_kind() != kind)
      error(message);
    advance();
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT, true);
  }

  void statement(void) {
    expression();
    consume(TokenKind::TK_SEMI, "expected `;` after expression");
  }
};

static Compiler* _main_compiler = nullptr;

FunctionObject* Compile::compile(VM& vm, const std::string& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(vm, lex);
  _main_compiler = &c;

  c.advance();
  c.statement();
  c.finish_compiler();

  _main_compiler = nullptr;
  if (c.had_error())
    return nullptr;

  return c.get_function();
}

void gray_compiler_roots(void) {
  auto* compiler_iter = _main_compiler;
  while (compiler_iter != nullptr) {
    compiler_iter->gray_function();
    compiler_iter = compiler_iter->get_enclosing();
  }
}

}
