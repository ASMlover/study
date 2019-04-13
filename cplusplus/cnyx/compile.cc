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
#include <cstdarg>
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
  bool upvalue{};

  Local(const Token& lname, int ldepth, bool lupvalue = false)
    : name(lname), depth(ldepth), upvalue(lupvalue) {}
  void assign(const Token& lname, int ldepth, bool lupvalue = false) {
    name = lname;
    depth = ldepth;
    upvalue = lupvalue;
  }
};

struct Upvalue {
  u8_t index{};
  bool local{};

  Upvalue(u8_t idx, bool l) : index(idx), local(l) {}
  void assign(u8_t idx, bool l) {
    index = idx;
    local = l;
  }
};

struct CompilerImpl {
  CompilerImpl* enclosing{};
  FunctionObject* function{};
  std::vector<Local> locals;
  std::vector<Upvalue> upvalues;
  int scope_depth{}; // 0: is global scope

  CompilerImpl(void) {}

  inline void inc_scope_depth(void) { ++scope_depth; }
  inline void dec_scope_depth(void) { --scope_depth; }
  inline void append_local(const Local& v) { locals.push_back(v); }
  inline void append_local(Local&& v) { locals.emplace_back(v); }
  inline void append_upvalue(const Upvalue& v) { upvalues.push_back(v); }
  inline void append_upvalue(Upvalue&& v) { upvalues.emplace_back(v); }
};

class CompileParser : private UnCopyable {
  VM& vm_;
  Lexer& lex_;
  CompilerImpl* curr_compiler_{};
  Token curr_;
  Token prev_;
  bool had_error_{};

  static constexpr int kMaxArguments = 8;

  void error(const char* format, ...) {
    std::cerr
      << "[LINE: " << prev_.get_lineno() << "] ERROR at "
      << "`" << prev_.get_literal() << "`: ";

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    std::cerr << std::endl;

    had_error_ = true;
  }

  void emit_byte(u8_t byte) {
    curr_compiler_->function->append_code(byte, prev_.get_lineno());
  }

