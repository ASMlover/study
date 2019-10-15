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

enum class Precedence {
  NONE,
  LOWEST,

  ASSIGNMENT, // =
  TERNARY,    // ?:
  LOGIC_OR,   // ||
  LOGIC_AND,  // &&
  EQUALITY,   // == !=
  IS,         // is
  COMPARISON, // < <= > >=
  BIT_OR,     // |
  BIT_XOR,    // ^
  BIT_AND,    // &
  BIT_SHIFT,  // << >>
  RANGE,      // .. ...
  TERM,       // + -
  FACTOR,     // * / %
  UNARY,      // unary - ! ~
  CALL,       // () []
  PRIMARY,    //
};

inline Precedence operator+(Precedence a, int b) {
  return Xt::as_type<Precedence>(Xt::as_type<int>(a) + b);
}

// the different signature syntaxes for different kinds of methods
enum class SignatureType {
  // a name followed by a (possibly empty) parenthesized parameter list, also
  // used for binary operators
  METHOD,

  GETTER,           // just a name, also used for unary operators
  SETTER,           // a name followed by `=`
  SUBSCRIPT,        // a square bracketed parameter list
  SUBSCRIPT_SETTER, // a square bracketed parameter list followed by `=`

  // a constructor initializer function, this has a distinct signature to
  // prevent it from being invoked directly outside of the constructor on
  // the metaclass
  INITIALIZER,
};

struct Signature {
  str_t name{};
  SignatureType type{};
  int arity{};

  Signature(void) noexcept {}
  Signature(const str_t& _name, SignatureType _type, int _arity = 0) noexcept
    : name(_name), type(_type), arity(_arity) {
  }

  inline void set_signature(
      const str_t& _name, SignatureType _type, int _arity = 0) {
    name = _name;
    type = _type;
    arity = _arity;
  }

  inline void set_name(const str_t& s) { name = s; }
  inline void set_arity(int n) { arity = n; }
  inline int inc_arity(void) { return ++arity; }
  inline void set_type(SignatureType t) { type = t; }
};

using GrammerFn = std::function<void (Compiler*, bool)>;
using SignatureFn = std::function<void (Compiler*, Signature&)>;


struct GrammerRule {
  GrammerFn prefix;
  GrammerFn infix;
  SignatureFn method;
  Precedence precedence;
  const char* name;
};

class Parser : private UnCopyable {
  WrenVM& vm_;
  ModuleObject* module_{};

  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool skip_newlines_{true};
  bool print_errors_{false};

  bool had_error_{};
public:
  Parser(WrenVM& vm, ModuleObject* module, Lexer& lex, bool print_errors) noexcept
    : vm_(vm), module_(module), lex_(lex), print_errors_(print_errors) {
  }

  inline const str_t& source_path(void) const { return lex_.source_path(); }
  inline const Token& prev(void) const { return prev_; }
  inline const Token& curr(void) const { return curr_; }
  inline bool had_error(void) const { return had_error_; }
  inline void set_error(bool b = true) { had_error_ = b; }
  inline bool print_errors(void) const { return print_errors_; }
  inline void set_print_errors(bool b = true) { print_errors_ = b; }
  inline WrenVM& get_vm(void) { return vm_; }
  inline ModuleObject* get_mod(void) const { return module_; }

