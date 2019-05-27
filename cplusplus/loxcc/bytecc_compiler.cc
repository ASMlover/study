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
  int depth{-1};
  bool is_upvalue{};

  LocalVar(const Token& arg_name,
      int arg_depth = -1, bool arg_upvalue = false) noexcept
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
  static constexpr int kMaxArguments = 8;

  VM& vm_;
  Lexer& lex_;

  Compiler* curr_compiler_{};
  ClassCompiler* curr_class_{};

  Token curr_;
  Token prev_;

  bool had_error_{};
  bool panic_mode_{};

  inline Chunk* curr_chunk(void) const { return curr_compiler_->fn->chunk(); }
  inline void error(const str_t& msg) { error_at(prev_, msg); }
  inline void error_at_current(const str_t& msg) { error_at(curr_, msg); }

  void error_at(const Token& tok, const str_t& msg) {
    if (panic_mode_)
      return;

    panic_mode_ = true;

    std::cerr << "[LINE: " << tok.lineno() << "] ERROR ";
    if (tok.kind() == TokenKind::TK_EOF)
      std::cerr << " at end";
    else if (tok.kind() == TokenKind::TK_ERR)
      (void)0;
    else
      std::cerr << " at " << tok.literal();
    std::cerr << ": " << msg << std::endl;

    panic_mode_ = false;
  }

  void advance(void) {
    prev_ = curr_;

    for (;;) {
      curr_ = lex_.next_token();
      if (curr_.kind() != TokenKind::TK_ERR)
        break;

      error_at_current(curr_.literal());
    }
  }

  void consume(TokenKind kind, const str_t& msg) {
    if (curr_.kind() == kind)
      advance();
    else
      error_at_current(msg);
  }

  bool check(TokenKind kind) const {
    return curr_.kind() == kind;
  }

  bool match(TokenKind kind) {
    if (check(kind)) {
      advance();
      return true;
    }

    return false;
  }

  template <typename T> inline void emit_byte(T b) {
    curr_chunk()->write(b, prev_.lineno());
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) {
    emit_byte(b1);
    emit_byte(b2);
  }

  void emit_loop(int loop_start) {
    emit_byte(Code::LOOP);

    int offset = curr_chunk()->codes_count() - loop_start + 2;
    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
  }

  template <typename T> int emit_jump(T instruction) {
    // emits [instruction] followed by a placeholder for a jump offset.
    // the placeholder can be patched by calling <patch_jump>.
    // returns the index of the placeholder

    emit_byte(instruction);
    emit_bytes(0xff, 0xff);
    return curr_chunk()->codes_count() - 2;
  }

  void emit_return(void) {
    if (curr_compiler_->type == FunctionType::CTOR)
      emit_bytes(Code::GET_LOCAL, 0);
    else
      emit_byte(Code::NIL);
    emit_byte(Code::RETURN);
  }

  u8_t make_constant(const Value& value) {
    return curr_chunk()->add_constant(value);
  }

  void emit_constant(const Value& value) {
    emit_bytes(Code::CONSTANT, make_constant(value));
  }

  void patch_jump(int offset) {
    // replaces the placeholder argument for a previous Code::JUMP or
    // Code::JUMP_IF_FALSE instruction with an offset that jumps to
    // the current end of bytecode.

    // -2 to adjust for the bytecode for the jump offset itself
    int jump = curr_chunk()->codes_count() - offset - 2;
    curr_chunk()->set_code(offset, (jump >> 8) & 0xff);
    curr_chunk()->set_code(offset + 1, jump & 0xff);
  }

  void init_compiler(Compiler* compiler, int scope_depth, FunctionType type) {
    StringObject* func_name;
    switch (type) {
    case FunctionType::CTOR:
    case FunctionType::METHOD:
    case FunctionType::FUNCTION:
      func_name = StringObject::create(vm_, prev_.as_string()); break;
    case FunctionType::TOP_LEVEL:
      func_name = nullptr; break;
    }

    compiler->enclosing = curr_compiler_;
    compiler->fn = FunctionObject::create(vm_);
    compiler->fn->set_name(func_name);
    compiler->type = type;
    compiler->scope_depth = scope_depth;

    curr_compiler_ = compiler;

    if (type != FunctionType::FUNCTION) {
      // in a method, it holds the instance object, "use `this`"
      curr_compiler_->locals.push_back(LocalVar(
            Token::make_custom("this"), curr_compiler_->scope_depth, false));
    }
    else {
      // in a function, it holds the function, but cannot be referenced,
      // so it has no name
      curr_compiler_->locals.push_back(LocalVar(
            Token::make_custom(""), curr_compiler_->scope_depth, false));
    }
  }

  FunctionObject* finish_compiler(void) {
    emit_return();

    FunctionObject* fn = curr_compiler_->fn;
#if defined(DEBUG_CODE)
    if (!had_error_)
      curr_chunk()->dis(fn->name_astr());
#endif
    curr_compiler_ = curr_compiler_->enclosing;

    return fn;
  }

  void enter_scope(void) {
    ++curr_compiler_->scope_depth;
  }

  void leave_scope(void) {
    --curr_compiler_->scope_depth;

    while (!curr_compiler_->locals.empty() &&
        curr_compiler_->locals.back().depth > curr_compiler_->scope_depth) {
      if (curr_compiler_->locals.back().is_upvalue)
        emit_byte(Code::CLOSE_UPVALUE);
      else
        emit_byte(Code::POP);

      curr_compiler_->locals.pop_back();
    }
  }

  u8_t identifier_constant(const Token& name) {
    return make_constant(StringObject::create(vm_, name.as_string()));
  }

  int resolve_local(Compiler* compiler, const Token& name) {
    int i = Xt::as_type<int>(compiler->locals.size()) - 1;
    for (; i >= 0; --i) {
      auto& local = compiler->locals[i];
      if (local.name == name) {
        if (local.depth == -1)
          error("cannot read local variable in its own initializer");
        return i;
      }
    }
    return -1;
  }

  int add_upvalue(Compiler* compiler, u8_t index, bool is_local) {
    // add an upvalue to [compiler]'s function with the given properties.
    // does not add one if an upvalue for that variable is already in the
    // list.
    // returns the index of the upvalues

    // find an existing one
    for (int i = 0; i < compiler->fn->upvalues_count(); ++i) {
      auto& upvalue = compiler->upvalues[i];
      if (upvalue.index == index && upvalue.is_local == is_local)
        return i;
    }

    // we need a new upvalue if we got here
    compiler->upvalues.push_back(Upvalue(index, is_local));
    return compiler->fn->inc_upvalues_count();
  }

  int resolve_upvalue(Compiler* compier, const Token& name) {
    // attempts to look up [name] in the functions enclosing the one beging
    // compiled by [compiler]. if found, adds an upvalue for it to this
    // compiler's list of upvalues and returns its index. otherwise return -1
    //
    // if the name is found outside of the immediately enclosing function,
    // this will flatten the closure and add upvalues to all of the intermediate
    // functions fo that it gets walked down to this one.

    // if we are at the top level, we do not need find it
    if (compier->enclosing == nullptr)
      return -1;

    // check if it's a local variable in the immediately enclosing function
    int local = resolve_local(compier->enclosing, name);
    if (local != -1) {
      // mark the local as an upvalue so we know to close it when it
      // goes out of the scope
      compier->enclosing->locals[local].is_upvalue = true;
      return add_upvalue(compier, Xt::as_type<u8_t>(local), true);
    }

    // check if it's an upvalue in the immediately enclosing function. in
    // other words, if it's a local variable in a non-immediately enclosing
    // function. this "flattens" closures automatically adds upvalues to all
    // of the intermediate functions to get from the function where a local
    // is declared all the way into the possibly deeply nested function that
    // is closing over it.
    int upvalue = resolve_upvalue(compier->enclosing, name);
    if (upvalue != -1)
      return add_upvalue(compier, Xt::as_type<u8_t>(upvalue), false);

    // we walked all the way up he parent chain and not found it
    return -1;
  }

  void declare_variable(void) {
    // global variables are implicitly declared
    if (curr_compiler_->scope_depth == 0)
      return;

    const Token& name = prev_;
    int i = Xt::as_type<int>(curr_compiler_->locals.size()) - 1;
    for (; i >= 0; --i) {
      auto& local = curr_compiler_->locals[i];
      if (local.depth != -1 && local.depth < curr_compiler_->scope_depth)
        break;

      if (local.name == name)
        error("variable with this name already declared in this scope");
    }

    curr_compiler_->locals.push_back(LocalVar(name, -1, false));
  }

  u8_t parse_variable(const str_t& err_msg) {
    consume(TokenKind::TK_IDENTIFIER, err_msg);

    declare_variable();
    if (curr_compiler_->scope_depth > 0)
      return 0;

    return identifier_constant(prev_);
  }

  void make_initialized(void) {
    // if in global scope, do nothing
    if (curr_compiler_->scope_depth == 0)
      return;

    curr_compiler_->locals.back().depth = curr_compiler_->scope_depth;
  }

  void define_variable(u8_t global) {
    if (curr_compiler_->scope_depth > 0) {
      // goto initialize the local variable scope depth
      make_initialized();
      return;
    }

    emit_bytes(Code::DEF_GLOBAL, global);
  }

  void named_variable(const Token& name, bool can_assign) {
    Code getop, setop;
    int arg = resolve_local(curr_compiler_, name);
    if (arg != -1) {
      getop = Code::GET_LOCAL;
      setop = Code::SET_LOCAL;
    }
    else if ((arg = resolve_upvalue(curr_compiler_, name)) != -1) {
      getop = Code::GET_UPVALUE;
      setop = Code::SET_UPVALUE;
    }
    else {
      arg = identifier_constant(name);
      getop = Code::GET_GLOBAL;
      setop = Code::SET_GLOBAL;
    }

    if (can_assign && match(TokenKind::TK_EQ)) {
      expression();
      emit_bytes(setop, arg);
    }
    else {
      emit_bytes(getop, arg);
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

  void and_exp(bool can_assign) {
    // left operand ...
    // Code::JUMP_IF_FALSE -----.
    // Code::POP                |
    // right operand            |
    // <------------------------'
    // ... other bytecodes

    // short circuit if the left operand is false
    int end_jump = emit_jump(Code::JUMP_IF_FALSE);

    // compile the right operand
    emit_byte(Code::POP);
    parse_precedence(Precedence::AND);

    patch_jump(end_jump);
  }

  void or_exp(bool can_assign) {
    // left operand ...
    // Code::JUMP_IF_FALSE --.
    // Code::JUMP          --+----.
    // <---------------------'    |
    // Code::POP                  |
    // right operand ...          |
    // <--------------------------'
    // ... other bytecodes

    // if the operand is "true" we want to keep it, so when it's false,
    // jump to the code to evaluate the right operand
    int else_jump = emit_jump(Code::JUMP_IF_FALSE);
    // the right operand is true, jump to the end to keep it
    int end_jump = emit_jump(Code::JUMP);

    // compile the right operand
    patch_jump(else_jump);
    emit_byte(Code::POP); // pop out the left operand

    parse_precedence(Precedence::OR);
    patch_jump(end_jump);
  }

  void binary(bool can_assign) {
    TokenKind oper_kind = prev_.kind();

    // compile the right operand
    auto& rule = get_rule(oper_kind);
    parse_precedence(rule.precedence + 1);

    // emit the operator instruction
    switch (oper_kind) {
    case TokenKind::TK_EQ: emit_byte(Code::EQ); break;
    case TokenKind::TK_BANGEQ: emit_byte(Code::NE); break;
    case TokenKind::TK_GT: emit_byte(Code::GT); break;
    case TokenKind::TK_GTEQ: emit_byte(Code::GE); break;
    case TokenKind::TK_LT: emit_byte(Code::LT); break;
    case TokenKind::TK_LTEQ: emit_byte(Code::LE); break;
    case TokenKind::TK_PLUS: emit_byte(Code::ADD); break;
    case TokenKind::TK_MINUS: emit_byte(Code::SUB); break;
    case TokenKind::TK_STAR: emit_byte(Code::MUL); break;
    case TokenKind::TK_SLASH: emit_byte(Code::DIV); break;
    default: return; // unreachable
    }
  }

  void call(bool can_assign) {
    emit_byte(Code::CALL_0 + argument_list());
  }

  void dot(bool can_assign) {
    consume(TokenKind::TK_IDENTIFIER, "expect attribute name after `.`");
    u8_t name_constant = identifier_constant(prev_);

    if (can_assign && match(TokenKind::TK_EQ)) {
      expression();
      emit_bytes(Code::SET_ATTR, name_constant);
    }
    else if (match(TokenKind::TK_LPAREN)) {
      emit_bytes(Code::INVOKE_0 + argument_list(), name_constant);
    }
    else {
      emit_bytes(Code::GET_ATTR, name_constant);
    }
  }

  void literal(bool can_assign) {
    switch (prev_.kind()) {
    case TokenKind::KW_NIL: emit_byte(Code::NIL); break;
    case TokenKind::KW_TRUE: emit_byte(Code::TRUE); break;
    case TokenKind::KW_FALSE: emit_byte(Code::FALSE); break;
    default: return; // unreachable
    }
  }

  void grouping(bool can_assign) {
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
  }

  void numeric(bool can_assign) {
    emit_constant(prev_.as_numeric());
  }

  void string(bool can_assign) {
    emit_constant(StringObject::create(vm_, prev_.as_string()));
  }

  void variable(bool can_assign) {
    named_variable(prev_, can_assign);
  }

  void super_exp(bool can_assign) {
    auto push_superclass = [](GlobalParser* p) {
      if (p->curr_class_ != nullptr)
        p->named_variable(Token::make_custom("super"), false);
    };

    if (curr_class_ == nullptr)
      error("cannot use `super` outside of a class");
    else if (!curr_class_->has_superclass)
      error("cannot use `super` in a class without superclass");

    consume(TokenKind::TK_DOT, "expect `.` after `super`");
    consume(TokenKind::TK_IDENTIFIER, "expect superclass method name");
    u8_t name_constant = identifier_constant(prev_);

    // push the receiver instance object
    named_variable(Token::make_custom("this"), false);

    if (match(TokenKind::TK_LPAREN)) {
      u8_t argc = argument_list();

      push_superclass(this);
      emit_bytes(Code::SUPER_0 + argc, name_constant);
    }
    else {
      push_superclass(this);
      emit_bytes(Code::GET_SUPER, name_constant);
    }
  }

  void this_exp(bool can_assign) {
    if (curr_class_ != nullptr)
      variable(false);
    else
      error("cannot use `this` outside of a class");
  }

  void unary(bool can_assign) {
    TokenKind oper_kind = prev_.kind();
    parse_precedence(Precedence::UNARY);

    switch (oper_kind) {
    case TokenKind::TK_BANG: emit_byte(Code::NOT); break;
    case TokenKind::TK_MINUS: emit_byte(Code::NEG); break;
    default: return; // unreachable
    }
  }

  const ParseRule& get_rule(TokenKind kind) {
    auto grouping_fn = [](GlobalParser* p, bool b) { p->grouping(b); };
    auto call_fn = [](GlobalParser* p, bool b) { p->call(b); };
    auto dot_fn = [](GlobalParser* p, bool b) { p->dot(b); };
    auto unary_fn = [](GlobalParser* p, bool b) { p->unary(b); };
    auto binary_fn = [](GlobalParser* p, bool b) { p->binary(b); };
    auto variable_fn = [](GlobalParser* p, bool b) { p->variable(b); };
    auto numeric_fn = [](GlobalParser* p, bool b) { p->numeric(b); };
    auto string_fn = [](GlobalParser* p, bool b) { p->string(b); };
    auto and_fn = [](GlobalParser* p, bool b) { p->and_exp(b); };
    auto or_fn = [](GlobalParser* p, bool b) { p->or_exp(b); };
    auto literal_fn = [](GlobalParser* p, bool b) { p->literal(b); };
    auto super_fn = [](GlobalParser* p, bool b) { p->super_exp(b); };
    auto this_fn = [](GlobalParser* p, bool b) { p->this_exp(b); };

    static const ParseRule _rules[] = {
      {grouping_fn, call_fn, Precedence::CALL}, // PUNCTUATOR(LPAREN, "(")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(RPAREN, ")")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(LBRACE, "{")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(RBRACE, "}")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(COMMA, ",")
      {nullptr, dot_fn, Precedence::CALL}, // PUNCTUATOR(DOT, ".")
      {unary_fn, binary_fn, Precedence::TERM}, // PUNCTUATOR(MINUS, "-")
      {nullptr, binary_fn, Precedence::TERM}, // PUNCTUATOR(PLUS, "+")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(SEMI, ";")
      {nullptr, binary_fn, Precedence::FACTOR}, // PUNCTUATOR(SLASH, "/")
      {nullptr, binary_fn, Precedence::FACTOR}, // PUNCTUATOR(STAR, "*")

      {unary_fn, nullptr, Precedence::NONE}, // PUNCTUATOR(BANG, "!")
      {nullptr, binary_fn, Precedence::EQUALITY}, // PUNCTUATOR(BANGEQ, "!=")
      {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(EQ, "=")
      {nullptr, binary_fn, Precedence::EQUALITY}, // PUNCTUATOR(EQEQ, "==")
      {nullptr, binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(GT, ">")
      {nullptr, binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(GTEQ, ">=")
      {nullptr, binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(LT, "<")
      {nullptr, binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(LTEQ, "<=")

      {variable_fn, nullptr, Precedence::NONE}, // TOKEN(IDENTIFIER, "identifier")
      {numeric_fn, nullptr, Precedence::NONE}, // TOKEN(NUMERIC, "numeric")
      {string_fn, nullptr, Precedence::NONE}, // TOKEN(STRING, "string")

      {nullptr, and_fn, Precedence::AND}, // KEYWORD(AND, "and")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(CLASS, "class")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(ELSE, "else")
      {literal_fn, nullptr, Precedence::NONE}, // KEYWORD(FALSE, "false")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(FOR, "for")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(FUN, "fun")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(IF, "if")
      {literal_fn, nullptr, Precedence::NONE}, // KEYWORD(NIL, "nil")
      {nullptr, or_fn, Precedence::OR}, // KEYWORD(OR, "or")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(PRINT, "print")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(RETURN, "return")
      {super_fn, nullptr, Precedence::NONE}, // KEYWORD(SUPER, "super")
      {this_fn, nullptr, Precedence::NONE}, // KEYWORD(THIS, "this")
      {literal_fn, nullptr, Precedence::NONE}, // KEYWORD(TRUE, "true")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(VAR, "var")
      {nullptr, nullptr, Precedence::NONE}, // KEYWORD(WHILE, "while")

      {nullptr, nullptr, Precedence::NONE}, // TOKEN(EOF, "eof")
      {nullptr, nullptr, Precedence::NONE}, // TOKEN(ERR, "error")
    };

    return _rules[Xt::as_type<int>(kind)];
  }

  void parse_precedence(Precedence precedence) {
    advance();
    auto prefix_fn = get_rule(prev_.kind()).prefix;
    if (!prefix_fn) {
      error("expect expression");
      return;
    }

    bool can_assign = precedence <= Precedence::ASSIGNMENT;
    prefix_fn(this, can_assign);

    while (precedence <= get_rule(curr_.kind()).precedence) {
      advance();

      auto infix_fn = get_rule(prev_.kind()).infix;
      if (infix_fn)
        infix_fn(this, can_assign);
    }

    if (can_assign && match(TokenKind::TK_EQ)) {
      error("invalid assignment target");
      expression();
    }
  }

  void block(void) {
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
      declaration();
    consume(TokenKind::TK_RBRACE, "expect `}` after block");
  }

  void function(FunctionType type) {
    Compiler compier;
    init_compiler(&compier, 1, type);

    // compile the parameters list
    consume(TokenKind::TK_LPAREN, "expect `(` after function name");
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        u8_t param_constant = parse_variable("expect parameter name");
        define_variable(param_constant);

        curr_compiler_->fn->inc_arity();
        if (curr_compiler_->fn->arity() > kMaxArguments) {
          error("cannot have more than " +
              std::to_string(kMaxArguments) + " parameters");
        }
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RPAREN, "expect `)` after function parameters");

    // compile the function body
    consume(TokenKind::TK_LBRACE, "expect `{` before function body");
    block();

    // create the function object
    leave_scope();
    FunctionObject* fn = finish_compiler();

    // capture the upvalues in the new closure object
    emit_bytes(Code::CLOSURE, make_constant(fn));

    // emit arguments for each upvalue to know whether to capture
    // a local or an upvalue
    for (int i = 0; i < fn->upvalues_count(); ++i) {
      emit_byte(compier.upvalues[i].is_local ? 1 : 0);
      emit_byte(compier.upvalues[i].index);
    }
  }

  void method(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect method name");
    u8_t method_constant = identifier_constant(prev_);

    // if the method name is "ctor", it's a constructor of class
    FunctionType type = FunctionType::METHOD;
    if (prev_.as_string() == "ctor")
      type = FunctionType::CTOR;
    function(type);

    emit_bytes(Code::METHOD, method_constant);
  }

  void synchronize(void) {
    panic_mode_ = false;

    while (curr_.kind() != TokenKind::TK_EOF) {
      if (prev_.kind() == TokenKind::TK_SEMI)
        return;

      switch (curr_.kind()) {
      case TokenKind::KW_CLASS:
      case TokenKind::KW_FUN:
      case TokenKind::KW_VAR:
      case TokenKind::KW_IF:
      case TokenKind::KW_WHILE:
      case TokenKind::KW_PRINT:
      case TokenKind::KW_RETURN:
        return;
      default: break; // do nothing
      }

      advance();
    }
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT);
  }

  void declaration(void) {
    if (match(TokenKind::KW_CLASS))
      class_decl();
    else if (match(TokenKind::KW_FUN))
      fun_decl();
    else if (match(TokenKind::KW_VAR))
      var_decl();
    else
      statement();

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
    else if (match(TokenKind::KW_PRINT)) {
      print_stmt();
    }
    else if (match(TokenKind::KW_RETURN)) {
      return_stmt();
    }
    else if (match(TokenKind::KW_WHILE)) {
      while_stmt();
    }
    else if (match(TokenKind::TK_LBRACE)) {
      enter_scope();
      block();
      leave_scope();
    }
    else {
      expr_stmt();
    }
  }

  void class_decl(void) {
    consume(TokenKind::TK_IDENTIFIER, "expect class name");
    Token class_name = prev_;
    u8_t name_constant = identifier_constant(prev_);
    declare_variable();

    emit_bytes(Code::CLASS, name_constant);
    define_variable(name_constant);

    ClassCompiler class_compiler(curr_class_, prev_, false);
    curr_class_ = &class_compiler;

    if (match(TokenKind::TK_LT)) {
      consume(TokenKind::TK_IDENTIFIER, "expect superclass name");
      if (class_name == prev_)
        error("class cannot inherit from itself");

      class_compiler.has_superclass = true;

      enter_scope();
      // store the superclass in a local variable named "super"
      variable(false);
      curr_compiler_->locals.push_back(
          LocalVar(Token::make_custom("super"), -1, false));
      define_variable(0);

      named_variable(class_name, false);
      emit_byte(Code::SUBCLASS);
    }

    consume(TokenKind::TK_LBRACE, "expect `{` before class body");
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE)) {
      named_variable(class_name, false);
      method();
    }
    consume(TokenKind::TK_RBRACE, "expect `}` after class body");

    if (class_compiler.has_superclass)
      leave_scope();

    curr_class_ = curr_class_->enclosing;
  }

  void fun_decl(void) {
    u8_t global = parse_variable("expect function name");
    make_initialized();
    function(FunctionType::FUNCTION);
    define_variable(global);
  }

  void var_decl(void) {
    u8_t global = parse_variable("expect variable name");

    if (match(TokenKind::TK_EQ))
      expression();
    else
      emit_byte(Code::NIL);
    consume(TokenKind::TK_SEMI, "expect `;` after variable declaration");

    define_variable(global);
  }

  void expr_stmt(void) {
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after expression");
    emit_byte(Code::POP);
  }

  void for_stmt(void) {
    // for (var i = 0; i < 10; i = i + 1) stmt;
    //
    //    var i = 0;
    // _START:                          <---.
    //    if (i >= 10) goto _EXIT;  ----.   |
    //    goto _BODY;     ----------.   |   |
    // _INCREMENT:        <---------+---+---+---.
    //    i = i + i;                |   |   |   |
    //    goto _START;    ----------+---+---'   |
    // _BODY:             <---------'   |       |
    //    stmt;                         |       |
    //    goto _INCREMENT;--------------+-------'
    // _EXIT:             <-------------'

    // create a scope for loop variable
    enter_scope();

    // the for loop initialization clause
    consume(TokenKind::TK_LPAREN, "expect `(` after keyword `for`");
    if (match(TokenKind::KW_VAR))
      var_decl();
    else if (match(TokenKind::TK_SEMI))
      (void)0;
    else
      expr_stmt();

    int loop_start = curr_chunk()->codes_count();
    // the exit condition
    int exit_jump = -1;
    if (!match(TokenKind::TK_SEMI)) {
      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after loop condition");

      // jump out of the loop if the condition is false
      exit_jump = emit_jump(Code::JUMP_IF_FALSE);
      emit_byte(Code::POP); // pop out the for condition
    }

    // increment step
    if (!match(TokenKind::TK_RPAREN)) {
      // we donot want to execute the increment before the body,
      // so jump over it
      int body_jump = emit_jump(Code::JUMP);

      int increment_start = curr_chunk()->codes_count();
      expression();
      emit_byte(Code::POP);
      consume(TokenKind::TK_RPAREN, "expect `)` after loop clauses");

      // after the increment, start the whole loop
      emit_loop(loop_start);

      // at the end of the body, we want to jump to the increment,
      // not the top of the loop
      loop_start = increment_start;
      patch_jump(body_jump);
    }

    // compile the body
    statement();

    // jump back to the beginning (or the increment start)
    emit_loop(loop_start);

    if (exit_jump != -1) {
      patch_jump(exit_jump);
      emit_byte(Code::POP); // pop out the for condition
    }

    leave_scope();
  }

  void if_stmt(void) {
    consume(TokenKind::TK_LPAREN, "expect `(` after keyword `if`");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

    // jump to the else branch if the condition is false
    int else_jump = emit_jump(Code::JUMP_IF_FALSE);

    // compile the then branch
    emit_byte(Code::POP); // pop out the if condition
    statement();

    // jump over the else branch when the if branch is taken
    int end_jump = emit_jump(Code::JUMP);

    // compile the else branch
    patch_jump(else_jump);
    emit_byte(Code::POP); // pop out the if condition

    if (match(TokenKind::KW_ELSE))
      statement();

    patch_jump(end_jump);
  }

  void print_stmt(void) {
    // TODO: need implementation print *args ;
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after print expression");
    emit_byte(Code::PRINT);
  }

  void return_stmt(void) {
    if (curr_compiler_->type == FunctionType::TOP_LEVEL)
      error("cannot return from top-level code");

    if (match(TokenKind::TK_SEMI)) {
      emit_return();
    }
    else {
      if (curr_compiler_->type == FunctionType::CTOR)
        error("cannot return a value from class constructor");

      expression();
      consume(TokenKind::TK_SEMI, "expect `;` after return expression");
      emit_byte(Code::RETURN);
    }
  }

  void while_stmt(void) {
    int loop_start = curr_chunk()->codes_count();

    consume(TokenKind::TK_LPAREN, "expect `(` before while condition");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

    // jump out of the while loop if the condition is false
    int exit_jump = emit_jump(Code::JUMP_IF_FALSE);

    // compile the body
    emit_byte(Code::POP); // pop out the while condition
    statement();

    // loop back to the while start
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(Code::POP); // pop out the while condition
  }
public:
  GlobalParser(VM& vm, Lexer& lex) noexcept
    : vm_(vm), lex_(lex) {
  }

  inline Compiler* curr_compiler(void) const { return curr_compiler_; }

  FunctionObject* run_compile(void) {
    Compiler compiler;
    init_compiler(&compiler, 0, FunctionType::TOP_LEVEL);

    advance();
    while (!match(TokenKind::TK_EOF))
      declaration();

    FunctionObject* fn = finish_compiler();
    return had_error_ ? nullptr : fn;
  }
};

FunctionObject* GlobalCompiler::compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  gparser_ = std::make_shared<GlobalParser>(vm, lex);

  return gparser_->run_compile();
}

void GlobalCompiler::mark_roots(VM& vm) {
  Compiler* iter_compiter = gparser_->curr_compiler();
  while (iter_compiter != nullptr) {
    vm.mark_object(iter_compiter->fn);
    iter_compiter = iter_compiter->enclosing;
  }
}

}
