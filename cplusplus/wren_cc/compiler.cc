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
#include "value.hh"
#include "vm.hh"
#include "compiler.hh"

namespace wrencc {

class Compiler;
using GrammerFn = std::function<void (Compiler*, bool)>;
using SignatureFn = std::function<void (Compiler*, str_t&)>;

enum class Precedence {
  NONE,
  LOWEST,

  ASSIGNMENT, // =
  LOGIC,      // && ||
  IS,         // is
  EQUALITY,   // == !=
  COMPARISON, // < <= > >=
  BITWISE,    // | &
  TERM,       // + -
  FACTOR,     // * / %
  UNARY,      // unary - ! ~
  CALL,       // ()
};

inline Precedence operator+(Precedence a, int b) {
  return Xt::as_type<Precedence>(Xt::as_type<int>(a) + b);
}

struct GrammerRule {
  GrammerFn prefix;
  GrammerFn infix;
  SignatureFn method;
  Precedence precedence;
  const char* name;
};

class Parser : private UnCopyable {
  VM& vm_;

  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool skip_newlines_{true};

  bool had_error_{};
public:
  Parser(VM& vm, Lexer& lex) noexcept
    : vm_(vm), lex_(lex) {
  }

  inline const Token& prev(void) const { return prev_; }
  inline const Token& curr(void) const { return curr_; }
  inline bool had_error(void) const { return had_error_; }
  inline void set_error(bool b = true) { had_error_ = b; }
  inline VM& get_vm(void) { return vm_; }

  inline void advance(void) {
    prev_ = curr_;

    for (;;) {
      curr_ = lex_.next_token();
      switch (curr_.kind()) {
      case TokenKind::TK_NL:
        if (!skip_newlines_) {
          skip_newlines_ = true;
          return;
        }
        break;
      case TokenKind::TK_LPAREN:
      case TokenKind::TK_LBRACKET:
      case TokenKind::TK_LBRACE:
      case TokenKind::TK_DOT:
      case TokenKind::TK_COMMA:
      case TokenKind::TK_STAR:
      case TokenKind::TK_SLASH:
      case TokenKind::TK_PERCENT:
      case TokenKind::TK_PLUS:
      case TokenKind::TK_MINUS:
      case TokenKind::TK_PIPE:
      case TokenKind::TK_PIPEPIPE:
      case TokenKind::TK_AMP:
      case TokenKind::TK_AMPAMP:
      case TokenKind::TK_BANG:
      case TokenKind::TK_EQ:
      case TokenKind::TK_LT:
      case TokenKind::TK_GT:
      case TokenKind::TK_LTEQ:
      case TokenKind::TK_GTEQ:
      case TokenKind::TK_EQEQ:
      case TokenKind::TK_BANGEQ:
      case TokenKind::KW_CLASS:
      case TokenKind::KW_ELSE:
      case TokenKind::KW_IF:
      case TokenKind::KW_IS:
      case TokenKind::KW_STATIC:
      case TokenKind::KW_VAR:
      case TokenKind::KW_WHILE:
        skip_newlines_ = true; return;
      default: skip_newlines_ = false; return;
      }
    }
  }
};

struct Scope {
  // the number of previously defined local variables when this scope was
  // created. used to know how many variables to discard when this scope is
  // exited
  int prev_locals{};

  // the scope enclosing this one, or nullptr if this is the top scope in
  // the function
  Scope* parent{};

