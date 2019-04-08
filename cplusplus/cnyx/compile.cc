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

class CompileParser;

struct ParseRule {
  std::function<void (CompileParser*, bool)> prefix;
  std::function<void (CompileParser*, bool)> infix;
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

struct CompilerImpl {
  CompilerImpl* enclosing{};
  FunctionObject* function{};
  std::vector<Local> locals;
  int scope_depth{-1};

  CompilerImpl(void) {}

  inline void inc_scope_depth(void) { ++scope_depth; }
  inline void dec_scope_depth(void) { --scope_depth; }
  inline void append_local(const Local& v) { locals.push_back(v); }
  inline void append_local(Local&& v) { locals.emplace_back(v); }
};

class CompileParser : private UnCopyable {
  VM& vm_;
  Lexer& lex_;
  CompilerImpl& curr_compiler_;
  Token curr_;
  Token prev_;
  bool had_error_{};

  void error_at(int lineno, const str_t& message) {
    std::cerr << "[LINE: " << lineno << "] ERROR: " << message << std::endl;
    had_error_ = true;
  }

  void error(const str_t& message) {
    error_at(prev_.get_lineno(), message);
  }

  void emit_byte(u8_t byte) {
    curr_compiler_.function->append_code(byte, prev_.get_lineno());
  }

