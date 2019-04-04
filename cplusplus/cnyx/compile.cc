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

  void error(const str_t& message) {
    std::cerr << message << std::endl;
    had_error_ = true;
  }

  void emit_byte(u8_t byte) {
    function_->append_code(byte);
  }

  void emit_bytes(u8_t byte1, u8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  u8_t add_constant(Value constant) {
    function_->append_constant(constant);
    return static_cast<u8_t>(function_->constants_count() - 1);
  }

  u8_t name_constant(void) {
    auto s = prev_.as_string();
    return add_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto grouping_fn = [](Compiler* p, bool b) { p->grouping(b); };
    static auto boolean_fn = [](Compiler* p, bool b) { p->boolean(b); };
    static auto numeric_fn = [](Compiler* p, bool b) { p->numeric(b); };
    static auto string_fn = [](Compiler* p, bool b) { p->string(b); };
    static auto variable_fn = [](Compiler* p, bool b) { p->variable(b); };
    static auto binary_fn = [](Compiler* p, bool b) { p->binary(b); };
    static auto unary_fn = [](Compiler* p, bool b) { p->unary(b); };

    static ParseRule _rules[] = {
      nullptr, nullptr, Precedence::NONE, // TK_ERROR
      nullptr, nullptr, Precedence::NONE, // TK_EOF
      variable_fn, nullptr, Precedence::NONE, // TK_IDENTIFIER
      string_fn, nullptr, Precedence::NONE, // TK_STRINGLITERAL
      numeric_fn, nullptr, Precedence::NONE, // TK_NUMERICCONST

      grouping_fn, nullptr, Precedence::NONE, // TK_LPAREN
      nullptr, nullptr, Precedence::NONE, // TK_RPAREN
      nullptr, nullptr, Precedence::NONE, // TK_LBRACE
      nullptr, nullptr, Precedence::NONE, // TK_RBRACE
      nullptr, nullptr, Precedence::NONE, // TK_COMMA
      nullptr, nullptr, Precedence::NONE, // TK_DOT
      nullptr, nullptr, Precedence::NONE, // TK_SEMI
      unary_fn, nullptr, Precedence::NONE, // TK_BANG
      nullptr, nullptr, Precedence::NONE, // TK_BANGEQUAL
      nullptr, nullptr, Precedence::NONE, // TK_EQUAL
      nullptr, nullptr, Precedence::NONE, // TK_EQUALEQUAL
      nullptr, binary_fn, Precedence::COMPARISON, // TK_GREATER
      nullptr, binary_fn, Precedence::COMPARISON, // TK_GREATEREQUAL
      nullptr, binary_fn, Precedence::COMPARISON, // TK_LESS
      nullptr, binary_fn, Precedence::COMPARISON, // TK_LESSEQUAL
      nullptr, binary_fn, Precedence::TERM, // TK_PLUS
      unary_fn, binary_fn, Precedence::TERM, // TK_MINUS
      nullptr, binary_fn, Precedence::FACTOR, // TK_STAR
      nullptr, binary_fn, Precedence::FACTOR, // TK_SLASH

      nullptr, nullptr, Precedence::NONE, // KW_AND
      nullptr, nullptr, Precedence::NONE, // KW_CLASS
      nullptr, nullptr, Precedence::NONE, // KW_ELSE
      boolean_fn, nullptr, Precedence::NONE, // KW_FALSE
      nullptr, nullptr, Precedence::NONE, // KW_FOR
      nullptr, nullptr, Precedence::NONE, // KW_FUN
      nullptr, nullptr, Precedence::NONE, // KW_IF
      nullptr, nullptr, Precedence::NONE, // KW_NIL
      nullptr, nullptr, Precedence::NONE, // KW_OR
      nullptr, nullptr, Precedence::NONE, // KW_PRINT
      nullptr, nullptr, Precedence::NONE, // KW_RETURN
      nullptr, nullptr, Precedence::NONE, // KW_SUPER
      nullptr, nullptr, Precedence::NONE, // KW_THIS
      boolean_fn, nullptr, Precedence::NONE, // KW_TRUE
      nullptr, nullptr, Precedence::NONE, // KW_VAR
      nullptr, nullptr, Precedence::NONE, // KW_WHILE
    };
    return _rules[Xenum::as_int<int>(kind)];
  }

  void parse_precedence(Precedence precedence) {
    advance();

    auto prefix = get_rule(prev_.get_kind()).prefix;
    if (!prefix) {
      // compiler error
      error("expected expression");
      return;
    }
    bool can_assign = precedence <= Precedence::ASSIGNMENT;
    prefix(this, can_assign);

    while (precedence <= get_rule(curr_.get_kind()).precedence) {
      advance();
      auto infix = get_rule(prev_.get_kind()).infix;
      if (infix)
        infix(this, can_assign);
    }
  }

  void boolean(bool can_assign) {
    bool value = prev_.get_kind() == TokenKind::KW_TRUE;
    u8_t constant = add_constant(BooleanObject::create(vm_, value));
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  void numeric(bool can_assign) {
    double value = prev_.as_numeric();
    u8_t constant = add_constant(NumericObject::create(vm_, value));
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  void string(bool can_assign) {
    auto s = prev_.as_string();
    u8_t constant = add_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  void variable(bool can_assign) {
    u8_t constant = name_constant();

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_bytes(OpCode::OP_SET_GLOBAL, constant);
    }
    else {
      emit_bytes(OpCode::OP_GET_GLOBAL, constant);
    }
  }

  void grouping(bool can_assign) {
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
  }

  void binary(bool can_assign) {
    auto oper_kind = prev_.get_kind();
    auto& rule = get_rule(oper_kind);

    // compile the right-hand operand
    parse_precedence(Xenum::as_enum<Precedence>(rule.precedence + 1));

    switch (oper_kind) {
    case TokenKind::TK_GREATER: emit_byte(OpCode::OP_GT); break;
    case TokenKind::TK_GREATEREQUAL: emit_byte(OpCode::OP_GE); break;
    case TokenKind::TK_LESS: emit_byte(OpCode::OP_LT); break;
    case TokenKind::TK_LESSEQUAL: emit_byte(OpCode::OP_LE); break;
    case TokenKind::TK_PLUS: emit_byte(OpCode::OP_ADD); break;
    case TokenKind::TK_MINUS: emit_byte(OpCode::OP_SUB); break;
    case TokenKind::TK_STAR: emit_byte(OpCode::OP_MUL); break;
    case TokenKind::TK_SLASH: emit_byte(OpCode::OP_DIV); break;
    default: assert(false); break; // unreachable
    }
  }

  void unary(bool can_assign) {
    auto oper_kind = prev_.get_kind();

    // compile the operand
    parse_precedence(Xenum::as_enum<Precedence>(Precedence::UNARY + 1));

    switch (oper_kind) {
    case TokenKind::TK_BANG: emit_byte(OpCode::OP_NOT); break;
    case TokenKind::TK_MINUS: emit_byte(OpCode::OP_NEG); break;
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

  void consume(TokenKind kind, const str_t& message) {
    if (curr_.get_kind() != kind)
      error(message);
    advance();
  }

  bool match(TokenKind kind) {
    if (curr_.get_kind() != kind)
      return false;

    advance();
    return true;
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT);
  }

  void statement(void) {
    if (match(TokenKind::KW_VAR)) {
      consume(TokenKind::TK_IDENTIFIER, "expect variable name");
      u8_t constant = name_constant();

      // check initializer
      consume(TokenKind::TK_EQUAL, "expect `=` after variable name");
      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after variable initializer");

      emit_bytes(OpCode::OP_DEF_GLOBAL, constant);
    }

    expression();
    consume(TokenKind::TK_SEMI, "expected `;` after expression");
  }
};

static Compiler* _main_compiler = nullptr;

FunctionObject* Compile::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(vm, lex);
  _main_compiler = &c;

  c.advance();
  do {
    c.statement();
  } while (!c.match(TokenKind::TK_EOF));
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