  void advance(void) {
    prev_ = curr_;

    // if we are out of tokens, donot try to tokenize any more we *do*
    // skill copy the TokenKind::TK_EOF to previois so that so that code
    // that expects it to be consumed will still work
    if (curr_.kind() == TokenKind::TK_EOF)
      return;

    curr_ = lex_.next_token();
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

struct Loop {
  // index of the instruction that the loop should jump back to
  int start{};

  // index of the argument for the `Code::JUMP_IF` instruction use to exit
  // the loop, stored so we can patch it once we know where the loop ends
  int exit_jump{};

  // index of the first instruction of the body of the loop
  int body{};

  // depth of the scope(s) that need to be exited if a break is hit inside
  // the loop
  int scope_depth{};

  // the loop enclosing this one or `nullptr` of this is the outermost loop
  Loop* enclosing{};
};

struct ClassCompiler {
  // bookkeeping information for compiling a class definition

  // symbol table for the fields of the class
  SymbolTable* fields{};

  // true if the class being compiled is a foreign class
  bool is_foreign{};

  // true if the current method being compiled is static
  bool in_static{};

  // the signature of the method being compiled
  Signature* signature{};

  ClassCompiler(void) noexcept {}
  ClassCompiler(SymbolTable* arg_fields,
      bool arg_foreign, bool arg_static, Signature* arg_signature) noexcept
    : fields(arg_fields)
    , is_foreign(arg_foreign)
    , in_static(arg_static)
    , signature(arg_signature) {
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
  static constexpr int kMaxConstants = 1 << 16;

  Parser& parser_;
  Compiler* parent_{};

  // the growable buffer of code that's been compiled so far
  std::vector<u8_t> bytecode_;
  // the buffer of source line mappings
  std::vector<int> debug_source_lines_;
  std::vector<Value> constants_;

  // the current level of block scope nesting, where zero is no nesting.
  // a -1 here means top-level code is being compiled and thers is no block
  // scope in effect at all. any variables declared will be module-level
  int scope_depth_{-1};

  // the current innermost loop being compiled, or `nullptr` if not in a loop
  Loop* loop_{};

  // if this is a compiler for a method, keeps track of the class enclosing it
  ClassCompiler* enclosing_class_{};

  // // for the fields of the nearest enclosing class, or nullptr if not
  // // currently inside a class
  // SymbolTable* fields_{};

  // // true if the current method being compiled is static, false if it's an
  // // instance method or we are not compiling a method
  // bool is_static_method_{};

  // // name of the method this compiler is compiling, or `empty` if this
  // // compiler is not for a method. note that this is just the bare method
  // // name and not its full signature
  // str_t method_name_;

  // the currently in scope local variables
  std::vector<Local> locals_;

  // the upvalues that this function has captured from outer scopes, the
  // count of them is stored in `fn->num_upvalues()`
  std::vector<CompilerUpvalue> upvalues_;
  int num_upvalues_{};

  // the number of parameters this method or function expects
  int num_params_{};

  inline SymbolTable& vm_mnames(void) { return parser_.get_vm().mnames(); }

  void error(const char* format, ...) {
    const Token& tok = parser_.prev();
    parser_.set_error(true);
    if (!parser_.print_errors())
      return;

    // if the parse error was caused by an error token, the lexer has already
    // reported it.
    if (tok.kind() == TokenKind::TK_ERROR)
      return;

    std::cerr
      << "[`" << parser_.source_path() << "` LINE:" << tok.lineno() << "] - "
      << "ERROR at ";
    if (tok.kind() == TokenKind::TK_NL)
      std::cerr << "`newline` : ";
    else if (tok.kind() == TokenKind::TK_EOF)
      std::cerr << "`end of file` : ";
    else
      std::cerr << "`" << tok.literal() << "` : ";

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    std::cerr << std::endl;
  }

  int intern_symbol(void) {
    return vm_mnames().ensure(parser_.prev().as_string());
  }

  template <typename T> inline int emit_byte(T b) {
    bytecode_.push_back(Xt::as_type<u8_t>(b));
    debug_source_lines_.push_back(parser_.prev().lineno());
    return Xt::as_type<int>(bytecode_.size() - 1);
  }

  template <typename T> inline void emit_u16(T arg) {
    emit_byte((arg >> 8) & 0xff);
    emit_byte(arg & 0xff);
  }

  template <typename T, typename U> inline int emit_bytes(T b1, U b2) {
    emit_byte(b1);
    return emit_byte(b2);
  }

  template <typename T> inline void emit_words(T b, u16_t arg) {
    // emits one bytecode instruction followed by a 16-bit argument,
    // which will be written big endian

    emit_byte(b);
    emit_u16(arg);
  }

  template <typename T> inline int emit_jump(T instruction) {
    // emits [instruction] followed by a placeholder for a jump offset, the
    // placeholder can be patched by calling [patch_jump], returns the index
    // of the placeholder

    emit_byte(instruction);
    return emit_bytes(0xff, 0xff) - 1;
  }

  inline void patch_jump(int offset) {
    // replaces the placeholder argument for a previous JUMP or JUMP_IF
    // instruction with an offset that jumps to the current end of bytecode

    // -2 to adjust for the bytecode for the jump offset iteself
    int jump = Xt::as_type<int>(bytecode_.size()) - offset - 2;

    bytecode_[offset] = (jump >> 8) & 0xff;
    bytecode_[offset + 1] = jump & 0xff;
  }

  inline int add_constant(const Value& v) {
    if (constants_.size() < kMaxConstants)
      constants_.push_back(v);
    else
      error("a function may only contain %d unique constants", kMaxConstants);

    return Xt::as_type<int>(constants_.size() - 1);
  }

  inline void emit_constant(const Value& v) {
    int b = add_constant(v);
    emit_words(Code::CONSTANT, b);
  }

  int discard_locals(int depth) {
    // generates code to discard local variables at [depth] or greater, does
    // not actually undeclare variables or pop any scopes, though. this is
    // called directly when compiling `break` statements to ditch the local
    // variables before jumping out of the loop even though they are still in
    // scope *past* the break instruction
    //
    // returns the number of local variables that were eliminated.

    ASSERT(scope_depth_ > -1, "cannot exit top-level scope");

    int local = Xt::as_type<int>(locals_.size()) - 1;
    while (local >= 0 && locals_[local].depth >= depth) {
      // if the local was closed over, make sure the upvalue gets closed
      // when it goes out of scope on the stack
      if (locals_[local].is_upvalue)
        emit_byte(Code::CLOSE_UPVALUE);
      else
        emit_byte(Code::POP);

      --local;
    }

    return Xt::as_type<int>(locals_.size()) - local - 1;
  }

  void push_scope(void) {
    // starts a new local block scope

    ++scope_depth_;
  }

  void pop_scope(void) {
    // closes the last pushed block scope and discards any local variables
    // decalred in that scope, this should only be called in a statement
    // context where no temporaries are still on the stack

    locals_.resize(locals_.size() - discard_locals(scope_depth_));
    --scope_depth_;
  }

  const GrammerRule& get_rule(TokenKind kind) const {
#define RULE(fn) [](Compiler* c, bool b) { c->fn(b); }
#define SIGN(fn) [](Compiler* c, Signature& s) { c->fn(s); }

#define UNUSED {nullptr, nullptr, nullptr, Precedence::NONE, nullptr}
#define PREFIX(fn) {RULE(fn), nullptr, nullptr, Precedence::NONE, nullptr}
#define INFIX(fn, prec) {nullptr, RULE(fn), nullptr, prec, nullptr}
#define INFIXOP(prec, name) {nullptr, RULE(infix_oper), SIGN(infix_signature), prec, name}
#define OPER(prec, name) {RULE(unary_oper), RULE(infix_oper), SIGN(mixed_signature), prec, name}
#define OPER2(pfn, ifn, prec) {RULE(pfn), RULE(ifn), SIGN(subscript_signature), prec, nullptr}
#define PREFIXOP(name) {RULE(unary_oper), nullptr, SIGN(unary_signature), Precedence::NONE, name}
#define OP(name) {RULE(unary_oper), RULE(infix_oper), SIGN(mixed_signature), Precedence::TERM, name}
#define PREFIXNAME {RULE(variable), nullptr, SIGN(named_signature), Precedence::NONE, nullptr}
#define CTOROP {nullptr, nullptr, SIGN(ctor_signature), Precedence::NONE, nullptr}
    static const GrammerRule _rules[] = {
      PREFIX(grouping),                         // PUNCTUATOR(LPAREN, "(")
      UNUSED,                                   // PUNCTUATOR(RPAREN, ")")
      OPER2(list, subscript, Precedence::CALL), // PUNCTUATOR(LBRACKET, "[")
      UNUSED,                                   // PUNCTUATOR(RBRACKET, "]")
      PREFIX(map),                              // PUNCTUATOR(LBRACE, "{")
      UNUSED,                                   // PUNCTUATOR(RBRACE, "}")
      UNUSED,                                   // PUNCTUATOR(COLON, ":")
      INFIX(call, Precedence::CALL),            // PUNCTUATOR(DOT, ".")
      INFIXOP(Precedence::RANGE, ".."),         // PUNCTUATOR(DOTDOT, "..")
      INFIXOP(Precedence::RANGE, "..."),        // PUNCTUATOR(DOTDOTDOT, "...")
      UNUSED,                                   // PUNCTUATOR(COMMA, ",")
      INFIXOP(Precedence::FACTOR, "*"),         // PUNCTUATOR(STAR, "*")
      INFIXOP(Precedence::FACTOR, "/"),         // PUNCTUATOR(SLASH, "/")
      INFIXOP(Precedence::FACTOR, "%"),         // PUNCTUATOR(PERCENT, "%")
      INFIXOP(Precedence::TERM, "+"),           // PUNCTUATOR(PLUS, "+")
      OPER(Precedence::TERM, "-"),              // PUNCTUATOR(MINUS, "-")
      INFIXOP(Precedence::BIT_SHIFT, "<<"),     // PUNCTUATOR(LTLT, "<<")
      INFIXOP(Precedence::BIT_SHIFT, ">>"),     // PUNCTUATOR(GTGT, ">>")
      INFIXOP(Precedence::BIT_OR, "|"),         // PUNCTUATOR(PIPE, "|")
      INFIX(or_exp, Precedence::LOGIC_OR),      // PUNCTUATOR(PIPEPIPE, "||")
      INFIXOP(Precedence::BIT_XOR, "^"),        // PUNCTUATOR(CARET, "^")
      INFIXOP(Precedence::BIT_AND, "&"),        // PUNCTUATOR(AMP, "&")
      INFIX(and_exp, Precedence::LOGIC_AND),    // PUNCTUATOR(AMPAMP, "&&")
      PREFIXOP("!"),                            // PUNCTUATOR(BANG, "!")
      PREFIXOP("~"),                            // PUNCTUATOR(TILDE, "~")
      INFIX(condition, Precedence::ASSIGNMENT), // PUNCTUATOR(QUESTION, "?")
      UNUSED,                                   // PUNCTUATOR(EQ, "=")
      INFIXOP(Precedence::COMPARISON, "<"),     // PUNCTUATOR(LT, "<")
      INFIXOP(Precedence::COMPARISON, ">"),     // PUNCTUATOR(GT, ">")
      INFIXOP(Precedence::COMPARISON, "<="),    // PUNCTUATOR(LTEQ, "<=")
      INFIXOP(Precedence::COMPARISON, ">="),    // PUNCTUATOR(GTEQ, ">=")
      INFIXOP(Precedence::EQUALITY, "=="),      // PUNCTUATOR(EQEQ, "==")
      INFIXOP(Precedence::EQUALITY, "!="),      // PUNCTUATOR(BANGEQ, "!=")

      UNUSED,                                   // KEYWORD(BREAK, "break")
      UNUSED,                                   // KEYWORD(CLASS, "class")
      CTOROP,                                   // KEYWORD(CONSTRUCT, "construct")
      UNUSED,                                   // KEYWORD(ELSE, "else")
      PREFIX(boolean),                          // KEYWORD(FALSE, "false")
      UNUSED,                                   // KEYWORD(FOR, "for")
      UNUSED,                                   // KEYWORD(FOREIGN, "foreign")
      UNUSED,                                   // KEYWORD(IF, "if")
      UNUSED,                                   // KEYWORD(IMPORT, "import")
      UNUSED,                                   // KEYWORD(IN, "in")
      INFIXOP(Precedence::IS, "is"),            // KEYWORD(IS, "is")
      PREFIX(nil),                              // KEYWORD(NIL, "nil")
      UNUSED,                                   // KEYWORD(RETURN, "return")
      UNUSED,                                   // KEYWORD(STATIC, "static")
      PREFIX(super_exp),                        // KEYWORD(SUPER, "super")
      PREFIX(this_exp),                         // KEYWORD(THIS, "this")
      PREFIX(boolean),                          // KEYWORD(TRUE, "true")
      UNUSED,                                   // KEYWORD(VAR, "var")
      UNUSED,                                   // KEYWORD(WHILE, "while")

      PREFIX(field),                            // TOKEN(FIELD, "field")
      PREFIX(static_field),                     // TOKEN(STATIC_FIELD, "static-field")
      PREFIXNAME,                               // TOKEN(IDENTIFIER, "identifier")
      PREFIX(literal),                          // TOKEN(NUMERIC, "numeric")
      PREFIX(literal),                          // TOKEN(STRING, "string")

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
      error("expected expression");
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

  int add_local(const str_t& name) {
    // create a new local variable with [name], assumes the current scope
    // is local and the name is unique

    locals_.push_back(Local(name, scope_depth_, false));
    return Xt::as_type<int>(locals_.size() - 1);
  }

  int declare_variable(Token* token = nullptr) {
    // declares a variable in the current scope whose name is the given token
    //
    // if [token] is `nullptr` use the previously consumed token, returns its
    // symbol

    str_t name(token == nullptr ? parser_.prev().as_string() : token->as_string());

    if (name.size() > MAX_VARIABLE_NAME) {
      error("variable name cannot be longger than %d characters",
          MAX_VARIABLE_NAME);
    }

    // top-level module scope
    if (scope_depth_ == -1) {
      int symbol = parser_.get_vm().define_variable(
          parser_.get_mod(), name, nullptr);
      if (symbol == -1)
        error("module variable is already defined");
      else if (symbol == -2)
        error("too many module variables defined");
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

    return add_local(name);
  }

  int declare_named_variable(void) {
    // parses a name token and declares a variable in the current scope with
    // that name, returns its symbol

    consume(TokenKind::TK_IDENTIFIER, "expect variable name");
    return declare_variable();
  }

  void define_variable(int symbol) {
    // store the variable if it's a local, the result of the initializer
    // is in the correct slot on the stack already so we are done.
    if (scope_depth_ >= 0)
      return;

    // it's a module variable, so store the value int the module slot and
    // then discard the temporary for the initializer
    emit_words(Code::STORE_MODULE_VAR, symbol);
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
    for (int i = 0; i < num_upvalues_; ++i) {
      auto& upvalue = upvalues_[i];
      if (upvalue.is_local == is_local && upvalue.index == index)
        return i;
    }

    // if got here, need add a new upvalue
    upvalues_.push_back(CompilerUpvalue(is_local, index));
    return num_upvalues_++;
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
    //
    // if it reaches a method boundary, this stops and returns -1 since
    // methods do not close over local variables

    // if we are at the top level, we did not find it
    if (parent_ == nullptr)
      return -1;

    // if we hit the method boundary (and the name isn't a static field), then
    // stop looking for it, we'll instead treat it as a self send
    if (name[0] != '_' && parent_->enclosing_class_ != nullptr)
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
    // words, if it's a local variable in a non-immediately enclosing
    // funtion. this will `flattens` closures automatically: it will adds
    // upvalues to all of the intermediate functions to get from the function
    // where a local is declared all the way into the possibly deeply
    // nested function that is closing over it.
    int upvalue = parent_->find_upvalue(name);
    if (upvalue != -1)
      return add_upvalue(false, upvalue);

    // if got here, walked all the way up the parent chain and could not
    // find it
    return -1;
  }

  int resolve_non_module(const str_t& name, Code& load_instruction) {
    // lookup [name] in the current scope to see what name it is bound to,
    // returns the index of the name either in local scope, or the enclosing
    // function's upvalue list, does not search the module scope, returns -1
    // if not found
    //
    // sets [load_instruction] to the instruction needed to load the variable
    // will be [Code::LOAD_LOCAL] or [Code::LOAD_UPVALUE]

    // look it up in the local scope, look in reverse order so that the
    // most nested variable is found first and shadows outer ones
    load_instruction = Code::LOAD_LOCAL;
    int local = resolve_local(name);
    if (local != -1)
      return local;

    // if we go here, it's not a local, so lets see if we are closing over
    // an outer local
    load_instruction = Code::LOAD_UPVALUE;
    return find_upvalue(name);
  }

  int resolve_name(const str_t& name, Code& load_instruction) {
    // loop up [name] in the current scope to see what name it is bound
    // to returns the index of the name either in module or local scope,
    // or the enclosing function's upvalue list. returns -1 if not found.
    //
    // sets [load_instruction] to the instruction needed to load the
    // variable. will be one of [LOAD_LOCAL], [LOAD_UPVALUE] or [LOAD_MODULE_VAR]

    int non_module = resolve_non_module(name, load_instruction);
    if (non_module != -1)
      return non_module;

    load_instruction = Code::LOAD_MODULE_VAR;
    return parser_.get_mod()->find_variable(name);
  }

  inline void load_local(int slot) {
    if (slot <= 8)
      emit_byte(Code::LOAD_LOCAL_0 + slot);
    else
      emit_bytes(Code::LOAD_LOCAL, slot);
  }

  Compiler* get_enclosing_class_compiler(void) {
    // walks the compiler chain to find the compiler for the nearest class
    // enclosing this one, returns nullptr if not currently inside a class
    // definition

    Compiler* c = this;
    while (c != nullptr) {
      if (c->enclosing_class_ != nullptr)
        return c;
      c = c->parent_;
    }
    return nullptr;
  }

  ClassCompiler* get_enclosing_class(void) {
    // walks the compiler chain to find the nearest class enclosing this one
    // returns nullptr if not currently inside a class definition

    Compiler* c = get_enclosing_class_compiler();
    return c == nullptr ? nullptr : c->enclosing_class_;
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

  void literal(bool allow_assignment) {
    const Token& tok = parser_.prev();
    if (tok.kind() == TokenKind::TK_NUMERIC) {
      emit_constant(tok.as_numeric());
    }
    else {
      emit_constant(
          StringObject::make_string(parser_.get_vm(), tok.as_string()));
    }
  }

  void load_core_variable(const str_t& name) {
    // pushes the value of a module-level variable implicitly imported from core

    int symbol = parser_.get_mod()->find_variable(name);
    ASSERT(symbol != -1, "should have already define core name");
    emit_words(Code::LOAD_MODULE_VAR, symbol);
  }

  void list(bool allow_assignment) {
    // instantiate a new list
    load_core_variable("List");
    call_method(0, "new()");

    // compile the list elements, each one compiles to a ".add()" call
    do {
      ignore_newlines();

      // stop if we hit the end of the list
      if (parser_.curr().kind() == TokenKind::TK_RBRACKET)
        break;

      emit_byte(Code::DUP);
      expression();

      call_method(1, "add(_)");

      // discard the result of the add() call
      emit_byte(Code::POP);
    } while (match(TokenKind::TK_COMMA));

    // allow newlines before the closing `]`
    ignore_newlines();
    consume(TokenKind::TK_RBRACKET, "expect `]` after list elements");
  }

  void map(bool allow_assignment) {
    // instantiate a new map
    load_core_variable("Map");
    call_method(0, "new()");

    // compile the map elements, each one is compiled to just invoke the
    // subscript setter on the map
    do {
      ignore_newlines();

      // stop if we hit the end of the map
      if (parser_.curr().kind() == TokenKind::TK_RBRACE)
        break;

      // push a copy of the map since the subscript call will consume it
      emit_byte(Code::DUP);

      // the key
      parse_precedence(false, Precedence::PRIMARY);
      consume(TokenKind::TK_COLON, "expect `:` after map key");
      ignore_newlines();

      // the value
      expression();

      call_method(2, "[_]=(_)");

      // discard the result of the setter call
      emit_byte(Code::POP);
    } while (match(TokenKind::TK_COMMA));

    // allow newlines before the closing `}`
    ignore_newlines();
    consume(TokenKind::TK_RBRACE, "expect `}` after map entries");
  }

  void load_this(void) {
    // loads the receiver of the currently enclosing method. correctly
    // handles functions defined inside methods

    Code load_instruction;
    int index = resolve_non_module("this", load_instruction);
    if (load_instruction == Code::LOAD_LOCAL)
      load_local(index);
    else
      emit_bytes(load_instruction, index);
  }

  void variable_impl(bool allow_assignment, int index, Code load_instruction) {
    // compiles a read or assignment to a variable at [index]
    // using [load_instruction]

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
      case Code::LOAD_MODULE_VAR: emit_words(Code::STORE_MODULE_VAR, index); break;
      default: UNREACHABLE();
      }
    }
    else if (load_instruction == Code::LOAD_MODULE_VAR) {
      emit_words(load_instruction, index);
    }
    else if (load_instruction == Code::LOAD_LOCAL) {
      load_local(index);
    }
    else {
      emit_bytes(load_instruction, index);
    }
  }

  void static_field(bool allow_assignment) {
    Code load_instruction = Code::LOAD_LOCAL;
    int index = 0xff;

    Compiler* class_compiler = get_enclosing_class_compiler();
    if (class_compiler == nullptr) {
      error("cannot use a static field outside of a class definition");
    }
    else {
      str_t name = parser_.prev().as_string();

      if (class_compiler->resolve_local(name) == -1) {
        int symbol = class_compiler->declare_variable();

        // implicitly initialize it to nil
        class_compiler->emit_byte(Code::NIL);
        class_compiler->define_variable(symbol);
      }

      // it definitely exists now, so resolve it properly, this is different
      // from the above resolve_local(...) call because we may have already
      // closed over it as an upvalue
      index = resolve_name(name, load_instruction);
    }

    variable_impl(allow_assignment, index, load_instruction);
  }

  inline bool is_local_name(const str_t& name) const {
    // returns `true` if [name] is a local variable name (starts with a
    // lowercase letter)

    return std::islower(name[0]);
  }

  void variable(bool allow_assignment) {
    // compiles a variable name or method call with an implicit receiver

    // look for the name in the scope chain up to the nearest enclosing method
    str_t token_name = parser_.prev().as_string();

    Code load_instruction;
    int index = resolve_non_module(token_name, load_instruction);
    if (index != -1) {
      variable_impl(allow_assignment, index, load_instruction);
      return;
    }

    // if we're inside a method and the name is lowercase, treat it as a method
    // on this
    if (is_local_name(token_name) && get_enclosing_class() != nullptr) {
      load_this();
      named_call(allow_assignment, Code::CALL_0);
      return;
    }

    // otherwise, look for a global variable with the name
    int module = parser_.get_mod()->find_variable(token_name);
    if (module == -1) {
      if (is_local_name(token_name)) {
        error("undefined variable");
        return;
      }

      // if it's a nonlocal name, implicitly define a module-level variable in
      // the hopes that we get a real definition later
      module = parser_.get_vm().declare_variable(parser_.get_mod(), token_name);
      if (module == -2) {
        error("too many module variables defined");
      }
    }
    variable_impl(allow_assignment, module, Code::LOAD_MODULE_VAR);
  }

  void field(bool allow_assignment) {
    // initialize it with a fake value so we can keep parsing and minimize
    // the number of cascaded erros
    int field = 0xff;

    ClassCompiler* enclosing_class = get_enclosing_class();
    if (enclosing_class == nullptr) {
      error("cannot reference a field outside of a class definition");
    }
    else if (enclosing_class->is_foreign) {
      error("cannot define fields in a foreign class");
    }
    else if (enclosing_class->in_static) {
      error("cannot use an instance field in a static method");
    }
    else {
      // look up the field, or implicitlt define it
      field = enclosing_class->fields->ensure(parser_.prev().as_string());

      if (field >= MAX_FIELDS)
        error("a class can only have %d fields", MAX_FIELDS);
    }

    // if there is an `=` after a filed name, it's an assignment
    bool is_load = true;
    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      // compile the right-hand side
      expression();
      is_load = false;
    }

    // if we are directly inside a method, use a more optional instruction
    if (parent_ != nullptr && parent_->enclosing_class_ == enclosing_class) {
      emit_bytes(is_load ?
          Code::LOAD_FIELD_THIS : Code::STORE_FIELD_THIS, field);
    }
    else {
      load_this();
      emit_bytes(is_load ? Code::LOAD_FIELD : Code::STORE_FIELD, field);
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

    if (parser_.prev().kind() != expected) {
      error(msg);

      // if the next token is the one we want, assume the current one is just
      // a spurious error and discard it to minimize the number of cascaded
      // errors
      if (parser_.curr().kind() == expected)
        parser_.advance();
    }
  }

  bool match_line(void) {
    // matches one or more newlines, returns true if at least one was found

    if (!match(TokenKind::TK_NL))
      return false;

    while (match(TokenKind::TK_NL)) {
    }
    return true;
  }

  void ignore_newlines(void) {
    // consumes the current token if its type is [expected], returns true if a
    // token was consumed, since [expected] is known to be in the middle of an
    // expression, any newlines following it are consumed and discarded

    match_line();
  }

  void consume_line(const char* msg) {
    // consumes the current token, emits an error if it is not a newline, then
    // discards any duplicate newlines following it

    consume(TokenKind::TK_NL, msg);
    ignore_newlines();
  }

  void start_loop(Loop* loop) {
    // marks the beginning of a lopp, keeps track of the current instruction
    // so we know what to loop back to at the end of the body

    loop->enclosing = loop_;
    loop->start = Xt::as_type<int>(bytecode_.size() - 1);
    loop->scope_depth = scope_depth_;
    loop_ = loop;
  }

  void test_exit_loop(void) {
    // emit the [Code::JUMP_IF] instruction used to test the loop condition
    // and potentially exit the loop, keeps tracks of the instruction so we
    // can patch it later once we know the end of the body is

    loop_->exit_jump = emit_jump(Code::JUMP_IF);
  }

  void loop_body(void) {
    // compiles the body of the loop and tracks its extent so that contained
    // `break` statements can be handled correctly

    loop_->body = Xt::as_type<int>(bytecode_.size());
    block();
  }

  void finish_loop(void) {
    // ends the current innermost loop, patches up all jumps and breaks now
    // that we know where the end of the loop is

    int loop_offset = Xt::as_type<int>(bytecode_.size()) - loop_->start + 2;
    emit_words(Code::LOOP, loop_offset);

    patch_jump(loop_->exit_jump);

    // find any break placeholder instructions (which will be Code::END in the
    // bytecode) and replace them with real jumps
    int i = loop_->body;
    while (i < bytecode_.size()) {
      if (Xt::as_type<Code>(bytecode_[i]) == Code::END) {
        bytecode_[i] = Xt::as_type<u8_t>(Code::JUMP);
        patch_jump(i + 1);
        i += 3;
      }
      else {
        // skip this instruction and its arguments
        i += 1 + FunctionObject::get_argc(
            bytecode_.data(), constants_.data(), i);
      }
    }
    loop_ = loop_->enclosing;
  }

  void class_definition(bool is_foreign) {
    // compiles a class definition, assumes the `class` token has already
    // been consumed (along with a possibly preceding `foreign` token)

    // create a variable to store the class in
    int slot = declare_named_variable();

    // make a string constant for the name
    emit_constant(
        StringObject::make_string(parser_.get_vm(), parser_.prev().as_string()));

    // load the superclass (if there is one)
    if (match(TokenKind::KW_IS)) {
      parse_precedence(false, Precedence::CALL);
    }
    else {
      // implicitly inherit from Object
      load_core_variable("Object");
    }

    // store a placeholder for the number of fields argument, we donot
    // know the value untial we have compiled all the methods to see
    // which fields are used.
    int num_fields_instruction = -1;
    if (is_foreign)
      emit_byte(Code::FOREIGN_CLASS);
    else
      num_fields_instruction = emit_bytes(Code::CLASS, 0xff);

    // store it in its name
    define_variable(slot);

    // push a local variable scope, static fields in a class body are hoisted
    // out into local variables declared in this scope, methods that use them
    // will have upvalues referencing them
    push_scope();

    ClassCompiler class_compiler;
    class_compiler.is_foreign = is_foreign;

    // set up a symbol table for the class's fields, we'll initially compile
    // them to slots starting at zero. when the method is bound to the close
    // the bytecode will be adjusted by [bind_method] to take inherited fields
    // into account.
    SymbolTable fields;
    class_compiler.fields = &fields;
    enclosing_class_ = &class_compiler;

    consume(TokenKind::TK_LBRACE, "expect `{` after class declaration");

    match_line();

    while (!match(TokenKind::TK_RBRACE)) {
      if (!method(&class_compiler, slot))
        break;

      // don't require a newline after the last definition
      if (match(TokenKind::TK_RBRACE))
        break;

      consume_line("expect newline after definition in class");
    }

    // update the class with the number of fields
    if (!is_foreign)
      bytecode_[num_fields_instruction] = fields.count();

    enclosing_class_ = nullptr;
    pop_scope();
  }

  void import(void) {
    consume(TokenKind::TK_STRING, "expect a string after `import`");
    int module_constant = add_constant(
        StringObject::make_string(parser_.get_vm(), parser_.prev().as_string()));

    // load the module
    emit_words(Code::LOAD_MODULE, module_constant);
    // discard the unused result value from calling the module's fiber
    emit_byte(Code::POP);

    // the for clause is optional
    if (!match(TokenKind::KW_FOR))
      return;

    // compile the comma-separated list of variables to import
    do {
      int slot = declare_named_variable();

      // define a string constant for the variable name
      int variable_constant = add_constant(StringObject::make_string(
            parser_.get_vm(), parser_.prev().as_string()));
      // load the variable from the other module
      emit_words(Code::IMPORT_VARIABLE, module_constant);
      emit_u16(variable_constant);

      // stores the result in the variable here
      define_variable(slot);
    } while (match(TokenKind::TK_COMMA));
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
    //       while (iter_ = seq_.iterate(iter_)) {
    //         var i = seq_.iterValue(iter_)
    //         IO.write(i)
    //       }
    //     }
    //
    // It's not exactly this, because the synthetic variables `seq_` and `iter_`
    // actually get names that aren't valid Wren identfiers, but that's the basic
    // idea.
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
    ignore_newlines();

    // evaluate the sequence expression and store it in a hidden local variable
    // the scope in the variable name ensures it won't collide with a user-defined
    // variable
    expression();
    int seq_slot = add_local("seq ");

    // create another hidden local for the iterator object
    nil(false);
    int iter_slot = add_local("iter ");

    consume(TokenKind::TK_RPAREN, "expect `)` after loop expression");

    Loop loop;
    start_loop(&loop);

    // advance the iterator by calling the `.iterate` method on the sequence
    load_local(seq_slot);
    load_local(iter_slot);

    call_method(1, "iterate(_)");

    // store the iterator back in its local for the next iteration
    emit_bytes(Code::STORE_LOCAL, iter_slot);

    test_exit_loop();

    // get the current value in the sequence by calling `.iterValue`
    load_local(seq_slot);
    load_local(iter_slot);

    call_method(1, "iterValue(_)");

    // bind the loop variable in its own scope, this ensures we get a fresh
    // variable each iteration so that closures for it donot all see the
    // same one
    push_scope();
    add_local(name);

    loop_body();
    // loop variable
    pop_scope();
    finish_loop();

    // hidden variables
    pop_scope();
  }

  void while_stmt(void) {
    Loop loop;
    start_loop(&loop);

    // compile the condition
    consume(TokenKind::TK_LPAREN, "expect `(` after `while`");
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after while condition");

    test_exit_loop();
    loop_body();
    finish_loop();
  }

  void var_definition(void) {
    // grab its name, but don't declare it yet, a (local) variable shouldn't
    // be in scope in its own initializer
    consume(TokenKind::TK_IDENTIFIER, "expect variable name");
    Token name_token = parser_.prev();

    // compile the initializer
    if (match(TokenKind::TK_EQ)) {
      expression();
    }
    else {
      // default initialize it to nil
      nil(false);
    }

    // now put it in scope
    int symbol = declare_variable(&name_token);
    define_variable(symbol);
  }

  void definition(void) {
    // compiles a `definition`, there are the statements that bind new
    // variables, they can only appear at the top level of a block and are
    // prohibited in places like the non-curly body of an if or while

    if (match(TokenKind::KW_CLASS)) {
      class_definition(false);
    }
    else if (match(TokenKind::KW_FOREIGN)) {
      consume(TokenKind::KW_CLASS, "expect `class` after `foreign`");
      class_definition(true);
    }
    else if (match(TokenKind::KW_IMPORT)) {
      import();
    }
    else if (match(TokenKind::KW_VAR)) {
      var_definition();
    }
    else {
      block();
    }
  }

  void statement(void) {
    // compiles a statement, there can only appear at the top-level or within
    // curly blocks, unlike expressions, these do not leave a value on the stack

    if (match(TokenKind::KW_BREAK)) {
      if (loop_ == nullptr) {
        error("cannot use `break` outside of a loop");
        return;
      }

      // since we will be jumping out of the scope, make sure any locals in it
      // are discarded first
      discard_locals(loop_->scope_depth + 1);

      // emit a placeholder instruction for the jump to the end of the body,
      // when we are done compiling the loop body and know where the end is.
      // we will replace these with `JUMP` instructions with appropriate offsets
      // we use `END` here because that cannot occur in the middle of bytecode
      emit_jump(Code::END);
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
      int if_jump = emit_jump(Code::JUMP_IF);

      // compile the then branch
      block();

      if (match(TokenKind::KW_ELSE)) {
        // jump over the else branch when the if branch is taken
        int else_jump = emit_jump(Code::JUMP);

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
      // compile the return value
      if (parser_.curr().kind() == TokenKind::TK_NL) {
        // impilicity return nil if there is no value
        emit_byte(Code::NIL);
      }
      else {
        expression();
      }
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
      if (finish_block()){
        // block was an expression, so discard it
        emit_byte(Code::POP);
      }
      pop_scope();
      return;
    }

    // single statement body
    statement();
  }

  bool finish_block(void) {
    // returns true if it was a expresion body, false if it was a statement
    // body (most precisely, returns true if a value was left on the stack,
    // an empty block returns false)

    // empty blocks do nothing
    if (match(TokenKind::TK_RBRACE))
      return false;

    // if there is no newline after the `{`, it's a single expression body
    if (!match_line()) {
      expression();
      consume(TokenKind::TK_RBRACE, "expect `}` at end of block");
      return true;
    }

    // empty blocks (with just a newline inside) do nothing
    if (match(TokenKind::TK_RBRACE))
      return false;

    // compile the definition list
    do {
      definition();

      // if we got into a weird error state, donot get stuck in a loop
      if (parser_.curr().kind() == TokenKind::TK_EOF)
        return true;

      consume_line("expect newline after statement");
    } while (!match(TokenKind::TK_RBRACE));
    return false;
  }

  void finish_body(bool is_initializer) {
    // parses a method or function body, after the initial `{` has been consumed
    //
    // if [is_initializer] is `true`, this is the body of a constructor
    // initializer, in that case, this adds the code to ensure it returns `this`

    bool is_expression_body = finish_block();
    if (is_initializer) {
      // if the initializer body evaluates to a value, discard it
      if (is_expression_body)
        emit_byte(Code::POP);

      // the receiver is always stored in the first local slot
      emit_byte(Code::LOAD_LOCAL_0);
    }
    else if (!is_expression_body) {
      // implicitly return nil in statement bodies
      emit_byte(Code::NIL);
    }
    emit_byte(Code::RETURN);
  }

  void create_constructor(const Signature& signature, int initializer_symbol) {
    // creates a matching constructor method for an initializer with [signature]
    // and [initializer_symbol]
    //
    // construction is a two-stage process in Wren that involves two separate
    // methods, there is a static method that allocates a new instance of the
    // class, it then invokes an initializer method on the new instance,
    // forwarding all of the constructor arguments to it.
    //
    // the allocator method always has a fixed implementation
    //
    //    CONSTANT  - replace the class in slot 0 with a new instance of it
    //    NEW       - invoke the initializer with [initializer_symbol]
    //
    // this creates that method and calls the initializer with [initializer_symbol]

    Compiler method_compiler(parser_, this);
    method_compiler.init_compiler(false);

    // allocate the instance
    method_compiler.emit_byte(enclosing_class_->is_foreign
        ? Code::FOREIGN_CONSTRUCT : Code::CONSTRUCT);

    // run its initializer
    method_compiler.emit_words(
        Code::CALL_0 + signature.arity, initializer_symbol);

    // return the instance
    method_compiler.emit_byte(Code::RETURN);
    method_compiler.finish_compiler("");
  }

  void define_method(int class_slot, bool is_static, int method_symbol) {
    // loads the enclosing class onto the stack and then binds the function
    // already on the stack as a method on that class

    // load the class we have to do this for each method because we cannot
    // keep the class on top of the stack, if there are statid fields, they
    // will be locals above the initial variable slot for the class on the
    // stack, to skip past those, we just load the class each time right
    // before defining a method
    if (scope_depth_ == 0) {
      // the class is at the top level (scope depth is 0, not -1 to account
      // for the static variable scope surrounding the class itself), so load
      // it from there
      emit_words(Code::LOAD_MODULE_VAR, class_slot);
    }
    else {
      load_local(class_slot);
    }

    // define the method
    Code instruction = is_static ? Code::METHOD_STATIC : Code::METHOD_INSTANCE;
    emit_words(instruction, method_symbol);
  }

  bool method(ClassCompiler* class_compiler, int class_slot) {
    // compiles a method definition inside a class body
    //
    // returns `true` if compiled successfully or `false` if the method
    // couldn't be parsed

    bool is_foreign = match(TokenKind::KW_FOREIGN);
    class_compiler->in_static = match(TokenKind::KW_STATIC);

    SignatureFn signature_fn = get_rule(parser_.curr().kind()).method;
    parser_.advance();

    if (!signature_fn) {
      error("expect method definition");
      return false;
    }

    // build the method signature
    Signature signature = signature_from_token(SignatureType::GETTER);
    class_compiler->signature = &signature;

    Compiler method_compiler(parser_, this);
    method_compiler.init_compiler(false);

    // compile the method signature
    signature_fn(&method_compiler, signature);
    if (class_compiler->in_static && signature.type == SignatureType::INITIALIZER)
      error("a constructor cannot be static");

    // include the full signature in debug messages in stack traces
    str_t full_signature = signature_to_string(signature);
    if (is_foreign) {
      // define a constant for the signature
      emit_constant(
          StringObject::make_string(parser_.get_vm(), full_signature));
    }
    else {
      consume(TokenKind::TK_LBRACE, "expect `{` to begin method body");
      method_compiler.finish_body(signature.type == SignatureType::INITIALIZER);
      method_compiler.finish_compiler(full_signature);
    }

    // define the method for a constructor, this defines the instance
    // initializer method
    int method_symbol = signature_symbol(signature);
    define_method(class_slot, class_compiler->in_static, method_symbol);
    if (signature.type == SignatureType::INITIALIZER) {
      // also define a matching constructor method on the metaclass
      signature.set_type(SignatureType::METHOD);
      int constructor_symbol = signature_symbol(signature);

      create_constructor(signature, method_symbol);
      define_method(class_slot, true, constructor_symbol);
    }

    return true;
  }

  void call(bool allow_assignment) {
    ignore_newlines();
    consume(TokenKind::TK_IDENTIFIER, "expect method name after `.`");
    named_call(allow_assignment, Code::CALL_0);
  }

  void subscript(bool allow_assignment) {
    // subscript or `array indexing` operator like `foo[index]`

    Signature signature("", SignatureType::SUBSCRIPT, 0);

    // parse the argument list
    finish_arguments(signature);
    consume(TokenKind::TK_RBRACKET, "expect `]` after subscript arguments");

    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");

      signature.set_type(SignatureType::SUBSCRIPT_SETTER);

      // compile the assigned value
      validate_num_parameters(signature.inc_arity());
      expression();
    }

    call_signature(Code::CALL_0, signature);
  }

  void and_exp(bool allow_assignment) {
    ignore_newlines();

    // skip the right argument if the left is false
    int jump = emit_jump(Code::AND);

    parse_precedence(false, Precedence::LOGIC_AND);
    patch_jump(jump);
  }

  void or_exp(bool allow_assignment) {
    ignore_newlines();

    // skip the right argument if the left if true
    int jump = emit_jump(Code::OR);

    parse_precedence(false, Precedence::LOGIC_OR);
    patch_jump(jump);
  }

  void condition(bool allow_assignment) {
    // ignore newline after `?`
    ignore_newlines();

    // jump to the else branch if the condition is false
    int if_jump = emit_jump(Code::JUMP_IF);

    // compile the then branch
    parse_precedence(allow_assignment, Precedence::TERNARY);
    consume(TokenKind::TK_COLON,
        "expect `:` after then branch of condition operator");
    ignore_newlines();

    // jump over the else branch when the if branch is taken
    int else_jump = emit_jump(Code::JUMP);
    // compile the else branch
    patch_jump(if_jump);
    parse_precedence(allow_assignment, Precedence::ASSIGNMENT);

    // patch the jump over the else
    patch_jump(else_jump);
  }

  void infix_oper(bool allow_assignment) {
    auto& rule = get_rule(parser_.prev().kind());

    // an infix operator cannot end an expression
    ignore_newlines();

    // compile the right hand side
    parse_precedence(false, rule.precedence + 1);

    // call the operator method on the left-hand side
    Signature signature(rule.name, SignatureType::METHOD, 1);
    call_signature(Code::CALL_0, signature);
  }

  void unary_oper(bool allow_assignment) {
    auto& rule = get_rule(parser_.prev().kind());

    ignore_newlines();

    // compile the argument
    parse_precedence(false, Precedence::UNARY + 1);

    // call the operator method on the left-hand side
    call_method(0, str_t(1, rule.name[0]));
  }

  void infix_signature(Signature& signature) {
    // compiles a method signature for an infix operator

    // add a space for the RHS parameter
    signature.set_type(SignatureType::METHOD);
    signature.set_arity(1);

    // parse the parameter name
    consume(TokenKind::TK_LPAREN, "expect `(` after operator name");
    declare_named_variable();
    consume(TokenKind::TK_RPAREN, "expect `)` after parameter name");
  }

  void unary_signature(Signature& signature) {
    // compiles a method signature for an unary operator
    signature.set_type(SignatureType::GETTER);
  }

  void mixed_signature(Signature& signature) {
    signature.set_type(SignatureType::GETTER);
    // if there is a parameter, it's an infix operator, otherwise it's unary
    if (match(TokenKind::TK_LPAREN)) {
      // add a space for the RHS parameter
      signature.set_type(SignatureType::METHOD);
      signature.set_arity(1);

      // parse the parameter name
      declare_named_variable();
      consume(TokenKind::TK_RPAREN, "expect `)` after parameter name");
    }
  }

  bool maybe_setter(Signature& signature) {
    // compiles an optional setter parameter in a method [signature]
    //
    // returns `true` if it was a setter

    // see if it is a setter
    if (!match(TokenKind::TK_EQ))
      return false;

    // it is a setter
    if (signature.type == SignatureType::SUBSCRIPT)
      signature.set_type(SignatureType::SUBSCRIPT_SETTER);
    else
      signature.set_type(SignatureType::SETTER);

    // parse the value parameter
    consume(TokenKind::TK_LPAREN, "expect `(` after `=`");
    declare_named_variable();
    consume(TokenKind::TK_RPAREN, "expect `)` after parameter name");

    signature.inc_arity();

    return true;
  }

  void subscript_signature(Signature& signature) {
    // compiles a method signature for a subscript operator

    signature.set_type(SignatureType::SUBSCRIPT);
    signature.set_name("");

    // parse the parameters inside the subscript
    finish_parameters(signature);
    consume(TokenKind::TK_RBRACKET, "expect `]` after parameters");

    maybe_setter(signature);
  }

  void named_signature(Signature& signature) {
    // compiles a method signature for a named method or setter

    signature.set_type(SignatureType::GETTER);
    if (maybe_setter(signature))
      return;

    // regular named method with an optional parameter list
    parameters(signature);
  }

  void ctor_signature(Signature& signature) {
    consume(TokenKind::TK_IDENTIFIER,
        "expect constructor name after `construct`");

    // capture the name
    signature = signature_from_token(SignatureType::INITIALIZER);

    if (match(TokenKind::TK_EQ))
      error("a constructor cannot be a setter");
    if (!match(TokenKind::TK_LPAREN)) {
      error("a constructor cannot be a getter");
      return;
    }

    // allow an empty parameter list
    if (match(TokenKind::TK_RPAREN))
      return;

    finish_parameters(signature);
    consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
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

  void finish_parameters(Signature& signature) {
    // parses the rest of a common-separated parameter list after the opening
    // delimeter, updates `arity` in [signature] with the number of parameters

    do {
      ignore_newlines();
      validate_num_parameters(signature.inc_arity());

      // define a local variable in the method for the parameter
      declare_named_variable();
    } while (match(TokenKind::TK_COMMA));
  }

  void parameters(Signature& signature) {
    // parses an optional parenthesis-delimited parameter list, updates `type`
    // and `arity` in [signature] to match what was parsed

    // the parameter list is optional
    if (!match(TokenKind::TK_LPAREN))
      return;

    signature.set_type(SignatureType::METHOD);

    if (match(TokenKind::TK_RPAREN))
      return;

    finish_parameters(signature);
    consume(TokenKind::TK_RPAREN, "expect `)` after parameters");
  }

  inline int method_symbol(const str_t& name) { return vm_mnames().ensure(name); }

  void signature_parameters(
      str_t& name, int num_params, char lbracket, char rbracket) {
    // appends characters to [name] for [num_params] `_` surrounded by
    // [lbracket] and [rbracket]

    name.push_back(lbracket);
    for (int i = 0; i < num_params; ++i) {
      if (i > 0)
        name.push_back(',');
      name.push_back('_');
    }
    name.push_back(rbracket);
  }

  str_t signature_to_string(const Signature& signature) {
    str_t name(signature.name);
    switch (signature.type) {
    case SignatureType::METHOD:
      signature_parameters(name, signature.arity, '(', ')');
      break;
    case SignatureType::GETTER:
      break;
    case SignatureType::SETTER:
      name.push_back('=');
      signature_parameters(name, 1, '(', ')');
      break;
    case SignatureType::SUBSCRIPT:
      signature_parameters(name, signature.arity, '[', ']');
      break;
    case SignatureType::SUBSCRIPT_SETTER:
      signature_parameters(name, signature.arity - 1, '[', ']');
      name.push_back('=');
      signature_parameters(name, 1, '(', ')');
      break;
    case SignatureType::INITIALIZER:
      name = "init " + signature.name;
      signature_parameters(name, signature.arity, '(', ')');
      break;
    }

    return name;
  }

  int signature_symbol(Signature& signature) {
    // gets the symbol for a method with [signature]

    return method_symbol(signature_to_string(signature));
  }

  Signature signature_from_token(SignatureType type) {
    // returns a signature with [type] whose name is from the last consumed
    // token

    // get the token for the method name
    const Token& token = parser_.prev();
    Signature signature(token.as_string(), type, 0);

    if (signature.name.size() > MAX_METHOD_NAME) {
      error("method names cannot be longer than %d characters", MAX_METHOD_NAME);
      signature.set_name(signature.name.substr(0, MAX_METHOD_NAME));
    }

    return signature;
  }

  void finish_arguments(Signature& signature) {
    // parses a comma-separated list of arguments, modifies [name] to include
    // the arity of the argument list

    do {
      ignore_newlines();
      validate_num_parameters(signature.inc_arity());
      expression();
    } while (match(TokenKind::TK_COMMA));
    // allow a newline before the closing delimiter
    ignore_newlines();
  }

  inline void call_signature(Code instruction, Signature& signature) {
    // compiles a method call with [signature] using [instruction]

    int symbol = signature_symbol(signature);
    emit_words(instruction + signature.arity, symbol);

    if (instruction == Code::SUPER_0)
      emit_u16(add_constant(nullptr));
  }

  inline void call_method(int argc, const str_t& name) {
    // compiles a method call with [argc] for a method with [name]

    emit_words(Code::CALL_0 + argc, method_symbol(name));
  }

  void method_call(Code instruction, const Signature& signature) {
    // compiles an (optional) argument list for a method call with
    // [method_signature] and then calls it

    // make a new signature that conatians the updated arity and type based on
    // the arguments we find
    Signature called(signature.name, SignatureType::GETTER, 0);

    // parse the argument list, if any
    if (match(TokenKind::TK_LPAREN)) {
      called.set_type(SignatureType::METHOD);

      // allow empty an argument list
      if (parser_.curr().kind() != TokenKind::TK_RPAREN) {
        finish_arguments(called);
      }
      consume(TokenKind::TK_RPAREN, "expect `)` after arguments");
    }

    // parse the block argument, if any
    if (match(TokenKind::TK_LBRACE)) {
      called.set_type(SignatureType::METHOD);
      called.inc_arity();

      Compiler fn_compiler(parser_, this);
      fn_compiler.init_compiler(true);

      // make a dummy signature to track the arity
      Signature fn_signature("", SignatureType::METHOD, 0);

      // parse the parameter list, if any
      if (match(TokenKind::TK_PIPE)) {
        fn_compiler.finish_parameters(fn_signature);
        consume(TokenKind::TK_PIPE, "expect `|` after function parameters");
      }
      fn_compiler.num_params_ = fn_signature.arity;

      fn_compiler.finish_body(false);

      // name the funciton based on the method its passed to
      str_t block_name = signature_to_string(called);
      block_name += " block argument";

      fn_compiler.finish_compiler(block_name);
    }

    // if this is a super() call for an initializer, make sure we got an
    // actual argument list
    if (signature.type == SignatureType::INITIALIZER) {
      if (called.type != SignatureType::METHOD)
        error("a superclass constructor must have an argument list");
      called.set_type(SignatureType::INITIALIZER);
    }

    call_signature(instruction, called);
  }

  void named_call(bool allow_assignment, Code instruction) {
    // compiles a call whose name is the previously consumed token, this
    // includes getters, method calls with arguments, and setter calls

    // get the token for the method name
    Signature signature = signature_from_token(SignatureType::GETTER);

    if (match(TokenKind::TK_EQ)) {
      if (!allow_assignment)
        error("invalid assignment");
      ignore_newlines();

      signature.set_type(SignatureType::SETTER);
      signature.set_arity(1);

      expression();
      call_signature(instruction, signature);
    }
    else {
      method_call(instruction, signature);
    }
  }

  void super_exp(bool allow_assignment) {
    ClassCompiler* enclosing_class = get_enclosing_class();
    if (enclosing_class == nullptr) {
      error("cannot use `super` outside of a method");
    }

    load_this();
    // see if it's a nemd super call, or an unnamed one
    if (match(TokenKind::TK_DOT)) {
      // compile the superclass call
      consume(TokenKind::TK_IDENTIFIER, "expect method name after `super`");
      named_call(allow_assignment, Code::SUPER_0);
    }
    else if (enclosing_class != nullptr) {
      // no explicit name, so use the name of the enclosing method, make sure
      // we check that enclosing_class_ is not nullptr first, we've already
      // reported the error, but we don't want to crash here

      method_call(Code::SUPER_0, *enclosing_class->signature);
    }
  }

  void this_exp(bool allow_assignment) {
    if (get_enclosing_class() == nullptr) {
      error("cannot use `this` outside of a method");
      return;
    }

    load_this();
  }
public:
  Compiler(Parser& parser, Compiler* parent = nullptr) noexcept
    : parser_(parser)
    , parent_(parent) {
  }

  ~Compiler(void) {
    // FIXME: fixed deallocate FunctionObject by GC
  }

  void init_compiler(bool is_func = true) {
    parser_.get_vm().set_compiler(this);

    if (parent_ == nullptr) {
      scope_depth_ = -1;
    }
    else {
      if (is_func)
        locals_.push_back(Local());
      else
        locals_.push_back(Local("this"));
      scope_depth_ = 0;
    }
  }

  FunctionObject* finish_compiler(const str_t& debug_name) {
    // finishes [compiler], which is compiling a function, method or chunk of
    // top level code. if there is a parent compiler, then this emits code in
    // the parent compiler to loadd the resulting function

    // if we hit an error, donot bother creating the function since it's borked
    // anyway
    if (parser_.had_error()) {
      parser_.get_vm().set_compiler(parent_);
      return nullptr;
    }

    // mark the end of the bytecode, since it may contain multiple early
    // returns, we cannot rely on `RETURN` to tell use we are at the end.
    emit_byte(Code::END);

    // create a function object for the code we just compiled
    FunctionObject* fn = FunctionObject::make_function(
        parser_.get_vm(),
        parser_.get_mod(),
        num_upvalues_, num_params_,
        bytecode_.data(), Xt::as_type<int>(bytecode_.size()),
        constants_.data(), Xt::as_type<int>(constants_.size()),
        parser_.source_path(), debug_name,
        debug_source_lines_.data(), Xt::as_type<int>(debug_source_lines_.size()));

    PinnedGuard pinned_fn(parser_.get_vm(), fn);

    // in the function that contains this one, load the resulting function
    // object.
    if (parent_ != nullptr) {
      int constant = parent_->add_constant(fn);

      // if the function has no upvalues, we donot need to create a closure.
      // we can just load and run the function directly
      if (num_upvalues_ == 0) {
        parent_->emit_words(Code::CONSTANT, constant);
      }
      else {
        // capture the upvalues in the new closure object.
        parent_->emit_words(Code::CLOSURE, constant);

        // emit a arguments for each upvalue to know whether to capture a
        // local or an upvalue
        for (int i = 0; i < num_upvalues_; ++i) {
          auto& uv = upvalues_[i];
          parent_->emit_bytes(uv.is_local ? 1 : 0, uv.index);
        }
      }
    }

    // pop this compiler off the stack
    parser_.get_vm().set_compiler(parent_);

    return fn;
  }

  FunctionObject* compile_function(TokenKind end_kind) {
    ignore_newlines();
    for (;;) {
      definition();

      // if there is no newline, it must be the end of the block on the same line
      if (!match_line()) {
        consume(end_kind, "expect end of file");
        break;
      }
      if (match(end_kind))
        break;
    }
    emit_bytes(Code::NIL, Code::RETURN);

    parser_.get_mod()->iter_variables(
        [this](int i, const Value& val, const str_t& name) {
          if (val.is_undefined()) {
            error("variable `%s` is used but not defined", name.c_str());
          }
        });

    return finish_compiler("(script)");
  }

  FunctionObject* compile_function(
      Parser& p, Compiler* parent, TokenKind end_kind) {
    Compiler c(p, parent);
    return c.compile_function(end_kind);
  }

  void mark_compiler(WrenVM& vm) {
    // walk up the parent chain to mark the outer compilers too, the VM
    // only tracks the innermost one

    Compiler* c = this;
    while (c != nullptr) {
      for (auto& v : c->constants_)
        vm.mark_value(v);
      c = c->parent_;
    }
  }
};

FunctionObject* compile(WrenVM& vm, ModuleObject* module,
    const str_t& source_path, const str_t& source_bytes, bool print_errors) {
  Lexer lex(source_path, source_bytes);
  Parser p(vm, module, lex, print_errors);

  p.advance();
  Compiler c(p, nullptr);
  c.init_compiler(true);

  return c.compile_function(TokenKind::TK_EOF);
}

void mark_compiler(WrenVM& vm, Compiler* compiler) {
  compiler->mark_compiler(vm);
}

}
