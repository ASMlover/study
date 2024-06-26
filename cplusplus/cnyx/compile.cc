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

inline Precedence operator+(Precedence a, int b) {
  return Xenum::as_enum<Precedence>(Xenum::as_int<int>(a) + b);
}

class CompileParser;

static CompileParser* _curr_cp{};

struct ParseRule {
  std::function<void (CompileParser*, bool)> prefix;
  std::function<void (CompileParser*, bool)> infix;
  Precedence precedence;
};

struct Local {
  Token name;
  int depth{-1};
  bool upvalue{};

  Local(const Token& lname, int ldepth = -1, bool lupvalue = false)
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

enum class FunctionKind {
  FUNCTION, // function
  CTOR, // constructor of class
  METHOD, // method of class

  TOP_LEVEL,
};

struct CompilerImpl {
  CompilerImpl* enclosing{};
  FunctionObject* function{};
  FunctionKind fun_kind{FunctionKind::FUNCTION};
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

struct ClassCompilerImpl {
  ClassCompilerImpl* enclosing{};
  Token name{};
  bool has_superclass{};

  ClassCompilerImpl(
      ClassCompilerImpl* lenclosing, const Token& lname, bool has_super = false)
    : enclosing(lenclosing), name(lname), has_superclass(has_super) {
  }
};

class CompileParser : private UnCopyable {
  VM& vm_;
  Lexer& lex_;
  CompilerImpl* curr_compiler_{};
  ClassCompilerImpl* curr_class_{};
  Token curr_;
  Token prev_;
  bool had_error_{};
  bool panic_mode_{};

  static constexpr int kMaxArguments = 8;

  void error_at(const Token& tok, const str_t& message) {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "[LINE: " << tok.get_lineno() << "] ERROR at ";
    if (tok.get_kind() == TokenKind::TK_EOF)
      std::cerr << "end";
    else
      std::cerr << "`" << tok.get_literal() << "`";
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
  }

  void error(const str_t& message) {
    error_at(prev_, message);
  }