  void emit_bytes(u8_t byte1, u8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  void emit_loop(int loop_start) {
    emit_byte(OpCode::OP_LOOP);

    // check for overflow
    int offset = curr_compiler_.function->codes_count() - loop_start + 2;
    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
  }

  int emit_jump(u8_t instruction) {
    emit_byte(instruction);
    emit_bytes(0xff, 0xff);
    return curr_compiler_.function->codes_count() - 2;
  }

  void patch_jump(int offset) {
    int jump = curr_compiler_.function->codes_count() - offset - 2;
    curr_compiler_.function->set_code(offset, (jump >> 8) & 0xff);
    curr_compiler_.function->set_code(offset + 1, jump & 0xff);
  }

  u8_t add_constant(Value constant) {
    return static_cast<u8_t>(curr_compiler_.function->append_constant(constant));
  }

  u8_t name_constant(void) {
    auto s = prev_.as_string();
    return add_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto or_fn = [](CompileParser* p, bool b) { p->or_op(b); };
    static auto and_fn = [](CompileParser* p, bool b) { p->and_op(b); };
    static auto grouping_fn = [](CompileParser* p, bool b) { p->grouping(b); };
    static auto boolean_fn = [](CompileParser* p, bool b) { p->boolean(b); };
    static auto nil_fn = [](CompileParser* p, bool b) { p->nil(b); };
    static auto numeric_fn = [](CompileParser* p, bool b) { p->numeric(b); };
    static auto string_fn = [](CompileParser* p, bool b) { p->string(b); };
    static auto variable_fn = [](CompileParser* p, bool b) { p->variable(b); };
    static auto binary_fn = [](CompileParser* p, bool b) { p->binary(b); };
    static auto unary_fn = [](CompileParser* p, bool b) { p->unary(b); };
    static auto call_fn = [](CompileParser* p, bool b) { p->call(b); };

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
    curr_compiler_.inc_scope_depth();
  }

  void leave_scope(void) {
    // for (int i = curr_compiler_.locals.size() - 1; i >= 0; --i) {
    //   auto& local = curr_compiler_.locals[i];
    //   if (local.depth < curr_compiler_.scope_depth)
    //     break;

    //   // TODO: local.name -> set literal.size -> 0
    //   local.name.set_force_ne(true);
    // }

    // TODO: use old version ???
    curr_compiler_.dec_scope_depth();
    while (curr_compiler_.locals.size() > 0 &&
        curr_compiler_.locals.back().depth > curr_compiler_.scope_depth) {
      emit_byte(OpCode::OP_POP);
      curr_compiler_.locals.pop_back();
    }
  }

  int resolve_local(const Token& name) {
    int i = static_cast<int>(curr_compiler_.locals.size() - 1);
    for (; i >= 0; --i) {
      if (name.is_equal(curr_compiler_.locals[i].name))
        return i;
    }
    return -1;
  }

  std::tuple<Token, u8_t> parse_variable(const str_t& error) {
    consume(TokenKind::TK_IDENTIFIER, error);
    auto name = prev_;
    u8_t constant = 0;
    if (curr_compiler_.scope_depth == -1)
      constant = name_constant();
    return std::make_tuple(name, constant);
  }

  void declare_variable(const Token& name, u8_t constant) {
    if (curr_compiler_.scope_depth == -1) {
      emit_bytes(OpCode::OP_DEF_GLOBAL, constant);
    }
    else {
      int i = static_cast<int>(curr_compiler_.locals.size() - 1);
      for (; i >= 0; --i) {
        auto& local = curr_compiler_.locals[i];
        if (local.depth < curr_compiler_.scope_depth)
          break;
        if (name.is_equal(local.name)) {
          error_at(name.get_lineno(),
              "variable with this name already declared in this scope");
        }
      }

      curr_compiler_.append_local(Local(name, curr_compiler_.scope_depth));
    }
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
  CompileParser(VM& vm, Lexer& lex, CompilerImpl& c)
    : vm_(vm), lex_(lex), curr_compiler_(c) {
  }

  inline bool had_error(void) const { return had_error_; }

  void begin_compiler(CompilerImpl& new_compiler) {
    new_compiler.enclosing = &curr_compiler_;
    new_compiler.function = FunctionObject::create(vm_);
    curr_compiler_ = new_compiler;
  }

  FunctionObject* finish_compiler(void) {
    emit_bytes(OpCode::OP_NIL, OpCode::OP_RETURN);
    auto* fn = curr_compiler_.function;
    curr_compiler_ = *curr_compiler_.enclosing;

    return had_error_ ? nullptr : fn;
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
    if (match(TokenKind::KW_FUN)) {
      fun_stmt();
    }
    else if (match(TokenKind::KW_IF)) {
      if_stmt();
    }
    else if (match(TokenKind::KW_RETURN)) {
      return_stmt();
    }
    else if (match(TokenKind::KW_VAR)) {
      var_stmt();
    }
    else if (match(TokenKind::KW_WHILE)) {
      while_stmt();
    }
    else if (check(TokenKind::TK_LBRACE)) {
      enter_scope();
      block_stmt();
      leave_scope();
    }
    else {
      expression();
      emit_byte(OpCode::OP_POP);
      consume(TokenKind::TK_SEMI, "expected `;` after expression");
    }
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
    auto [name, constant] = parse_variable("expect variable name");

    // compile the initializer
    consume(TokenKind::TK_EQUAL, "expect `=` after variable name");
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after initializer");

    declare_variable(name, constant);
  }

  void while_stmt(void) {
    int loop_start = curr_compiler_.function->codes_count();

    consume(TokenKind::TK_LPAREN, "expect `(` before while condition");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

    enter_scope();

    // jump out of the loop if the condition is false
    int exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    // compile the loop body
    emit_byte(OpCode::OP_POP); // condition
    statement();

    emit_loop(loop_start); // loop back to the start
    patch_jump(exit_jump);

    leave_scope();
  }

  void fun_stmt(void) {
    auto [name, name_constant] = parse_variable("expect function name");

    CompilerImpl fn_compiler;
    begin_compiler(fn_compiler);
    enter_scope();
    consume(TokenKind::TK_LPAREN, "expect `(` after function name");
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        auto [param, param_constant] = parse_variable("expect parameter name");
        curr_compiler_.function->inc_arity();
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
    block_stmt();
    leave_scope();
    auto* fn = finish_compiler();

    u8_t fn_constant = curr_compiler_.function->append_constant(fn);
    emit_bytes(OpCode::OP_CONSTANT, fn_constant);
    declare_variable(name, name_constant);
  }

  void return_stmt(void) {
    if (match(TokenKind::TK_SEMI)) {
      emit_byte(OpCode::OP_NIL);
    }
    else {
      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after return value");
    }
    emit_byte(OpCode::OP_RETURN);
  }
};

static CompilerImpl* _main_compiler = nullptr;

FunctionObject* Compile::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  CompilerImpl c;
  CompileParser p(vm, lex, c);

  p.begin_compiler(c);
  _main_compiler = &c;

  p.advance();
  if (!p.match(TokenKind::TK_EOF)) {
    do {
      p.statement();
    } while (!p.match(TokenKind::TK_EOF));
  }

  auto* fn = p.finish_compiler();
  _main_compiler = nullptr;

  return fn;
}

void gray_compiler_roots(VM& vm) {
  auto* compiler_iter = _main_compiler;
  while (compiler_iter != nullptr) {
    vm.gray_value(compiler_iter->function);
    compiler_iter = compiler_iter->enclosing;
  }
}

}
