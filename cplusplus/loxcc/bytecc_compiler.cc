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
    // Code::JUMP_IF_FALSE ------
    // Code::POP                |
    // right operand            |
    // <-------------------------
    // ... other bytecodes

    // short circuit if the left operand is false
    int end_jump = emit_jump(Code::JUMP_IF_FALSE);

    // compile the right operand
    emit_byte(Code::POP);
    parse_precedence(Precedence::AND);

    patch_jump(end_jump);
  }

  const ParseRule& get_rule(TokenKind kind) {
    // TODO:

    static const ParseRule _rules[] = {
      {nullptr, nullptr, Precedence::NONE},
    };

    return _rules[Xt::as_type<int>(kind)];
  }

  void parse_precedence(Precedence precedence) {
    // TODO:
  }

  void declaration(void) {}
  void statement(void) {}
  void expression(void) {}
public:
};

FunctionObject* GlobalCompiler::compile(VM& vm, const str_t& source_bytes) {
  // TODO:
  return nullptr;
}

void GlobalCompiler::mark_roots(VM& vm) {
  // TODO:
}

}
