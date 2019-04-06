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

struct Local {
  Token name;
  int depth{};

  Local(const Token& lname, int ldepth) : name(lname), depth(ldepth) {}
  void assign(const Token& lname, int ldepth) {
    name = lname;
    depth = ldepth;
  }
};

class Compiler : private UnCopyable {
  Compiler* enclosing_{};

  VM& vm_;
  Lexer& lex_;
  Token curr_;
  Token prev_;
  bool had_error_{};

  FunctionObject* function_{};

  std::vector<Local> locals_;
  int scope_depth_{-1};

  void error(const str_t& message) {
    std::cerr << "[" << curr_.get_lineno() << "] ERROR:" << message << std::endl;
    had_error_ = true;
  }

  void emit_byte(u8_t byte) {
    function_->append_code(byte, prev_.get_lineno());
  }

  void emit_bytes(u8_t byte1, u8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  int emit_jump(u8_t instruction) {
    emit_byte(instruction);
    emit_bytes(0xff, 0xff);
    return function_->codes_count() - 2;
  }

  void patch_jump(int offset) {
    int jump = function_->codes_count() - offset - 2;
    function_->set_code(offset, (jump >> 8) & 0xff);
    function_->set_code(offset + 1, jump & 0xff);
  }

  u8_t add_constant(Value constant) {
    return static_cast<u8_t>(function_->append_constant(constant));
  }

  u8_t name_constant(void) {
    auto s = prev_.as_string();
    return add_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto or_fn = [](Compiler* p, bool b) { p->or_op(b); };
    static auto and_fn = [](Compiler* p, bool b) { p->and_op(b); };
    static auto grouping_fn = [](Compiler* p, bool b) { p->grouping(b); };
    static auto boolean_fn = [](Compiler* p, bool b) { p->boolean(b); };
    static auto nil_fn = [](Compiler* p, bool b) { p->nil(b); };
    static auto numeric_fn = [](Compiler* p, bool b) { p->numeric(b); };
    static auto string_fn = [](Compiler* p, bool b) { p->string(b); };
    static auto variable_fn = [](Compiler* p, bool b) { p->variable(b); };
    static auto binary_fn = [](Compiler* p, bool b) { p->binary(b); };
    static auto unary_fn = [](Compiler* p, bool b) { p->unary(b); };
    static auto call_fn = [](Compiler* p, bool b) { p->call(b); };

    static ParseRule _rules[] = {
      nullptr, nullptr, Precedence::NONE, // TK_ERROR
      nullptr, nullptr, Precedence::NONE, // TK_EOF
      variable_fn, nullptr, Precedence::NONE, // TK_IDENTIFIER
      string_fn, nullptr, Precedence::NONE, // TK_STRINGLITERAL
      numeric_fn, nullptr, Precedence::NONE, // TK_NUMERICCONST

      grouping_fn, call_fn, Precedence::CALL, // TK_LPAREN
      nullptr, nullptr, Precedence::NONE, // TK_RPAREN
      nullptr, nullptr, Precedence::NONE, // TK_LBRACE
      nullptr, nullptr, Precedence::NONE, // TK_RBRACE
      nullptr, nullptr, Precedence::NONE, // TK_COMMA
      nullptr, nullptr, Precedence::NONE, // TK_DOT
      nullptr, nullptr, Precedence::NONE, // TK_SEMI
      unary_fn, nullptr, Precedence::NONE, // TK_BANG
      nullptr, binary_fn, Precedence::EQUALITY, // TK_BANGEQUAL
      nullptr, nullptr, Precedence::NONE, // TK_EQUAL
      nullptr, binary_fn, Precedence::EQUALITY, // TK_EQUALEQUAL
      nullptr, binary_fn, Precedence::COMPARISON, // TK_GREATER
      nullptr, binary_fn, Precedence::COMPARISON, // TK_GREATEREQUAL
      nullptr, binary_fn, Precedence::COMPARISON, // TK_LESS
      nullptr, binary_fn, Precedence::COMPARISON, // TK_LESSEQUAL
      nullptr, binary_fn, Precedence::TERM, // TK_PLUS
      unary_fn, binary_fn, Precedence::TERM, // TK_MINUS
      nullptr, binary_fn, Precedence::FACTOR, // TK_STAR
      nullptr, binary_fn, Precedence::FACTOR, // TK_SLASH

      nullptr, and_fn, Precedence::AND, // KW_AND
      nullptr, nullptr, Precedence::NONE, // KW_CLASS
      nullptr, nullptr, Precedence::NONE, // KW_ELSE
      boolean_fn, nullptr, Precedence::NONE, // KW_FALSE
      nullptr, nullptr, Precedence::NONE, // KW_FOR
      nullptr, nullptr, Precedence::NONE, // KW_FUN
      nullptr, nullptr, Precedence::NONE, // KW_IF
      nil_fn, nullptr, Precedence::NONE, // KW_NIL
      nullptr, or_fn, Precedence::OR, // KW_OR
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

  void enter_scope(void) {
    ++scope_depth_;
  }

  void leave_scope(void) {
    --scope_depth_;

    while (locals_.size() > 0 &&
        locals_.back().depth > scope_depth_) {
      emit_byte(OpCode::OP_POP);
      locals_.pop_back();
    }
  }

  int resolve_local(const Token& name) {
    int i = static_cast<int>(locals_.size() - 1);
    for (; i >= 0; --i) {
      if (name.is_equal(locals_[i].name))
        return i;
    }
    return -1;
  }

  void boolean(bool can_assign) {
    bool value = prev_.get_kind() == TokenKind::KW_TRUE;
    u8_t constant = add_constant(BooleanObject::create(vm_, value));
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  void nil(bool can_assign) {
    emit_byte(OpCode::OP_NIL);
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
    OpCode setop{OpCode::OP_SET_LOCAL}, getop{OpCode::OP_GET_LOCAL};
    int local = resolve_local(prev_);
    u8_t constant = static_cast<u8_t>(local);
    if (local == -1) {
      constant = name_constant();
      setop = OpCode::OP_SET_GLOBAL;
      getop = OpCode::OP_GET_GLOBAL;
    }

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_bytes(setop, constant);
    }
    else {
      emit_bytes(getop, constant);
    }
  }

  void or_op(bool can_assign) {
    int else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OpCode::OP_JUMP);
    patch_jump(else_jump);
    emit_byte(OpCode::OP_POP);

    parse_precedence(Precedence::OR);
    patch_jump(end_jump);
  }

  void and_op(bool can_assign) {
    int end_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    emit_byte(OpCode::OP_POP);

    parse_precedence(Precedence::AND);
    patch_jump(end_jump);
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
    case TokenKind::TK_BANGEQUAL: emit_byte(OpCode::OP_NE); break;
    case TokenKind::TK_EQUALEQUAL: emit_byte(OpCode::OP_EQ); break;
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

  void call(bool can_assign) {
    u8_t argc{};
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        expression();
        ++argc;
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    emit_byte(OpCode::OP_CALL_0 + argc);
  }
public:
  Compiler(VM& vm, Lexer& lex) : vm_(vm), lex_(lex) {
    function_ = FunctionObject::create(vm_);
  }

  inline bool had_error(void) const { return had_error_; }
  inline FunctionObject* get_function(void) const { return function_; }
  inline Compiler* get_enclosing(void) const { return enclosing_; }
  inline void gray_function(void) { vm_.gray_value(function_); }

  inline void begin_compiler(void) { advance(); }
  inline void finish_compiler(void) { emit_byte(OpCode::OP_RETURN); }

  void advance(void) {
    prev_ = curr_;
    curr_ = lex_.next_token();
  }

  void consume(TokenKind kind, const str_t& message) {
    if (curr_.get_kind() != kind)
      error(message);
    advance();
  }

  bool check(TokenKind kind) const {
    return curr_.get_kind() == kind;
  }

  bool match(TokenKind kind) {
    if (!check(kind))
      return false;

    advance();
    return true;
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT);
  }

  void statement(void) {
    if (match(TokenKind::KW_IF)) {
      if_stmt();
      return;
    }
    if (match(TokenKind::KW_VAR)) {
      var_stmt();
      return;
    }
    if (check(TokenKind::TK_LBRACE)) {
      enter_scope();
      block_stmt();
      leave_scope();
      return;
    }

    expression();
    emit_byte(OpCode::OP_POP);
    consume(TokenKind::TK_SEMI, "expected `;` after expression");
  }

  void block_stmt(void) {
    consume(TokenKind::TK_LBRACE, "expect `{` before block");
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
      statement();
    consume(TokenKind::TK_RBRACE, "expect `}` after block");
  }

  void if_stmt(void) {
    consume(TokenKind::TK_LPAREN, "expect `(` before if condition");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

    enter_scope();

    // jump to the else branch if the condition is false
    int else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    // compile the then branch
    emit_byte(OpCode::OP_POP); // condition
    statement();

    // jump over the else branch when the if branch is taken
    int end_jump = emit_jump(OpCode::OP_JUMP);
    // compile the else branch
    patch_jump(else_jump);
    emit_byte(OpCode::OP_POP); // condition

    if (match(TokenKind::KW_ELSE))
      statement();
    patch_jump(end_jump);

    leave_scope();
  }

  void var_stmt(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect variable name");
    auto name = prev_;
    u8_t constant = name_constant();

    // compile the initializer
    consume(TokenKind::TK_EQUAL, "expect `=` after variable name");
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after initializer");

    if (scope_depth_ == -1)
      emit_bytes(OpCode::OP_DEF_GLOBAL, constant);
    else
      locals_.push_back(Local(name, scope_depth_));
  }
};

static Compiler* _main_compiler = nullptr;

FunctionObject* Compile::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(vm, lex);
  _main_compiler = &c;

  c.begin_compiler();
  if (!c.match(TokenKind::TK_EOF)) {
    do {
      c.statement();
    } while (!c.match(TokenKind::TK_EOF));
  }
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