  void emit_bytes(u8_t byte1, u8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  void emit_loop(int loop_start) {
    emit_byte(OpCode::OP_LOOP);

    // check for overflow
    int offset = curr_compiler_->function->codes_count() - loop_start + 2;
    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
  }

  int emit_jump(u8_t instruction) {
    emit_byte(instruction);
    emit_bytes(0xff, 0xff);
    return curr_compiler_->function->codes_count() - 2;
  }

  void patch_jump(int offset) {
    int jump = curr_compiler_->function->codes_count() - offset - 2;
    curr_compiler_->function->set_code(offset, (jump >> 8) & 0xff);
    curr_compiler_->function->set_code(offset + 1, jump & 0xff);
  }

  u8_t add_constant(Value constant) {
    vm_.invoke_push(constant);
    auto i = curr_compiler_->function->append_constant(vm_.invoke_pop());
    return static_cast<u8_t>(i);
  }

  u8_t identifier_constant(void) {
    auto s = prev_.as_string();
    return add_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  void emit_constant(Value value) {
    u8_t constant = add_constant(value);
    emit_bytes(OpCode::OP_CONSTANT, constant);
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto or_fn = [](CompileParser* p, bool b) { p->or_exp(b); };
    static auto and_fn = [](CompileParser* p, bool b) { p->and_exp(b); };
    static auto grouping_fn = [](CompileParser* p, bool b) { p->grouping(b); };
    static auto boolean_fn = [](CompileParser* p, bool b) { p->boolean(b); };
    static auto nil_fn = [](CompileParser* p, bool b) { p->nil(b); };
    static auto numeric_fn = [](CompileParser* p, bool b) { p->numeric(b); };
    static auto string_fn = [](CompileParser* p, bool b) { p->string(b); };
    static auto variable_fn = [](CompileParser* p, bool b) { p->variable(b); };
    static auto binary_fn = [](CompileParser* p, bool b) { p->binary(b); };
    static auto unary_fn = [](CompileParser* p, bool b) { p->unary(b); };
    static auto call_fn = [](CompileParser* p, bool b) { p->call(b); };
    static auto dot_fn = [](CompileParser* p, bool b) { p->dot(b); };
    static auto this_fn = [](CompileParser* p, bool b) { p->this_exp(b); };

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
      nullptr, dot_fn, Precedence::CALL, // TK_DOT
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
      // nullptr, nullptr, Precedence::NONE, // KW_PRINT
      nullptr, nullptr, Precedence::NONE, // KW_RETURN
      nullptr, nullptr, Precedence::NONE, // KW_SUPER
      this_fn, nullptr, Precedence::NONE, // KW_THIS
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
    curr_compiler_->inc_scope_depth();
  }

  void leave_scope(void) {
    curr_compiler_->dec_scope_depth();
    while (curr_compiler_->locals.size() > 0 &&
        curr_compiler_->locals.back().depth > curr_compiler_->scope_depth) {
      if (curr_compiler_->locals.back().upvalue)
        emit_byte(OpCode::OP_CLOSE_UPVALUE);
      else
        emit_byte(OpCode::OP_POP);
      curr_compiler_->locals.pop_back();
    }
  }

  int resolve_local(CompilerImpl* compiler, const Token& name, bool in_func) {
    int i = static_cast<int>(compiler->locals.size() - 1);
    for (; i >= 0; --i) {
      auto& local = compiler->locals[i];
      if (name.is_equal(local.name)) {
        if (!in_func && local.depth == -1)
          error("a local variable cannot be use in its own initializer");
        return i;
      }
    }
    return -1;
  }

  int add_upvalue(CompilerImpl* compiler, u8_t index, bool is_local) {
    for (int i = 0; i < compiler->function->upvalues_count(); ++i) {
      auto& upvalue = compiler->upvalues[i];
      if (upvalue.index == index && upvalue.local == is_local)
        return i;
    }
    compiler->append_upvalue(Upvalue(index, is_local));
    return compiler->function->inc_upvalues_count();
  }

  int resolve_upvalue(CompilerImpl* compiler, const Token& name) {
    if (compiler->enclosing == nullptr)
      return -1;

    int local = resolve_local(compiler->enclosing, name, true);
    if (local != -1) {
      compiler->enclosing->locals[local].upvalue = true;
      return add_upvalue(compiler, static_cast<u8_t>(local), true);
    }

    int upvalue = resolve_upvalue(compiler->enclosing, name);
    if (upvalue != -1)
      return add_upvalue(compiler, static_cast<u8_t>(upvalue), false);
    return -1;
  }

  void declare_variable(void) {
    if (curr_compiler_->scope_depth == 0)
      return;

    auto name = prev_;
    int begindex = static_cast<int>(curr_compiler_->locals.size() - 1);
    for (int i = begindex; i >= 0; --i) {
      auto& local = curr_compiler_->locals[i];
      if (local.depth != -1 && local.depth < curr_compiler_->scope_depth)
        break;
      if (name.is_equal(local.name))
        error("variable with this name already declared in this scope");
    }

    curr_compiler_->append_local(Local(name, -1, false));
  }

  u8_t parse_variable(const str_t& error_message) {
    consume(TokenKind::TK_IDENTIFIER, error_message);

    if (curr_compiler_->scope_depth == 0)
      return identifier_constant();

    declare_variable();
    return 0;
  }

  void define_variable(u8_t global) {
    if (curr_compiler_->scope_depth == 0)
      emit_bytes(OpCode::OP_DEF_GLOBAL, global);
    else
      curr_compiler_->locals.back().depth = curr_compiler_->scope_depth;
  }

  u8_t argument_list(void) {
    u8_t argc = 0;
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        expression();
        ++argc;

        if (argc > kMaxArguments)
          error("cannot have more than %d arguments", kMaxArguments);
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    return argc;
  }

  void boolean(bool can_assign) {
    bool value = prev_.get_kind() == TokenKind::KW_TRUE;
    emit_constant(BooleanObject::create(vm_, value));
  }

  void nil(bool can_assign) {
    emit_byte(OpCode::OP_NIL);
  }

  void numeric(bool can_assign) {
    double value = prev_.as_numeric();
    emit_constant(NumericObject::create(vm_, value));
  }

  void string(bool can_assign) {
    auto s = prev_.as_string();
    emit_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  void variable(bool can_assign) {
    OpCode setop, getop;
    int arg = resolve_local(curr_compiler_, prev_, false);
    if (arg != -1) {
      setop = OpCode::OP_SET_LOCAL;
      getop = OpCode::OP_GET_LOCAL;
    }
    else if ((arg = resolve_upvalue(curr_compiler_, prev_)) != -1) {
      setop = OpCode::OP_SET_UPVALUE;
      getop = OpCode::OP_GET_UPVALUE;
    }
    else {
      arg = identifier_constant();
      setop = OpCode::OP_SET_GLOBAL;
      getop = OpCode::OP_GET_GLOBAL;
    }
    u8_t constant = static_cast<u8_t>(arg);

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_bytes(setop, constant);
    }
    else {
      emit_bytes(getop, constant);
    }
  }

  void or_exp(bool can_assign) {
    int else_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OpCode::OP_JUMP);
    patch_jump(else_jump);
    emit_byte(OpCode::OP_POP);

    parse_precedence(Precedence::OR);
    patch_jump(end_jump);
  }

  void and_exp(bool can_assign) {
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
    u8_t argc = argument_list();
    emit_byte(OpCode::OP_CALL_0 + argc);
  }

  void dot(bool can_assign) {
    consume(TokenKind::TK_IDENTIFIER, "expect property name");
    u8_t name = identifier_constant();

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_bytes(OpCode::OP_SET_FIELD, name);
    }
    else if (match(TokenKind::TK_LPAREN)) {
      u8_t argc = argument_list();
      emit_bytes(OpCode::OP_INVOKE_0 + argc, name);
    }
    else {
      emit_bytes(OpCode::OP_GET_FIELD, name);
    }
  }

