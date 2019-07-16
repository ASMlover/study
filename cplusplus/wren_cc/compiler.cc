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
  CALL,       // () []
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
  WrenVM& vm_;

  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool skip_newlines_{true};

  bool had_error_{};
public:
  Parser(WrenVM& vm, Lexer& lex) noexcept
    : vm_(vm), lex_(lex) {
  }

  inline const Token& prev(void) const { return prev_; }
  inline const Token& curr(void) const { return curr_; }
  inline bool had_error(void) const { return had_error_; }
  inline void set_error(bool b = true) { had_error_ = b; }
  inline WrenVM& get_vm(void) { return vm_; }

  void advance(void) {
    prev_ = curr_;

    // if we are out of tokens, donot try to tokenize any more we *do*
    // skill copy the TokenKind::TK_EOF to previois so that so that code
    // that expects it to be consumed will still work
    if (curr_.kind() == TokenKind::TK_EOF)
      return;

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
      case TokenKind::TK_TILDE:
      case TokenKind::TK_EQ:
      case TokenKind::TK_LT:
      case TokenKind::TK_GT:
      case TokenKind::TK_LTEQ:
      case TokenKind::TK_GTEQ:
      case TokenKind::TK_EQEQ:
      case TokenKind::TK_BANGEQ:
      case TokenKind::KW_CLASS:
      case TokenKind::KW_ELSE:
      case TokenKind::KW_FOR:
      case TokenKind::KW_IF:
      case TokenKind::KW_IN:
      case TokenKind::KW_IS:
      case TokenKind::KW_NEW:
      case TokenKind::KW_STATIC:
      case TokenKind::KW_SUPER:
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

struct Local {
  str_t name{}; // the name of the local variable

  // the depth in the scope chain that this variable was declared at. Zero
  // is the outermost scope-parameters for a method, or the first local block
  // in top level code. one is the scope within that, etc.
  int depth{};

  // true if thie local variable is being used as an upvalue
  bool is_upvalue{};

  Local(const str_t& n = str_t(), int d = -1, bool up = false) noexcept
    : name(n), depth(d), is_upvalue(up) {
  }
};

struct CompilerUpvalue {
  // true if this upvalue is capturing a local variable from the
  // enclosing function. false if it's capturing an upvalue
  bool is_local{};

  // index of the local or upvalue being captured in the enclosing
  // function
  int index{};

  CompilerUpvalue(bool local = false, int idx = 0) noexcept
    : is_local(local), index(idx) {
  }
};

class Compiler : private UnCopyable {
  // the maximum number of arguments that can be passed to a method, note
  // that this limtation is hardcoded in other places in th VM, in particular
  // the `CALL_xx` instructions assume a certain maximum number
  static constexpr int kMaxArguments = 16;

  // the maximum number of local variables that can be declared in a single
  // function, method, or check of top level code. this is the maximum number
  // of variables in scope at one time, and spans block scopes.
  //
  // note that this limitation is also explicit in the bytecode, since [LOAD_LOCAL]
  // and [STORE_LOCAL] use a single argument byte to identify the local, only
  // 256 can be in scope at one time.
  static constexpr int kMaxLocals = 256;

  // the maximum number of distinct constants that a function can contain. this
  // value is explicit in the bytecode since `CONSTANT` only takes a single
  // argument
  static constexpr int kMaxConstants = 256;

  Parser& parser_;
  Compiler* parent_{};
  FunctionObject* fn_{};

  // for the fields of the nearest enclosing class, or nullptr if not
  // currently inside a class
  SymbolTable* fields_{};

  // the current level of block scope nesting, where zero is no nesting.
  // a -1 here means top-level code is being compiled and thers is no block
  // scope in effect at all. any variables declared will be global
  int scope_depth_{-1};

  // index of the first instruction of the body of the innermost loop
  // currently being compiled. will be -1 if not currently inside a loop.
  int loop_body_{-1};

  // name of the method this compiler is compiling, or `empty` if this
  // compiler is not for a method. note that this is just the bare method
  // name and not its full signature
  str_t method_name_;

  // the currently in scope local variables
  std::vector<Local> locals_;

  // the upvalues that this function has captured from outer scopes, the
  // count of them is stored in `fn->num_upvalues()`
  std::vector<CompilerUpvalue> upvalues_;

  inline SymbolTable& vm_methods(void) { return parser_.get_vm().methods(); }
  inline SymbolTable& vm_gsymbols(void) { return parser_.get_vm().gsymbols(); }

  void error(const char* format, ...) {
    const Token& tok = parser_.prev();
    parser_.set_error(true);
    std::cerr << "[LINE:" << tok.lineno() << "] - "
      << "Compile ERROR on ";
    if (tok.kind() == TokenKind::TK_NL)
      std::cerr << "`newline` : ";
    else
      std::cerr << "`" << tok.literal() << "` : ";

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

  template <typename T, typename U> inline int emit_bytes(T b1, U b2) {
    emit_byte(b1);
    return emit_byte(b2);
  }

  inline void patch_jump(int offset) {
    // replaces the placeholder argument for a previous JUMP or JUMP_IF
    // instruction with an offset that jumps to the current end of bytecode

    fn_->set_code(offset, fn_->codes_count() - offset - 1);
  }

  inline int add_constant(const Value& v) {
    // see if an equivalent constant has already been added
    for (int i = 0; i < fn_->constants_count(); ++i) {
      if (fn_->get_constant(i) == v)
        return i;
    }

    if (fn_->constants_count() < kMaxConstants)
      fn_->add_constant(v);
    else
      error("a function may only contain %d unique constants", kMaxConstants);

    return fn_->constants_count() - 1;
  }

  inline void emit_constant(const Value& v) {
    int b = add_constant(v);
    emit_bytes(Code::CONSTANT, b);
  }

  void push_scope(void) {
    // starts a new local block scope

    ++scope_depth_;
  }

  void pop_scope(void) {
    // closes the last pushed block scope. this should only be called in
    // a statement context where no temporaries are still on the stack

    ASSERT(scope_depth_ > -1, "cannot pop top-level scope");

    while (!locals_.empty() && locals_.back().depth == scope_depth_) {
      bool is_upvalue = locals_.back().is_upvalue;
      locals_.pop_back();

      // if the local was closed over, make sure the upvalue gets closed
      // when it goes out of scope on the stack
      if (is_upvalue)
        emit_byte(Code::CLOSE_UPVALUE);
      else
        emit_byte(Code::POP);
    }
    --scope_depth_;
  }

  const GrammerRule& get_rule(TokenKind kind) const {
#define RULE(fn) [](Compiler* c, bool b) { c->fn(b); }
#define SIGN(fn) [](Compiler* c, str_t& n) { c->fn(n); }

#define UNUSED {nullptr, nullptr, nullptr, Precedence::NONE, nullptr}
#define PREFIX(fn) {RULE(fn), nullptr, nullptr, Precedence::NONE, nullptr}
#define INFIX(fn, prec) {nullptr, RULE(fn), nullptr, prec, nullptr}
#define INFIXOP(prec, name) {nullptr, RULE(infix_oper), SIGN(infix_signature), prec, name}
#define OPER(prec, name) {RULE(unary_oper), RULE(infix_oper), SIGN(mixed_signature), prec, name}
#define OPER2(pfn, ifn, prec) {RULE(pfn), RULE(ifn), nullptr, prec, nullptr}
#define PREFIXOP(name) {RULE(unary_oper), nullptr, SIGN(unary_signature), Precedence::NONE, name}
#define OP(name) {RULE(unary_oper), RULE(infix_oper), SIGN(mixed_signature), Precedence::TERM, name}
#define PREFIXNAME {RULE(variable), nullptr, SIGN(named_signature), Precedence::NONE, nullptr}
#define NEWOP(fn) {RULE(fn), nullptr, SIGN(ctor_signature), Precedence::NONE, nullptr}
    static const GrammerRule _rules[] = {
      PREFIX(grouping),                         // PUNCTUATOR(LPAREN, "(")
      UNUSED,                                   // PUNCTUATOR(RPAREN, ")")
      OPER2(list, subscript, Precedence::CALL), // PUNCTUATOR(LBRACKET, "[")
      UNUSED,                                   // PUNCTUATOR(RBRACKET, "]")
      UNUSED,                                   // PUNCTUATOR(LBRACE, "{")
      UNUSED,                                   // PUNCTUATOR(RBRACE, "}")
      UNUSED,                                   // PUNCTUATOR(COLON, ":")
      INFIX(call, Precedence::CALL),            // PUNCTUATOR(DOT, ".")
      UNUSED,                                   // PUNCTUATOR(COMMA, ",")
      INFIXOP(Precedence::FACTOR, "* "),        // PUNCTUATOR(STAR, "*")
      INFIXOP(Precedence::FACTOR, "/ "),        // PUNCTUATOR(SLASH, "/")
      INFIXOP(Precedence::TERM, "% "),          // PUNCTUATOR(PERCENT, "%")
      INFIXOP(Precedence::TERM, "+ "),          // PUNCTUATOR(PLUS, "+")
      OPER(Precedence::TERM, "- "),             // PUNCTUATOR(MINUS, "-")
      UNUSED,                                   // PUNCTUATOR(PIPE, "|")
      INFIX(or_expr, Precedence::LOGIC),        // PUNCTUATOR(PIPEPIPE, "||")
      UNUSED,                                   // PUNCTUATOR(AMP, "&")
      INFIX(and_expr, Precedence::LOGIC),       // PUNCTUATOR(AMPAMP, "&&")
      PREFIXOP("!"),                            // PUNCTUATOR(BANG, "!")
      PREFIXOP("~"),                            // PUNCTUATOR(TILDE, "~")
      UNUSED,                                   // PUNCTUATOR(EQ, "=")
      INFIXOP(Precedence::COMPARISON, "< "),    // PUNCTUATOR(LT, "<")
      INFIXOP(Precedence::COMPARISON, "> "),    // PUNCTUATOR(GT, ">")
      INFIXOP(Precedence::COMPARISON, "<= "),   // PUNCTUATOR(LTEQ, "<=")
      INFIXOP(Precedence::COMPARISON, ">= "),   // PUNCTUATOR(GTEQ, ">=")
      INFIXOP(Precedence::EQUALITY, "== "),     // PUNCTUATOR(EQEQ, "==")
      INFIXOP(Precedence::EQUALITY, "!= "),     // PUNCTUATOR(BANGEQ, "!=")

      UNUSED,                                   // KEYWORD(BREAK, "break")
      UNUSED,                                   // KEYWORD(CLASS, "class")
      UNUSED,                                   // KEYWORD(ELSE, "else")
      PREFIX(boolean),                          // KEYWORD(FALSE, "false")
      PREFIX(function),                         // KEYWORD(FN, "fn")
      UNUSED,                                   // KEYWORD(FOR, "for")
      UNUSED,                                   // KEYWORD(IF, "if")
      UNUSED,                                   // KEYWORD(IN, "in")
      INFIX(is, Precedence::IS),                // KEYWORD(IS, "is")
      NEWOP(new_exp),                           // KEYWORD(NEW, "new")
      PREFIX(nil),                              // KEYWORD(NIL, "nil")
      UNUSED,                                   // KEYWORD(RETURN, "return")
      UNUSED,                                   // KEYWORD(STATIC, "static")
      PREFIX(super_exp),                        // KEYWORD(SUPER, "super")
      PREFIX(this_exp),                         // KEYWORD(THIS, "this")
      PREFIX(boolean),                          // KEYWORD(TRUE, "true")
      UNUSED,                                   // KEYWORD(VAR, "var")
      UNUSED,                                   // KEYWORD(WHILE, "while")

      PREFIX(field),                            // TOKEN(FIELD, "field")
      PREFIXNAME,                               // TOKEN(IDENTIFIER, "identifier")
      PREFIX(numeric),                          // TOKEN(NUMERIC, "numeric")
      PREFIX(string),                           // TOKEN(STRING, "string")

      UNUSED,                                   // TOKEN(NL, "new-line")

      UNUSED,                                   // TOKEN(ERROR, "error")
      UNUSED,                                   // TOKEN(EOF, "eof")
    };
#undef SIGN
#undef RULE

    return _rules[Xt::as_type<int>(kind)];
  }

  void parse_precedence(bool allow_assignment, Precedence precedence) {
    parser_.advance();
    auto& prefix_fn = get_rule(parser_.prev().kind()).prefix;
    if (!prefix_fn) {
      error("unexpected token for expression");
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

  int define_local(const str_t& name) {
    // create a new local variable with [name], assumes the current scope
    // is local and the name is unique

    locals_.push_back(Local(name, scope_depth_, false));
    return Xt::as_type<int>(locals_.size() - 1);
  }

  int declare_variable(void) {
    consume(TokenKind::TK_IDENTIFIER, "expected variable name");

    str_t name = parser_.prev().as_string();
    // top-level global scope
    if (scope_depth_ == -1) {
      int symbol = vm_gsymbols().add(name);
      if (symbol == -1)
        error("global variable is already defined");
      return symbol;
    }

    // see if there is already a variable with this name declared in the
    // current scope (outer scopes are OK: those get shadowed)
    int i = Xt::as_type<int>(locals_.size()) - 1;
    for (; i >= 0; --i) {
      auto& local = locals_[i];

      // once we escape this scope and hit an outer one, we can stop
      if (local.depth < scope_depth_)
        break;
      if (local.name == name) {
        error("variable is already declared in this scope");
        return i;
      }
    }

    if (locals_.size() >= kMaxLocals) {
      error("cannot declare more than %d variables in one scope", kMaxLocals);
      return -1;
    }

    return define_local(name);
  }

  void define_variable(int symbol) {
    // store the variable if it's a local, the result of the initializer
    // is in the correct slot on the stack already so we are done.
    if (scope_depth_ >= 0)
      return;

    // it's a global variable, so store the value int the global slot and
    // then discard the temporary for the initializer
    emit_bytes(Code::STORE_GLOBAL, symbol);
    emit_byte(Code::POP);
  }

  int resolve_local(const str_t& name) {
    // attempts to look up the name in the local variables of [compiler],
    // if found returns its index, otherwise returns -1

    // look it up in the local scopes. look in reverse order so that the
    // most nested variable is found first and shadows outer ones
    int i = Xt::as_type<int>(locals_.size()) - 1;
    for (; i >= 0; --i) {
      if (locals_[i].name == name)
        return i;
    }
    return -1;
  }

  int add_upvalue(bool is_local, int index) {
    // adds an upvalue to [compiler]'s function with the given properties,
    // does not add one if an upvalue for that variable is already in the
    // list. returns the index of the upvalue

    // look for an existing one
    for (int i = 0; i < fn_->num_upvalues(); ++i) {
      auto& upvalue = upvalues_[i];
      if (upvalue.is_local == is_local && upvalue.index == index)
        return i;
    }

    // if got here, need add a new upvalue
    upvalues_.push_back(CompilerUpvalue(is_local, index));
    return fn_->inc_num_upvalues();
  }

  int find_upvalue(const str_t& name) {
    // attempts to look up [name] in the functions enclosing the one being
    // compiled by [compiler]. if found, it adds an upvalue for it to this
    // compiler's list of upvalues and returns its index, if not found,
    // returns -1
    //
    // if the name is found outside of the immediately enclosing function
    // this will flatten the closure and add upvalues to all of the
    // intermediate functions so that it gets walked down to this one.

    // if out of enclosing functions, it cannot be an upvalue
    if (parent_ == nullptr)
      return -1;

    // if it's a local variable in the immediately enclosing function
    int local = parent_->resolve_local(name);
    if (local != -1) {
      // mark the local as an upvalue so we know to clsoe it when it
      // goes out of scope
      parent_->locals_[local].is_upvalue = true;
      return add_upvalue(true, local);
    }

    // if it's an upvalue in the immediately enclosing function, in outer
    // words, if its a local variable in a non-immediately enclosing
    // funtion. this will `flatten` closures automatically: it will add
    // upvalues to all of the immediate functions to get from the function
    // where a local is declared all the way into the possibly deeply
    // nested function that is closing over it.
    int upvalue = parent_->find_upvalue(name);
    if (upvalue != -1)
      return add_upvalue(false, upvalue);

    // if got here, walked all the way up the parent chain and could not
    // find it
    return -1;
  }

  int resolve_name(const str_t& name, Code& load_instruction) {
    // loop up [name] in the current scope to see what name it is bound
    // to returns the index of the name either in global or local scope,
    // or the enclosing function's upvalue list. returns -1 if not found.
    //
    // sets [load_instruction] to the instruction needed to load the
    // variable. will be one of [LOAD_LOCAL], [LOAD_UPVALUE] or [LOAD_GLOBAL]

    // look it up in the local scope, look in reverse order so that the
    // most nested variable is found first and shadows outer ones
    load_instruction = Code::LOAD_LOCAL;
    int local = resolve_local(name);
    if (local != -1)
      return local;

    // if got here, it's not a local, so lets see if we are closing over
    // an outer local
    load_instruction = Code::LOAD_UPVALUE;
    int upvalue = find_upvalue(name);
    if (upvalue != -1)
      return upvalue;

    // if got here, it was not in a local scope, so try the global scope
    load_instruction = Code::LOAD_GLOBAL;
    return vm_gsymbols().get(name);
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

  void list(bool allow_assignment) {
    // compile the list elements
    int num_elements = 0;
    if (parser_.curr().kind() != TokenKind::TK_RBRACKET) {
      do {
        ++num_elements;
        expression();

        // ignore a newline after the element but before the `,` or `]`
        match(TokenKind::TK_NL);
      } while (match(TokenKind::TK_COMMA));
    }
    consume(TokenKind::TK_RBRACKET, "expect `]` after list elements");

    // create the list
    emit_bytes(Code::LIST, num_elements);
  }

  void load_this(void) {
    // loads the receiver of the currently enclosing method. correctly
    // handles functions defined inside methods

    Code load_instruction;
    int index = resolve_name("this", load_instruction);
    emit_bytes(load_instruction, index);
  }

  void variable(bool allow_assignment) {
    // look up the name in the scope chain
    const auto& token = parser_.prev();

    Code load_instruction;
    int index = resolve_name(token.as_string(), load_instruction);
    if (index == -1)
      error("undefined variable");

    // if there's an `=` after a bare name, it's a variable assignment
    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      // compile the right-hand side
      expression();

      // emit the store instruction
      switch (load_instruction) {
      case Code::LOAD_LOCAL: emit_bytes(Code::STORE_LOCAL, index); break;
      case Code::LOAD_UPVALUE: emit_bytes(Code::STORE_UPVALUE, index); break;
      case Code::LOAD_GLOBAL: emit_bytes(Code::STORE_GLOBAL, index); break;
      default: UNREACHABLE();
      }
    }
    else {
      emit_bytes(load_instruction, index);
    }
  }

  void field(bool allow_assignment) {
    int field;
    if (fields_ != nullptr) {
      // look up the field, or implicitlt define it
      field = fields_->ensure(parser_.prev().as_string());
    }
    else {
      error("cannot reference a field outside of a class definition");
      // initialize it with a fake value so we can keep parsing and minimize
      // the number of cascaded erros
      field = 0xff;
    }

    // if there is an `=` after a filed name, it's an assignment
    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      // compile the right-hand side
      expression();

      // if we are directly inside a method, use a more optional instruction
      if (!method_name_.empty()) {
        emit_bytes(Code::STORE_FIELD_THIS, field);
      }
      else {
        load_this();
        emit_bytes(Code::STORE_FIELD, field);
      }
    }
    else {
      // if we are directly inside a method, use a more optional instruction
      if (!method_name_.empty()) {
        emit_bytes(Code::LOAD_FIELD_THIS, field);
      }
      else {
        load_this();
        emit_bytes(Code::LOAD_FIELD, field);
      }
    }
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

  int start_loop_body(void) {
    int outer_loop_body = loop_body_;
    loop_body_ = fn_->codes_count();
    return outer_loop_body;
  }

  void finish_loop_body(int outer_loop_body) {
    // find any break placeholder instructions (which will be Code::END in the
    // bytecode) and replace them with real jumps
    int i = loop_body_;
    while (i < fn_->codes_count()) {
      if (Xt::as_type<Code>(fn_->get_code(i)) == Code::END) {
        fn_->set_code(i, Code::JUMP);
        patch_jump(i + 1);
        i += 2;
      }
      else {
        // skip this instruction and its arguments
        i += 1 + fn_->get_argc(i);
      }
    }
    loop_body_ = outer_loop_body;
  }

  void class_definition(void) {
    // compiles a class definition, assumes the `class` token has already
    // been consumed.

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

    // store a placeholder for the number of fields argument, we donot
    // know the value untial we have compiled all the methods to see
    // which fields are used.
    int num_fields_instruction = emit_byte(0xff);

    // set up a symbol table for the class's fields, we'll initially compile
    // them to slots starting at zero. when the method is bound to the close
    // the bytecode will be adjusted by [bind_method] to take inherited fields
    // into account.
    SymbolTable* prev_fields = fields_;
    SymbolTable fileds;
    fields_ = &fileds;

    consume(TokenKind::TK_LBRACE, "expect `{` before class body");
    while (!match(TokenKind::TK_RBRACE)) {
      Code instruction = Code::METHOD_INSTANCE;
      bool is_ctor = false;

      if (match(TokenKind::KW_STATIC)) {
        instruction = Code::METHOD_STATIC;
      }
      else if (parser_.curr().kind() == TokenKind::KW_NEW) {
        // if the method name is `new` it's a named constructor
        is_ctor = true;
      }

      auto& signature = get_rule(parser_.curr().kind()).method;
      parser_.advance();

      if (!signature) {
        error("expect method definition");
        break;
      }
      method(instruction, is_ctor, signature);
      consume(TokenKind::TK_NL, "expect newline after definition in class");
    }

    // update the class with the number of fields
    fn_->set_code(num_fields_instruction, fileds.count());
    fields_ = prev_fields;

    // store it in its name
    define_variable(symbol);
  }

  void for_stmt(void) {
    // A for statement like:
    //
    //     for (i in sequence.expression) {
    //       IO.write(i)
    //     }
    //
    // Is compiled to bytecode almost as if the source looked like this:
    //
    //     {
    //       var seq_ = sequence.expression
    //       var iter_
    //       while (true) {
    //         iter_ = seq_.iterate(iter_)
    //         if (!iter_) break
    //         var i = set_.iteratorValue(iter_)
    //         IO.write(i)
    //       }
    //     }
    //
    // It's not exactly this, because the synthetic variables `seq_` and `iter_`
    // actually get names that aren't valid Wren identfiers. Also, the `while`
    // and `break` are just the bytecode for explicit loops and jumps. But that's
    // the basic idea.
    //
    // The important parts are:
    // - The sequence expression is only evaluated once.
    // - The .iterate() method is used to advance the iterator and determine if
    //   it should exit the loop.
    // - The .iteratorValue() method is used to get the value at the current
    //   iterator position.

    // Create a scope for the hidden local variables used for the iterator.

    push_scope();

    consume(TokenKind::TK_LPAREN, "expect `(` after keyword `for`");
    consume(TokenKind::TK_IDENTIFIER, "expect for loop variable name");

    // remeber the name of the loop variable
    str_t name = parser_.prev().as_string();

    consume(TokenKind::KW_IN, "expect `in` after for loop variable");

    // evaluate the sequence expression and store it in a hidden local variable
    // the scope in the variable name ensures it won't collide with a user-defined
    // variable
    expression();
    int seq_slot = define_local("seq ");

    // create another hidden local for the iterator object
    nil(false);
    int iter_slot = define_local("iter ");

    consume(TokenKind::TK_RPAREN, "expect `)` after loop expression");

    // remeber what instrution to loop back to
    int loop_start = fn_->codes_count() - 1;

    // advance the iterator by calling the `.iterate` method on the sequence
    emit_bytes(Code::LOAD_LOCAL, seq_slot);
    emit_bytes(Code::LOAD_LOCAL, iter_slot);

    int iterate_symbol = vm_methods().ensure("iterate ");
    emit_bytes(Code::CALL_1, iterate_symbol);

    // store the iterator back in its local for the next iteration
    emit_bytes(Code::STORE_LOCAL, iter_slot);

    // if it returned something falsely, jump out of the loop
    int exit_jump = emit_bytes(Code::JUMP_IF, 0xff);

    // create a scope for the loop body
    push_scope();

    // get the current value in the sequence by calling `.iterValue`
    emit_bytes(Code::LOAD_LOCAL, seq_slot);
    emit_bytes(Code::LOAD_LOCAL, iter_slot);

    int iter_value_symbol = vm_methods().ensure("iterValue ");
    emit_bytes(Code::CALL_1, iter_value_symbol);

    // bind it to the loop variable
    define_local(name);

    // compile the body
    int outer_loop_body = start_loop_body();
    block();

    pop_scope();

    // loop back to the top
    emit_byte(Code::LOOP);
    int loop_offset = fn_->codes_count() - loop_start;
    emit_byte(loop_offset);

    patch_jump(exit_jump);
    finish_loop_body(outer_loop_body);

    pop_scope();
  }

  void while_stmt(void) {
    // remeber what instrution to loop back to
    int loop_start = fn_->codes_count() - 1;

    // compile the condition
    consume(TokenKind::TK_LPAREN, "expect `(` after `while`");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

    int exit_jump = emit_bytes(Code::JUMP_IF, 0xff);

    // compile the body
    int outer_loop_body = start_loop_body();
    block();

    // loop back to the top
    emit_byte(Code::LOOP);
    int loop_offset = fn_->codes_count() - loop_start;
    emit_byte(loop_offset);

    patch_jump(exit_jump);
    finish_loop_body(outer_loop_body);
  }

  void var_definition(void) {
    int symbol = declare_variable();
    // compile the initializer
    if (match(TokenKind::TK_EQ)) {
      expression();
    }
    else {
      // default initialize it to nil
      nil(false);
    }
    define_variable(symbol);
  }

  void definition(void) {
    // compiles a `definition`, there are the statements that bind new
    // variables, they can only appear at the top level of a block and are
    // prohibited in places like the non-curly body of an if or while

    if (match(TokenKind::KW_CLASS)) {
      class_definition();
      return;
    }

    if (match(TokenKind::KW_VAR)) {
      var_definition();
      return;
    }

    block();
  }

  void statement(void) {
    // compiles a statement, there can only appear at the top-level or within
    // curly blocks, unlike expressions, these do not leave a value on the stack

    if (match(TokenKind::KW_BREAK)) {
      if (loop_body_ == -1)
        error("cannot use `break` outside of a loop");

      // emit a placeholder instruction for the jump to the end of the body,
      // when we are done compiling the loop body and know where the end is.
      // we will replace these with `JUMP` instructions with appropriate offsets
      // we use `END` here because that cannot occur in the middle of bytecode
      emit_bytes(Code::END, 0);
      return;
    }

    if (match(TokenKind::KW_FOR)) {
      for_stmt();
      return;
    }

    if (match(TokenKind::KW_IF)) {
      // compile the condition
      consume(TokenKind::TK_LPAREN, "expect `(` after `if` keyword");
      expression();
      consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

      // jump to the else branch if the condition is false
      int if_jump = emit_bytes(Code::JUMP_IF, 0xff);

      // compile the then branch
      block();

      if (match(TokenKind::KW_ELSE)) {
        // jump over the else branch when the if branch is taken
        int else_jump = emit_bytes(Code::JUMP, 0xff);

        patch_jump(if_jump);
        block();

        // patch the jump over the else
        patch_jump(else_jump);
      }
      else {
        patch_jump(if_jump);
      }
      return;
    }

    if (match(TokenKind::KW_RETURN)) {
      expression();
      emit_byte(Code::RETURN);
      return;
    }

    if (match(TokenKind::KW_WHILE)) {
      while_stmt();
      return;
    }

    // expression statement
    expression();
    emit_byte(Code::POP);
  }

  void expression(void) {
    parse_precedence(true, Precedence::LOWEST);
  }

  void grouping(bool allow_assignment) {
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
  }

  void block(void) {
    // parses a curly block or an expression statement, used in places like
    // the arms of an if statement where either a single expression or a
    // curly body is allowd

    if (match(TokenKind::TK_LBRACE)) {
      push_scope();
      finish_block();
      pop_scope();
      return;
    }

    // single statement body
    statement();
  }

  void finish_block(void) {
    // parses a block body, after the initial `{` has been consumed

    for (;;) {
      definition();

      if (!match(TokenKind::TK_NL)) {
        consume(TokenKind::TK_RBRACE, "expect `}` after block body");
        break;
      }
      if (match(TokenKind::TK_RBRACE))
        break;
    }
  }

  void function(bool allow_assignment) {
    Compiler fn_compiler(parser_, this, "");
    int fn_constant = fn_compiler.init_compiler();

    str_t dummy_name;
    fn_compiler.parameters(dummy_name);

    if (fn_compiler.match(TokenKind::TK_LBRACE)) {
      fn_compiler.finish_block();

      // implicitly return nil
      fn_compiler.emit_bytes(Code::NIL, Code::RETURN);
    }
    else {
      fn_compiler.expression();
      fn_compiler.emit_byte(Code::RETURN);
    }

    fn_compiler.finish_compiler(fn_constant);
  }

  void method(Code instruction, bool is_ctor, const SignatureFn& signature) {
    // compiles a method definition inside a class body

    // build the method name, the mangled name includes all of the name parts
    // in a mixfix call as well as spaces for every argument.
    // so a method like:
    //    foo.bar(a, b) else (c) last
    //
    // will have name: "bar  else last"
    str_t name(parser_.prev().as_string());

    Compiler method_compiler(parser_, this, name);
    int method_constant = method_compiler.init_compiler();

    signature(&method_compiler, name);
    int symbol = vm_methods().ensure(name);

    consume(TokenKind::TK_LBRACE, "expect `{` to begin method body");

    method_compiler.finish_block();
    // if it's a constructor, return `this`
    if (is_ctor) {
      // the receiver is always stored in the first local slot
      method_compiler.emit_bytes(Code::LOAD_LOCAL, 0);
    }
    else {
      // implicitly return nil in case there is no explicit return
      method_compiler.emit_byte(Code::NIL);
    }
    method_compiler.emit_byte(Code::RETURN);
    method_compiler.finish_compiler(method_constant);

    // compile the code to define the method it
    emit_bytes(instruction, symbol);
  }

  void call(bool allow_assignment) {
    named_call(allow_assignment, Code::CALL_0);
  }

  void subscript(bool allow_assignment) {
    // subscript or `array indexing` operator like `foo[index]`

    int argc = 0;
    // build the method name, allow overloading by arity, add a space to
    // the name for each argument
    str_t name(1, '[');

    // parse the arguments list
    do {
      validate_num_parameters(++argc);
      expression();

      // add a space in the name for each argument, lets overload by arity
      name.push_back(' ');
    } while (match(TokenKind::TK_COMMA));
    consume(TokenKind::TK_RBRACKET, "expect `]` after subscript arguments");
    name.push_back(']');

    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      name.push_back('=');

      // compile the assigned value
      validate_num_parameters(++argc);
      expression();
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

    int jump = emit_bytes(Code::AND, 0xff);

    parse_precedence(false, Precedence::LOGIC);
    patch_jump(jump);
  }

  void or_expr(bool allow_assignment) {
    // skip the right argument if the left if true

    int jump = emit_bytes(Code::OR, 0xff);

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

  void named_signature(str_t& name) {
    // compiles a method signature for a named method or setter

    if (match(TokenKind::TK_EQ)) {
      name.push_back('=');
      name.push_back(' ');

      // parse the value parameter
      declare_variable();
    }
    else {
      // regular named method with an optional parameter list
      parameters(name);
    }
  }

  void ctor_signature(str_t& name) {
    // add the parameters if there are any
    parameters(name);
  }

  void validate_num_parameters(int argc) {
    // the VM can only handle a certain number of parameters, so check that
    // we havenot exceeded that and give a usable error

    if (argc >= kMaxArguments + 1) {
      // only show an error at exactly max + 1 so that we can keep parsing
      // the parameters and minimize cascaded errors
      error("methods cannot have more than %d parameters", kMaxArguments);
    }
  }

  void parameters(str_t& name) {
    // parse the parameter list, if any
    if (match(TokenKind::TK_LPAREN)) {
      int argc = 0;
      do {
        validate_num_parameters(++argc);

        // define a local variable in the method for the parameters
        declare_variable();

        // add a space in the name for the parameters
        name.push_back(' ');
      } while (match(TokenKind::TK_COMMA));
      consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
    }
  }

  void method_call(Code instruction, const str_t& method_name) {
    // compiles an (optional) argument list and then calls it

    // parse the argument list, if any
    int argc = 0;
    str_t name(method_name);
    if (match(TokenKind::TK_LPAREN)) {
      do {
        validate_num_parameters(++argc);
        expression();

        // add a space in the name for each argument, let's us overload by arity
        name.push_back(' ');
      } while (match(TokenKind::TK_COMMA));
      consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    }

    int symbol = vm_methods().ensure(name);
    emit_bytes(instruction + argc, symbol);
  }

  void named_call(bool allow_assignment, Code instruction) {
    // compiles the method name and argument list for a `<...>.name(...)` call

    // build the method name
    consume(TokenKind::TK_IDENTIFIER, "expect method name after `.`");
    str_t name(parser_.prev().as_string());

    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      name.push_back('=');
      name.push_back(' ');

      expression();

      int symbol = vm_methods().ensure(name);
      emit_bytes(instruction + 1, symbol);
    }
    else {
      method_call(instruction, name);
    }
  }

  bool is_inside_method(void) {
    // returns true if [compiler] is compiling a chunk of code that is either
    // directly or indirectly contained in a method for a class

    auto* c = this;
    // walk up the parent chain to see if there is an enclosing method
    while (c != nullptr) {
      if (!c->method_name_.empty())
        return true;
      c = c->parent_;
    }
    return false;
  }

  void new_exp(bool allow_assignment) {
    parse_precedence(false, Precedence::CALL);

    // create the instance of the class
    emit_byte(Code::NEW);

    method_call(Code::CALL_0, "new");
  }

  void super_exp(bool allow_assignment) {
    if (!is_inside_method())
      error("cannot use `super` outside of a method");

    load_this();
    // see if it's a nemd super call, or an unnamed one
    if (match(TokenKind::TK_DOT)) {
      // compile the superclass call
      named_call(allow_assignment, Code::SUPER_0);
    }
    else {
      str_t name;
      Compiler* this_compiler = this;
      while (this_compiler != nullptr) {
        if (!this_compiler->method_name_.empty()) {
          name = this_compiler->method_name_;
          break;
        }
        this_compiler = this_compiler->parent_;
      }

      // call the superclass method with the same name
      method_call(Code::SUPER_0, name);
    }
  }

  void this_exp(bool allow_assignment) {
    if (!is_inside_method()) {
      error("cannot use `this` outside of a method");
      return;
    }

    load_this();
  }
public:
  Compiler(Parser& parser,
      Compiler* parent = nullptr, const str_t& method_name = "") noexcept
    : parser_(parser)
    , parent_(parent)
    , method_name_(method_name) {
  }

  ~Compiler(void) {
    // FIXME: fixed deallocate FunctionObject by GC
  }

  int init_compiler(void) {
    if (parent_ == nullptr) {
      scope_depth_ = -1;
    }
    else {
      if (!method_name_.empty())
        locals_.push_back(Local("this"));
      else
        locals_.push_back(Local());
      scope_depth_ = 0;
    }

    if (parent_ != nullptr)
      fields_ = parent_->fields_;
    fn_ = FunctionObject::make_function(parser_.get_vm());

    if (parent_ == nullptr)
      return -1;

    // add the block to the constant table, do this eagerly to it's
    // reachable by the GC
    return parent_->add_constant(fn_);
  }

  void finish_compiler(int constant) {
    // finishes [compiler], which is compiling a function, method or chunk of
    // top level code. if there is a parent compiler, then this emits code in
    // the parent compiler to loadd the resulting function

    // mark the end of the bytecode, since it may contain multiple early
    // returns, we cannot rely on `RETURN` to tell use we are at the end.
    emit_byte(Code::END);

    // in the function that contains this one, load the resulting function
    // object.
    if (parent_ != nullptr) {
      // if the function has no upvalues, we donot need to create a closure.
      // we can just load and run the function directly
      if (fn_->num_upvalues() == 0) {
        parent_->emit_bytes(Code::CONSTANT, constant);
      }
      else {
        // capture the upvalues in the new closure object.
        parent_->emit_bytes(Code::CLOSURE, constant);

        // emit a arguments for each upvalue to know whether to capture a
        // local or an upvalue
        for (int i = 0; i < fn_->num_upvalues(); ++i) {
          auto& uv = upvalues_[i];
          parent_->emit_bytes(uv.is_local ? 1 : 0, uv.index);
        }
      }
    }
  }

  FunctionObject* compile_function(TokenKind end_kind) {
    Pinned pinned;
    parser_.get_vm().pin_object(fn_, &pinned);
    for (;;) {
      definition();

      // if there is no newline, it must be the end of the block on the same line
      if (!match(TokenKind::TK_NL)) {
        consume(end_kind, "expect end of file");
        break;
      }
      if (match(end_kind))
        break;
    }
    emit_bytes(Code::NIL, Code::RETURN);
    finish_compiler(-1);

    parser_.get_vm().unpin_object();
    return parser_.had_error() ? nullptr : fn_;
  }

  FunctionObject* compile_function(
      Parser& p, Compiler* parent, TokenKind end_kind) {
    Compiler c(p, parent, "");
    return c.compile_function(end_kind);
  }
};

FunctionObject* compile(WrenVM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Parser p(vm, lex);

  p.advance();
  Compiler c(p, nullptr, "");
  c.init_compiler();

  return c.compile_function(TokenKind::TK_EOF);
}

}