  void error_at_current(const str_t& message) {
    error_at(curr_, message);
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

  void emit_return(void) {
    if (curr_compiler_->fun_kind == FunctionKind::CTOR)
      emit_bytes(OpCode::OP_GET_LOCAL, 0);
    else
      emit_byte(OpCode::OP_NIL);
    emit_byte(OpCode::OP_RETURN);
  }

  u8_t make_constant(const Value& value) {
    int constant = curr_compiler_->function->append_constant(value);
    if (constant > UINT8_MAX) {
      error("too many constants in one function");
      return 0;
    }
    return static_cast<u8_t>(constant);
  }

  u8_t identifier_constant(const Token& name) {
    auto s = name.as_string();
    return make_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  void emit_constant(const Value& value) {
    emit_bytes(OpCode::OP_CONSTANT, make_constant(value));
  }

  ParseRule& get_rule(TokenKind kind) {
    static auto or_fn = [](CompileParser* p, bool b) { p->or_exp(b); };
    static auto and_fn = [](CompileParser* p, bool b) { p->and_exp(b); };
    static auto grouping_fn = [](CompileParser* p, bool b) { p->grouping(b); };
    static auto true_fn = [](CompileParser* p, bool b) { p->true_exp(b); };
    static auto false_fn = [](CompileParser* p, bool b) { p->false_exp(b); };
    static auto nil_fn = [](CompileParser* p, bool b) { p->nil(b); };
    static auto numeric_fn = [](CompileParser* p, bool b) { p->numeric(b); };
    static auto string_fn = [](CompileParser* p, bool b) { p->string(b); };
    static auto variable_fn = [](CompileParser* p, bool b) { p->variable(b); };
    static auto binary_fn = [](CompileParser* p, bool b) { p->binary(b); };
    static auto unary_fn = [](CompileParser* p, bool b) { p->unary(b); };
    static auto call_fn = [](CompileParser* p, bool b) { p->call(b); };
    static auto dot_fn = [](CompileParser* p, bool b) { p->dot(b); };
    static auto super_fn = [](CompileParser* p, bool b) { p->super_exp(b); };
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
      false_fn, nullptr, Precedence::NONE, // KW_FALSE
      nullptr, nullptr, Precedence::NONE, // KW_FOR
      nullptr, nullptr, Precedence::NONE, // KW_FUN
      nullptr, nullptr, Precedence::NONE, // KW_IF
      nil_fn, nullptr, Precedence::NONE, // KW_NIL
      nullptr, or_fn, Precedence::OR, // KW_OR
      // nullptr, nullptr, Precedence::NONE, // KW_PRINT
      nullptr, nullptr, Precedence::NONE, // KW_RETURN
      super_fn, nullptr, Precedence::NONE, // KW_SUPER
      this_fn, nullptr, Precedence::NONE, // KW_THIS
      true_fn, nullptr, Precedence::NONE, // KW_TRUE
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

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      error("invalid assignment target");
      expression();
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
          error("cannot read local variable before initializing it");
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

    curr_compiler_->append_local(Local{name, -1, false});
  }

  u8_t parse_variable(const str_t& error_message) {
    consume(TokenKind::TK_IDENTIFIER, error_message);

    declare_variable();
    if (curr_compiler_->scope_depth > 0)
      return 0;

    return identifier_constant(prev_);
  }

  void make_initialized(void) {
    if (curr_compiler_->scope_depth == 0)
      return;
    curr_compiler_->locals.back().depth = curr_compiler_->scope_depth;
  }

  void define_variable(u8_t global) {
    if (curr_compiler_->scope_depth > 0) {
      make_initialized();
      return;
    }

    emit_bytes(OpCode::OP_DEF_GLOBAL, global);
  }

  void name_variable(const Token& name, bool can_assign) {
    u8_t getop, setop;
    int arg = resolve_local(curr_compiler_, name, false);
    if (arg != -1) {
      getop = OpCode::OP_GET_LOCAL;
      setop = OpCode::OP_SET_LOCAL;
    }
    else if ((arg = resolve_upvalue(curr_compiler_, name)) != -1) {
      getop = OpCode::OP_GET_UPVALUE;
      setop = OpCode::OP_SET_UPVALUE;
    }
    else {
      arg = identifier_constant(name);
      getop = OpCode::OP_GET_GLOBAL;
      setop = OpCode::OP_SET_GLOBAL;
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

  u8_t argument_list(void) {
    u8_t argc = 0;
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        expression();
        ++argc;

        if (argc > kMaxArguments) {
          error("cannot have more than " +
              std::to_string(kMaxArguments) + " arguments");
        }
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    return argc;
  }

  void push_superclass(void) {
    if (curr_class_ == nullptr)
      return;
    name_variable(Token::custom_token("super"), false);
  }

  void true_exp(bool can_assign) {
    emit_byte(OpCode::OP_TRUE);
  }

  void false_exp(bool can_assign) {
    emit_byte(OpCode::OP_FALSE);
  }

  void nil(bool can_assign) {
    emit_byte(OpCode::OP_NIL);
  }

  void numeric(bool can_assign) {
    emit_constant(prev_.as_numeric());
  }

  void string(bool can_assign) {
    auto s = prev_.as_string();
    emit_constant(
        StringObject::create(vm_, s.c_str(), static_cast<int>(s.size())));
  }

  void variable(bool can_assign) {
    name_variable(prev_, can_assign);
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
    // parse_precedence(Precedence::CALL);
    parse_precedence(Precedence::UNARY);

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
    consume(TokenKind::TK_IDENTIFIER, "expect property name after `.`");
    u8_t name = identifier_constant(prev_);

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

  void super_exp(bool can_assign) {
    if (curr_class_ == nullptr)
      error("cannot use `super` outside of a class");
    else if (!curr_class_->has_superclass)
      error("cannot use `super` in a class without superclass");

    consume(TokenKind::TK_DOT, "expect `.` after `super`");
    consume(TokenKind::TK_IDENTIFIER, "expect superclass method name");
    u8_t name = identifier_constant(prev_);

    // push the receiver
    name_variable(Token::custom_token("this"), false);

    if (match(TokenKind::TK_LPAREN)) {
      u8_t argc = argument_list();

      push_superclass();
      emit_bytes(OpCode::OP_SUPER_0 + argc, name);
    }
    else {
      push_superclass();
      emit_bytes(OpCode::OP_GET_SUPER, name);
    }
  }

  void this_exp(bool can_assign) {
    if (curr_class_ == nullptr)
      error("cannot use `this` outside of a class");
    else
      variable(false);
  }
public:
  CompileParser(VM& vm, Lexer& lex)
    : vm_(vm), lex_(lex) {
    _curr_cp = this;
  }
  ~CompileParser(void) { _curr_cp = nullptr; }

  inline CompilerImpl* curr_compiler(void) const { return curr_compiler_; }
  inline bool had_error(void) const { return had_error_; }

  void begin_compiler(
      CompilerImpl& new_compiler, int scope_depth, FunctionKind fun_kind) {
    // create function name StringObject first ...
    StringObject* func_name{};
    switch (fun_kind) {
    case FunctionKind::FUNCTION:
      {
        auto& name = prev_.get_literal();
        func_name = StringObject::create(vm_, name);
      } break;
    case FunctionKind::CTOR:
    case FunctionKind::METHOD:
      {
        str_t name(curr_class_->name.get_literal());
        name += "." + prev_.get_literal();
        func_name = StringObject::create(vm_, name);
      } break;
    case FunctionKind::TOP_LEVEL:
      break;
    }

    new_compiler.enclosing = curr_compiler_;
    new_compiler.function = FunctionObject::create(vm_);
    new_compiler.fun_kind = fun_kind;
    new_compiler.scope_depth = scope_depth;
    curr_compiler_ = &new_compiler;
    curr_compiler_->function->set_name(func_name);

    if (fun_kind != FunctionKind::FUNCTION) {
      curr_compiler_->append_local(
          Local{Token::custom_token("this"), curr_compiler_->scope_depth});
    }
    else {
      curr_compiler_->append_local(Local{
          Token::custom_token(), curr_compiler_->scope_depth});
    }
  }

  FunctionObject* finish_compiler(void) {
    emit_return();

    auto* fn = curr_compiler_->function;
#if defined(DEBUG_PRINT_CODE)
    if (!had_error_)
      fn->disassemble(fn->name() != nullptr ? fn->name()->chars() : "<top>");
#endif
    curr_compiler_ = curr_compiler_->enclosing;

    return fn;
  }

  void advance(void) {
    prev_ = curr_;

    for (;;) {
      curr_ = lex_.next_token();
      if (curr_.get_kind() != TokenKind::TK_ERROR)
        break;

      error_at_current(curr_.get_literal());
    }
  }

  void consume(TokenKind kind, const str_t& message) {
    if (curr_.get_kind() == kind) {
      advance();
      return;
    }

    error_at_current(message);
  }

  void synchronize(void) {
    panic_mode_ = false;

    while (curr_.get_kind() != TokenKind::TK_EOF) {
      if (prev_.get_kind() == TokenKind::TK_SEMI)
        return;

      switch (curr_.get_kind()) {
      case TokenKind::KW_CLASS:
      case TokenKind::KW_FUN:
      case TokenKind::KW_VAR:
      case TokenKind::KW_FOR:
      case TokenKind::KW_IF:
      case TokenKind::KW_WHILE:
      case TokenKind::KW_RETURN:
        return;
      default: break; // do nothing
      }
      advance();
    }
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

  void declaration(void) {
    if (match(TokenKind::KW_CLASS)) {
      class_decl();
    }
    else if (match(TokenKind::KW_FUN)) {
      fun_decl();
    }
    else if (match(TokenKind::KW_VAR)) {
      var_decl();
    }
    else {
      statement();
    }

    if (panic_mode_)
      synchronize();
  }

  void statement(void) {
    if (match(TokenKind::KW_FOR)) {
      for_stmt();
    }
    else if (match(TokenKind::KW_IF)) {
      if_stmt();
    }
    else if (match(TokenKind::KW_RETURN)) {
      return_stmt();
    }
    else if (match(TokenKind::KW_WHILE)) {
      while_stmt();
    }
    else if (match(TokenKind::TK_LBRACE)) {
      enter_scope();
      block_stmt();
      leave_scope();
    }
    else {
      expr_stmt();
    }
  }

  void class_decl(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect class anme");
    u8_t name_constant = identifier_constant(prev_);
    Token class_name = prev_;
    declare_variable();

    emit_bytes(OpCode::OP_CLASS, name_constant);
    define_variable(name_constant);

    ClassCompilerImpl class_compiler(curr_class_, prev_, false);
    curr_class_ = &class_compiler;

    if (match(TokenKind::TK_LESS)) {
      consume(TokenKind::TK_IDENTIFIER, "expect superclass name");
      if (class_name.is_equal(prev_))
        error("a class cannot inherit from itself");
      class_compiler.has_superclass = true;

      enter_scope();

      // store the superclass in a local variable named `super`
      variable(false);
      curr_compiler_->append_local(
          Local{Token::custom_token("super"), -1, false});
      define_variable(0);

      name_variable(class_name, false);
      emit_byte(OpCode::OP_SUBCLASS);
    }

    consume(TokenKind::TK_LBRACE, "expect `{` before class body");
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE)) {
      name_variable(class_name, false);
      method();
    }
    consume(TokenKind::TK_RBRACE, "expect `}` after class body");

    if (class_compiler.has_superclass)
      leave_scope();

    define_variable(name_constant);

    curr_class_ = curr_class_->enclosing;
  }

  void fun_decl(void) {
    u8_t name_constant = parse_variable("expect function name");
    make_initialized();
    fn_common(FunctionKind::FUNCTION);
    define_variable(name_constant);
  }

  void var_decl(void) {
    u8_t global = parse_variable("expect variable name");

    if (match(TokenKind::TK_EQUAL)) {
      // compile the initializer
      expression();
    }
    else {
      // default initialize with nil
      emit_byte(OpCode::OP_NIL);
    }
    consume(TokenKind::TK_SEMI, "expect `;` after variable declaration");

    define_variable(global);
  }

  void expr_stmt(void) {
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after expression");
    emit_byte(OpCode::OP_POP);
  }

  void for_stmt(void) {
    enter_scope();

    consume(TokenKind::TK_LPAREN, "expect `(` after keyword `for`");
    // the initialization clause
    if (match(TokenKind::KW_VAR)) {
      var_decl();
    }
    else if (match(TokenKind::TK_SEMI)) {
      // no initializer
    }
    else {
      expr_stmt();
    }
    int loop_start = curr_compiler_->function->codes_count();
    // the exit condition
    int exit_jump = -1;
    if (!match(TokenKind::TK_SEMI)) {
      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after loop condition");

      // jump out of the loop if the condition is false
      exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
      emit_byte(OpCode::OP_POP); // condition
    }

    // increment loop step
    if (!match(TokenKind::TK_RPAREN)) {
      // don't want to execute the increment before the body, so jump over it
      int body_jump = emit_jump(OpCode::OP_JUMP);
      int increment_start = curr_compiler_->function->codes_count();

      expression();
      emit_byte(OpCode::OP_POP);
      consume(TokenKind::TK_RPAREN, "expect `)` after `for` clauses");

      // after the increment, start the whole loop over
      emit_loop(loop_start);
      // at the end of the body, jump to the increment
      loop_start = increment_start;
      patch_jump(body_jump);
    }

    // compile the loop body
    statement();
    // jump back to the begining or the increment
    emit_loop(loop_start);

    if (exit_jump != -1) {
      patch_jump(exit_jump);
      emit_byte(OpCode::OP_POP); // condition
    }

    leave_scope();
  }

  void if_stmt(void) {
    consume(TokenKind::TK_LPAREN, "expect `(` before if condition");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

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
  }

  void return_stmt(void) {
    if (curr_compiler_->fun_kind == FunctionKind::TOP_LEVEL)
      error("cannot return from top-level code");

    if (match(TokenKind::TK_SEMI)) {
      emit_return();
    }
    else {
      if (curr_compiler_->fun_kind == FunctionKind::CTOR)
        error("cannot return a value from an constructor");

      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after return value");
      emit_byte(OpCode::OP_RETURN);
    }
  }

  void while_stmt(void) {
    int loop_start = curr_compiler_->function->codes_count();

    consume(TokenKind::TK_LPAREN, "expect `(` before while condition");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

    // jump out of the loop if the condition is false
    int exit_jump = emit_jump(OpCode::OP_JUMP_IF_FALSE);
    // compile the loop body
    emit_byte(OpCode::OP_POP); // condition
    statement();

    emit_loop(loop_start); // loop back to the start
    patch_jump(exit_jump);
    emit_byte(OpCode::OP_POP); // condition
  }

  void block_stmt(void) {
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
      declaration();

    consume(TokenKind::TK_RBRACE, "expect `}` after block");
  }

  void fn_common(FunctionKind fun_kind) {
    CompilerImpl fn_compiler;
    begin_compiler(fn_compiler, 1, fun_kind);
    enter_scope();
    consume(TokenKind::TK_LPAREN, "expect `(` after function name");
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        u8_t param_constant = parse_variable("expect parameter name");
        define_variable(param_constant);
        curr_compiler_->function->inc_arity();

        if (curr_compiler_->function->arity() > kMaxArguments) {
          error("cannot have more than " +
              std::to_string(kMaxArguments) + " parameters");
        }
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after parameters");

    // the body
    consume(TokenKind::TK_LBRACE, "expect `{` before function body");
    block_stmt();
    leave_scope();
    auto* fn = finish_compiler();

    emit_bytes(OpCode::OP_CLOSURE, make_constant(fn));

    for (int i = 0; i < fn->upvalues_count(); ++i) {
      auto upval = fn_compiler.upvalues[i];
      emit_bytes(upval.local ? 1 : 0, upval.index);
    }
  }

  void method(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect method name");
    u8_t method_constant = identifier_constant(prev_);

    // if the method is named `ctor`, it's the constructor
    FunctionKind fun_kind{FunctionKind::METHOD};
    if (prev_.get_literal() == "ctor")
      fun_kind = FunctionKind::CTOR;
    fn_common(fun_kind);
    emit_bytes(OpCode::OP_METHOD, method_constant);
  }
};

FunctionObject* Compile::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  CompileParser p(vm, lex);

  CompilerImpl c;
  p.begin_compiler(c, 0, FunctionKind::TOP_LEVEL);
  p.advance();
  if (!p.match(TokenKind::TK_EOF)) {
    do {
      p.declaration();
    } while (!p.match(TokenKind::TK_EOF));
  }
  auto* fn = p.finish_compiler();

  return p.had_error() ? nullptr : fn;
}

void gray_compiler_roots(VM& vm) {
  if (_curr_cp == nullptr)
    return;

  auto* compiler_iter = _curr_cp->curr_compiler();
  while (compiler_iter != nullptr) {
    vm.gray_object(compiler_iter->function);
    compiler_iter = compiler_iter->enclosing;
  }
}

}