  void this_exp(bool can_assign) {
    int arg = resolve_local(curr_compiler_, prev_, false);
    if (arg != -1)
      emit_bytes(OpCode::OP_GET_LOCAL, static_cast<u8_t>(arg));
    else if ((arg = resolve_upvalue(curr_compiler_, prev_)) != -1)
      emit_bytes(OpCode::OP_GET_UPVALUE, static_cast<u8_t>(arg));
    else
      error("cannot use `this` outside of a class");
  }
public:
  CompileParser(VM& vm, Lexer& lex)
    : vm_(vm), lex_(lex) {
  }

  inline bool had_error(void) const { return had_error_; }

  void begin_compiler(
      CompilerImpl& new_compiler, int scope_depth, bool is_method) {
    new_compiler.enclosing = curr_compiler_;
    new_compiler.function = FunctionObject::create(vm_);
    new_compiler.scope_depth = scope_depth;
    curr_compiler_ = &new_compiler;

    if (is_method) {
      curr_compiler_->append_local(
          Local{Token{TokenKind::TK_STRINGLITERAL, "this", 0},
          curr_compiler_->scope_depth});
    }
    else {
      curr_compiler_->append_local(
          Local{Token{TokenKind::TK_STRINGLITERAL, "", 0},
          curr_compiler_->scope_depth});
    }
  }

  FunctionObject* finish_compiler(void) {
    emit_bytes(OpCode::OP_NIL, OpCode::OP_RETURN);
    auto* fn = curr_compiler_->function;
    curr_compiler_ = curr_compiler_->enclosing;

#if defined(DEBUG_PRINT_CODE)
    if (!had_error_)
      fn->dump();
#endif

    return fn;
  }

  void advance(void) {
    prev_ = curr_;
    curr_ = lex_.next_token();
  }

  void consume(TokenKind kind, const str_t& message) {
    if (curr_.get_kind() != kind)
      error(message.c_str());
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
    if (match(TokenKind::KW_CLASS)) {
      class_stmt();
    }
    else if (match(TokenKind::KW_FUN)) {
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
      block_stmt();
    }
    else {
      expression();
      emit_byte(OpCode::OP_POP);
      consume(TokenKind::TK_SEMI, "expected `;` after expression");
    }
  }

  void block_stmt(void) {
    consume(TokenKind::TK_LBRACE, "expect `{` before block");

    enter_scope();
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
      statement();
    leave_scope();

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
    u8_t global = parse_variable("expect variable name");

    // compile the initializer
    consume(TokenKind::TK_EQUAL, "expect `=` after variable name");
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after initializer");

    define_variable(global);
  }

  void while_stmt(void) {
    int loop_start = curr_compiler_->function->codes_count();

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

  void fn_common(bool is_method = false) {
    CompilerImpl fn_compiler;
    begin_compiler(fn_compiler, 1, is_method);
    enter_scope();
    consume(TokenKind::TK_LPAREN, "expect `(` after function name");
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        u8_t param_constant = parse_variable("expect parameter name");
        define_variable(param_constant);
        curr_compiler_->function->inc_arity();

        if (curr_compiler_->function->arity() > kMaxArguments)
          error("cannot have more than %d parameters", kMaxArguments);
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
    block_stmt();
    leave_scope();
    auto* fn = finish_compiler();

    u8_t constant = add_constant(fn);
    emit_bytes(OpCode::OP_CLOSURE, constant);

    for (int i = 0; i < fn->upvalues_count(); ++i) {
      auto upval = fn_compiler.upvalues[i];
      emit_bytes(upval.local ? 1 : 0, upval.index);
    }
  }

  void fun_stmt(void) {
    u8_t name_constant = parse_variable("expect function name");
    fn_common(false);
    define_variable(name_constant);
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

  void method(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect method name");
    u8_t method_constant = identifier_constant();
    fn_common(true);
    emit_bytes(OpCode::OP_METHOD, method_constant);
  }

  void class_stmt(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect class anme");
    u8_t name_constant = identifier_constant();
    declare_variable();
    emit_bytes(OpCode::OP_CLASS, name_constant);

    consume(TokenKind::TK_LBRACE, "expect `{` before class body");
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
      method();
    consume(TokenKind::TK_RBRACE, "expect `}` after class body");

    define_variable(name_constant);
  }
};

static CompilerImpl* _main_compiler = nullptr;

FunctionObject* Compile::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  CompileParser p(vm, lex);

  CompilerImpl c;
  p.begin_compiler(c, 0, false);
  _main_compiler = &c;

  p.advance();
  if (!p.match(TokenKind::TK_EOF)) {
    do {
      p.statement();
    } while (!p.match(TokenKind::TK_EOF));
  }

  auto* fn = p.finish_compiler();
  _main_compiler = nullptr;

  return p.had_error() ? nullptr : fn;
}

void gray_compiler_roots(VM& vm) {
  auto* compiler_iter = _main_compiler;
  while (compiler_iter != nullptr) {
    vm.gray_value(compiler_iter->function);
    compiler_iter = compiler_iter->enclosing;
  }
}

}