  void set_scope(int _locals_count = 0, Scope* _parent = nullptr) {
    prev_locals = _locals_count;
    parent = _parent;
  }
};

#define PUSH_SCOPE\
  Scope _scope##__LINE__;\
  push_scope(&_scope##__LINE__)
#define POP_SCOPE pop_scope()

class Compiler : private UnCopyable {
  Parser& parser_;
  Compiler* parent_{};
  FunctionObject* fn_{};
  int num_codes_{};
  SymbolTable locals_;
  bool is_method_{};

  // the current local variable scope, initially nullptr
  Scope* scope_{};

  inline SymbolTable& vm_methods(void) { return parser_.get_vm().methods(); }
  inline SymbolTable& vm_gsymbols(void) { return parser_.get_vm().gsymbols(); }

  void error(const char* format, ...) {
    parser_.set_error(true);
    std::cerr << "[LINE:" << parser_.prev().lineno() << "] - "
      << "Compile ERROR on `" << parser_.prev().literal() << "` : ";

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    std::cerr << std::endl;
  }

  int intern_symbol(void) {
    return vm_methods().ensure(parser_.prev().as_string());
  }

  template <typename T> inline int emit_byte(T b) {
    return fn_->add_code(b);
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) {
    emit_byte(b1);
    emit_byte(b2);
  }

  inline void patch_jump(int offset) {
    // replaces the placeholder argument for a previous JUMP or JUMP_IF
    // instruction with an offset that jumps to the current end of bytecode

    fn_->set_code(offset, fn_->codes_count() - offset - 1);
  }

  inline void emit_constant(const Value& v) {
    u8_t b = fn_->add_constant(v);
    emit_bytes(Code::CONSTANT, b);
  }

  void push_scope(Scope* scope) {
    // starts a new local block scope

    scope->set_scope(locals_.count(), scope_);
    scope_ = scope;
  }

  void pop_scope(void) {
    // closes the last pushed block scope

    ASSERT(scope_ != nullptr, "cannot pop top-level scope");

    Scope* scope = scope_;
    // pop locals off the stack
    for (int i = scope->prev_locals; i < locals_.count(); ++i)
      emit_byte(Code::POP);

    locals_.truncate(scope->prev_locals);
    scope_ = scope->parent;
  }

  const GrammerRule& get_rule(TokenKind kind) const {
#define RULE(fn) [](Compiler* c, bool b) { c->fn(b); }
#define SIGN(fn) [](Compiler* c, str_t& n) { c->fn(n); }

#define UNUSED {nullptr, nullptr, nullptr, Precedence::NONE, nullptr}
#define PREFIX(fn) {RULE(fn), nullptr, nullptr, Precedence::NONE, nullptr}
#define INFIX(fn, prec) {nullptr, RULE(fn), nullptr, prec, nullptr}
#define INFIXOP(prec, name) {nullptr, RULE(infix_oper), SIGN(infix_signature), prec, name}
#define OPER(prec, name) {RULE(unary_oper), RULE(infix_oper), SIGN(mixed_signature), prec, name}
#define PREFIXOP(name) {RULE(unary_oper), nullptr, SIGN(unary_signature), Precedence::NONE, name}
#define PREFIXNAME {RULE(variable), nullptr, SIGN(parameters), Precedence::NONE, nullptr}
    static const GrammerRule _rules[] = {
      PREFIX(grouping),                       // PUNCTUATOR(LPAREN, "(")
      UNUSED,                                 // PUNCTUATOR(RPAREN, ")")
      UNUSED,                                 // PUNCTUATOR(LBRACKET, "[")
      UNUSED,                                 // PUNCTUATOR(RBRACKET, "]")
      UNUSED,                                 // PUNCTUATOR(LBRACE, "{")
      UNUSED,                                 // PUNCTUATOR(RBRACE, "}")
      UNUSED,                                 // PUNCTUATOR(COLON, ":")
      INFIX(call, Precedence::CALL),          // PUNCTUATOR(DOT, ".")
      UNUSED,                                 // PUNCTUATOR(COMMA, ",")
      INFIXOP(Precedence::FACTOR, "* "),      // PUNCTUATOR(STAR, "*")
      INFIXOP(Precedence::FACTOR, "/ "),      // PUNCTUATOR(SLASH, "/")
      INFIXOP(Precedence::TERM, "% "),        // PUNCTUATOR(PERCENT, "%")
      INFIXOP(Precedence::TERM, "+ "),        // PUNCTUATOR(PLUS, "+")
      OPER(Precedence::TERM, "- "),           // PUNCTUATOR(MINUS, "-")
      UNUSED,                                 // PUNCTUATOR(PIPE, "|")
      INFIX(or_expr, Precedence::LOGIC),      // PUNCTUATOR(PIPEPIPE, "||")
      UNUSED,                                 // PUNCTUATOR(AMP, "&")
      INFIX(and_expr, Precedence::LOGIC),     // PUNCTUATOR(AMPAMP, "&&")
      PREFIXOP("!"),                          // PUNCTUATOR(BANG, "!")
      UNUSED,                                 // PUNCTUATOR(EQ, "=")
      INFIXOP(Precedence::COMPARISON, "< "),  // PUNCTUATOR(LT, "<")
      INFIXOP(Precedence::COMPARISON, "> "),  // PUNCTUATOR(GT, ">")
      INFIXOP(Precedence::COMPARISON, "<= "), // PUNCTUATOR(LTEQ, "<=")
      INFIXOP(Precedence::COMPARISON, ">= "), // PUNCTUATOR(GTEQ, ">=")
      INFIXOP(Precedence::EQUALITY, "== "),   // PUNCTUATOR(EQEQ, "==")
      INFIXOP(Precedence::EQUALITY, "!= "),   // PUNCTUATOR(BANGEQ, "!=")

      UNUSED,                                 // KEYWORD(CLASS, "class")
      UNUSED,                                 // KEYWORD(ELSE, "else")
      PREFIX(boolean),                        // KEYWORD(FALSE, "false")
      PREFIX(function),                       // KEYWORD(FN, "fn")
      UNUSED,                                 // KEYWORD(IF, "if")
      INFIX(is, Precedence::IS),              // KEYWORD(IS, "is")
      PREFIX(nil),                            // KEYWORD(NIL, "nil")
      UNUSED,                                 // KEYWORD(STATIC, "static")
      PREFIX(this_exp),                       // KEYWORD(THIS, "this")
      PREFIX(boolean),                        // KEYWORD(TRUE, "true")
      UNUSED,                                 // KEYWORD(VAR, "var")
      UNUSED,                                 // KEYWORD(WHILE, "while")

      PREFIXNAME,                             // TOKEN(IDENTIFIER, "identifier")
      PREFIX(numeric),                        // TOKEN(NUMERIC, "numeric")
      PREFIX(string),                         // TOKEN(STRING, "string")

      UNUSED,                                 // TOKEN(NL, "new-line")

      UNUSED,                                 // TOKEN(ERROR, "error")
      UNUSED,                                 // TOKEN(EOF, "eof")
    };
#undef SIGN
#undef RULE

    return _rules[Xt::as_type<int>(kind)];
  }

  void parse_precedence(bool allow_assignment, Precedence precedence) {
    parser_.advance();
    auto& prefix_fn = get_rule(parser_.prev().kind()).prefix;
    if (!prefix_fn) {
      error("no prefix parser");
      return;
    }

    prefix_fn(this, allow_assignment);

    while (precedence <= get_rule(parser_.curr().kind()).precedence) {
      parser_.advance();

      auto& infix_fn = get_rule(parser_.prev().kind()).infix;
      if (infix_fn)
        infix_fn(this, allow_assignment);
    }
  }

  int declare_variable(void) {
    consume(TokenKind::TK_IDENTIFIER, "expected variable name");

    int symbol;
    str_t name = parser_.prev().as_string();
    // the top-level scope of the top-level compiler is global scope
    if (parent_ == nullptr && scope_ == nullptr)
      symbol = vm_gsymbols().add(name); // top level global variable
    else
      symbol = locals_.add(name); // nested block, this is a local variable

    if (symbol == -1)
      error("variable is already defined");
    return symbol;
  }

  void define_variable(int symbol) {
    // the top-level scope of the top-level compiler is global scope
    if (parent_ == nullptr && scope_ == nullptr) {
      // it's a global variable, so store the value int the global slot
      emit_bytes(Code::STORE_GLOBAL, symbol);
    }
    else {
      // it's a local variable, the value is already in the right slot to store
      // the local, but later code will pop and discard that. to cancel that out
      // duplicate it now, so that the temporary value will be discarded and
      // leave the local still on the stack.
      //
      //    var a = "value"
      //    io.write(a)
      //
      //    CONSTANT "value"    -> put constant into local slot
      //    DUP                 -> dup it so the top is a temporary
      //    POP                 -> discard previous result in sequence
      //    <code for io.write>
      //
      // would be good to either peephole optimize this or be smarted about
      // generating code for defining local variables to not emit the DUP
      // sometimes
      emit_byte(Code::DUP);
    }
  }

  void nil(bool allow_assignment) {
    emit_byte(Code::NIL);
  }

  void boolean(bool allow_assignment) {
    if (parser_.prev().kind() == TokenKind::KW_TRUE)
      emit_byte(Code::TRUE);
    else
      emit_byte(Code::FALSE);
  }

  void numeric(bool allow_assignment) {
    auto& tok = parser_.prev();
    emit_constant(tok.as_numeric());
  }

  void string(bool allow_assignment) {
    auto& tok = parser_.prev();
    emit_constant(StringObject::make_string(
          parser_.get_vm(), tok.as_string()));
  }

  void variable(bool allow_assignment) {
    str_t variable_name = parser_.prev().as_string();
    int local = locals_.get(variable_name);

    // see if it's a global variable
    int global = 0;
    if (local == -1)
      global = vm_gsymbols().get(variable_name);

    if (local == -1 && global == -1)
      error("undefined variable");

    // if there's an `=` after a bare name, it's a variable assignment
    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      // compile the right-hand side
      statement();

      if (local != -1) {
        emit_bytes(Code::STORE_LOCAL, local);
        return;
      }

      emit_bytes(Code::STORE_GLOBAL, global);
      return;
    }

    // otherwise, it's just a variable access

    if (local != -1) {
      emit_bytes(Code::LOAD_LOCAL, local);
      return;
    }
    emit_bytes(Code::LOAD_GLOBAL, global);
  }

  bool match(TokenKind expected) {
    if (parser_.curr().kind() != expected)
      return false;

    parser_.advance();
    return true;
  }

  void consume(TokenKind expected, const char* msg) {
    parser_.advance();

    if (parser_.prev().kind() != expected)
      error(msg);
  }

  void definition(void) {
    if (match(TokenKind::KW_CLASS)) {
      // create a variable to store the class in
      int symbol = declare_variable();

      // load the superclass (if there is one)
      if (match(TokenKind::KW_IS)) {
        parse_precedence(false, Precedence::CALL);
        emit_byte(Code::SUBCLASS);
      }
      else {
        // create the empty class
        emit_byte(Code::CLASS);
      }

      // store it in its name
      define_variable(symbol);
      // compile the method definition
      consume(TokenKind::TK_LBRACE, "expect `{` before class body");
      while (!match(TokenKind::TK_RBRACE)) {
        bool is_static = match(TokenKind::KW_STATIC);
        auto& signature = get_rule(parser_.curr().kind()).method;
        parser_.advance();

        if (!signature) {
          error("expect method definition");
          break;
        }
        method(is_static, signature);
        consume(TokenKind::TK_NL, "expect newline after definition in class");
      }
      return;
    }

    if (match(TokenKind::KW_VAR)) {
      int symbol = declare_variable();
      consume(TokenKind::TK_EQ, "expect `=` after variable name");
      // compile the initializer
      statement();
      define_variable(symbol);
      return;
    }
    statement();
  }

  void statement(void) {
    if (match(TokenKind::KW_IF)) {
      // compile the condition
      consume(TokenKind::TK_LPAREN, "expect `(` after `if` keyword");
      assignment();
      consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

      // jump to the else branch if the condition is false
      emit_byte(Code::JUMP_IF);
      int if_jump = emit_byte(0xff);

      // compile the then branch
      PUSH_SCOPE;
      definition();
      POP_SCOPE;

      // jump over the else branch when the if branch is taken
      emit_byte(Code::JUMP);
      int else_jump = emit_byte(0xff);

      patch_jump(if_jump);
      // compile the else branch if thers is one
      if (match(TokenKind::KW_ELSE)) {
        PUSH_SCOPE;
        definition();
        POP_SCOPE;
      }
      else {
        emit_byte(Code::NIL); // just default to nil
      }
      // patch the jump over the else
      patch_jump(else_jump);

      return;
    }

    if (match(TokenKind::KW_WHILE)) {
      // remember what instruction to loop back to
      int loop_start = fn_->codes_count() - 1;

      // compile the condition
      consume(TokenKind::TK_LPAREN, "expect `(` after while keyword");
      assignment();
      consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

      emit_byte(Code::JUMP_IF);
      int exit_jump = emit_byte(0xff);

      // compile the while body
      PUSH_SCOPE;
      definition();
      POP_SCOPE;

      // loop back to the while top
      emit_byte(Code::LOOP);
      int loop_offset = fn_->codes_count() - loop_start;
      emit_byte(loop_offset);

      patch_jump(exit_jump);

      // a while loop always evaluates to nil
      emit_byte(Code::NIL);
      return;
    }

    // curly block
    if (match(TokenKind::TK_LBRACE)) {
      PUSH_SCOPE;
      for (;;) {
        definition();

        // if there is no newline, it must be the end of the block on the same line
        if (!match(TokenKind::TK_NL)) {
          consume(TokenKind::TK_RBRACE, "expect `}` after block body");
          break;
        }

        if (match(TokenKind::TK_RBRACE))
          break;

        // discard the result of the previous expression
        emit_byte(Code::POP);
      }
      POP_SCOPE;
      return;
    }

    assignment();
  }

  void expression(bool allow_assignment) {
    parse_precedence(allow_assignment, Precedence::LOWEST);
  }

  void grouping(bool allow_assignment) {
    assignment();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
  }

  void function(bool allow_assignment) {
    Compiler fn_compiler(parser_, this, false);
    // add the function to the constant table
    u8_t fn_constant = fn_->add_constant(fn_compiler.fn_);

    // define a fake local slot for the receiver (the function object itself)
    // so that later locals have the correct slot indices
    fn_compiler.locals_.add("(this)");

    str_t dummy_name;
    fn_compiler.parameters(dummy_name);

    if (fn_compiler.match(TokenKind::TK_LBRACE)) {
      for (;;) {
        fn_compiler.definition();

        if (!fn_compiler.match(TokenKind::TK_NL)) {
          fn_compiler.consume(
              TokenKind::TK_RBRACE, "expect `}` after function body");
          break;
        }
        if (fn_compiler.match(TokenKind::TK_RBRACE))
          break;
        fn_compiler.emit_byte(Code::POP);
      }
    }
    else {
      fn_compiler.expression(false);
    }
    fn_compiler.emit_byte(Code::END);

    emit_bytes(Code::CONSTANT, fn_constant);
  }

  void method(bool is_static, const SignatureFn& signature) {
    // compiles a method definition inside a class body
    // consume(TokenKind::TK_IDENTIFIER, "expect method name");

    Compiler method_compiler(parser_, this, true);

    // add the block into the constant table
    int method_constant = fn_->add_constant(method_compiler.fn_);

    // define a fake local slot for the receiver so that later locals have the
    // correct slot indices
    method_compiler.locals_.add("(this)");

    // build the method name, the mangled name includes all of the name parts
    // in a mixfix call as well as spaces for every argument.
    // so a method like:
    //    foo.bar(a, b) else (c) last
    //
    // will have name: "bar  else last"
    str_t name(parser_.prev().as_string());
    signature(&method_compiler, name);
    int symbol = vm_methods().ensure(name);

    consume(TokenKind::TK_LBRACE, "expect `{` to begin method body");
    // block body
    for (;;) {
      method_compiler.definition();

      // if there is no newline, is must be the end of the block on the same line
      if (!method_compiler.match(TokenKind::TK_NL)) {
        method_compiler.consume(
            TokenKind::TK_RBRACE, "expect `}` after method body");
        break;
      }

      if (method_compiler.match(TokenKind::TK_RBRACE))
        break;

      // discard the result of the previous expression
      method_compiler.emit_byte(Code::POP);
    }
    method_compiler.emit_byte(Code::END);

    if (is_static)
      emit_byte(Code::METACLASS);

    // compile the code to define the method it
    emit_byte(Code::METHOD);
    emit_byte(symbol);
    emit_byte(method_constant);

    if (is_static)
      emit_byte(Code::POP);
  }

  void assignment(void) {
    // assignment statement
    expression(true);
  }

  void call(bool allow_assignment) {
    str_t name;
    int argc = 0;

    consume(TokenKind::TK_IDENTIFIER, "expect method name after `.`");
    name += parser_.prev().as_string();
    if (match(TokenKind::TK_LPAREN)) {
      do {
        statement();
        ++argc;

        name.push_back(' ');
      } while (match(TokenKind::TK_COMMA));
      consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    }
    int symbol = vm_methods().ensure(name);

    // compile the method call
    emit_bytes(Code::CALL_0 + argc, symbol);
  }

  void is(bool allow_assignment) {
    // compile the right-hand side
    parse_precedence(false, Precedence::CALL);
    emit_byte(Code::IS);
  }

  void and_expr(bool allow_assignment) {
    // skip the right argument if the left is false

    emit_byte(Code::AND);
    int jump = emit_byte(0xff);

    parse_precedence(false, Precedence::LOGIC);
    patch_jump(jump);
  }

  void or_expr(bool allow_assignment) {
    // skip the right argument if the left if true

    emit_byte(Code::OR);
    int jump = emit_byte(0xff);

    parse_precedence(false, Precedence::LOGIC);
    patch_jump(jump);
  }

  void infix_oper(bool allow_assignment) {
    auto& rule = get_rule(parser_.prev().kind());

    // compile the right hand side
    parse_precedence(false, rule.precedence + 1);

    // call the operator method on the left-hand side
    int symbol = vm_methods().ensure(rule.name);
    emit_bytes(Code::CALL_1, symbol);
  }

  void unary_oper(bool allow_assignment) {
    auto& rule = get_rule(parser_.prev().kind());

    // compile the argument
    parse_precedence(false, Precedence::UNARY + 1);

    // call the operator method on the left-hand side
    int symbol = vm_methods().ensure(str_t(1, rule.name[0]));
    emit_bytes(Code::CALL_0, symbol);
  }

  void infix_signature(str_t& name) {
    // compiles a method signature for an infix operator

    // add a space for the RHS parameter
    name.push_back(' ');

    // parse the parameter name
    declare_variable();
  }

  void unary_signature(str_t& name) {
    // compiles a method signature for an unary operator
  }

  void mixed_signature(str_t& name) {
    // if there is a parameter name, it's an infix operator, otherwise it's unary
    if (parser_.curr().kind() == TokenKind::TK_IDENTIFIER) {
      // add a space for the RHS parameter
      name.push_back(' ');

      // parse the parameter name
      declare_variable();
    }
  }

  void parameters(str_t& name) {
    // parse the parameter list, if any
    if (match(TokenKind::TK_LPAREN)) {
      do {
        // define a local variable in the method for the parameters
        declare_variable();

        // add a space in the name for the parameters
        name.push_back(' ');
      } while (match(TokenKind::TK_COMMA));
      consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
    }
  }

  void this_exp(bool allow_assignment) {
    // walk up the parent chain to see if there is an enclosing method
    Compiler* this_compiler = this;
    bool inside_method = false;
    while (this_compiler != nullptr) {
      if (this_compiler->is_method_) {
        inside_method = true;
        break;
      }
      this_compiler = this_compiler->parent_;
    }

    if (!inside_method) {
      error("cannot use `this` outside of a method");
      return;
    }

    // the receiver is always stored in the first local slot
    emit_bytes(Code::LOAD_LOCAL, 0);
  }
public:
  Compiler(Parser& parser,
      Compiler* parent = nullptr, bool is_method = false) noexcept
    : parser_(parser)
    , parent_(parent)
    , is_method_(is_method) {
    fn_ = FunctionObject::make_function(parser_.get_vm());
  }

  ~Compiler(void) {
    // FIXME: fixed deallocate FunctionObject by GC
  }

  FunctionObject* compile_function(TokenKind end_kind) {
    parser_.get_vm().pin_object(fn_);
    for (;;) {
      definition();

      // if there is no newline, it must be the end of the block on the same line
      if (!match(TokenKind::TK_NL)) {
        consume(end_kind, "expect end of file");
        break;
      }
      if (match(end_kind))
        break;
      emit_byte(Code::POP);
    }
    emit_byte(Code::END);

    parser_.get_vm().unpin_object(fn_);
    return parser_.had_error() ? nullptr : fn_;
  }

  FunctionObject* compile_function(
      Parser& p, Compiler* parent, TokenKind end_kind) {
    Compiler c(p, parent, false);
    return c.compile_function(end_kind);
  }
};

FunctionObject* compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Parser p(vm, lex);

  p.advance();
  Compiler c(p, nullptr, false);

  return c.compile_function(TokenKind::TK_EOF);
}

}
