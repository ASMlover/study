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
#include <cstdio>
#include <functional>
#include <iostream>
#include <vector>
#include "chunk.h"
#include "scanner.h"
#include "object.h"
#include "compiler.h"

namespace lox {

enum class Precedence {
  NONE,
  ASSIGNMENT, // =
  OR, // or
  AND, // and
  EQUALITY, // == !=
  COMPARISON, // < <= > >=
  TERM, // + -
  FACTOR, // * /
  UNARY, // ! - +
  CALL, // . () []
  PRIMARY
};

class Parser;
using ParserPtr = std::shared_ptr<Parser>;

struct ParseRule {
  std::function<void (const ParserPtr&, bool)> prefix;
  std::function<void (const ParserPtr&, bool)> infix;
  Precedence precedence;
};

struct Local {
  Token name;
  int depth{};

  inline void set_local(const Token& lname, int ldepth) {
    name = lname;
    depth = ldepth;
  }
};

static constexpr std::size_t kLocalLimit = 256;
struct CompilerImp {
  std::vector<Local> locals{kLocalLimit};
  int local_count{};
  int scope_depth{};

  inline void append_local(const Token& name) {
    auto& local = locals[local_count++];
    local.set_local(name, scope_depth);
  }
};

class Parser
  : private UnCopyable, public std::enable_shared_from_this<Parser> {
  Chunk& compiling_chunk_;
  Scanner& scan_;
  CompilerImp& curr_compiler_;
  Token prev_; // previous token
  Token curr_; // current token
  bool had_error_{};
  bool panic_mode_{};

  void error(const std::string& message) {
    error_at(prev_, message);
  }

  void error_at_current(const std::string& message) {
    error_at(curr_, message);
  }

  void error_at(const Token& tok, const std::string& message) {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "line(" << tok.get_lineno() << "): ERROR: ";
    if (tok.get_kind() == TokenKind::TK_EOF) {
      std::cerr << "at end";
    }
    else if (tok.get_kind() == TokenKind::TK_ERROR) {
      // nothing
    }
    else {
      std::cerr << "at " << tok.get_literal();
    }
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
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
      case TokenKind::KW_PRINT:
      case TokenKind::KW_RETURN:
        return;
      default:
        break;
      }

      advance();
    }
  }

  std::uint8_t make_constant(const Value& value) {
    int constant = compiling_chunk_.add_constant(value);
    if (constant > UINT8_MAX) {
      error("too many constants in one chunk ...");
      return 0;
    }
    return static_cast<std::uint8_t>(constant);
  }

  void emit_byte(std::uint8_t byte) {
    compiling_chunk_.write(byte, prev_.get_lineno());
  }

  void emit_bytes(std::uint8_t byte1, std::uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  void emit_return(void) {
    emit_byte(OpCode::OP_RETURN);
  }

  void emit_constant(const Value& value) {
    emit_bytes(OpCode::OP_CONSTANT, make_constant(value));
  }

  std::uint8_t parse_variable(const std::string& err_msg) {
    consume(TokenKind::TK_IDENTIFIER, err_msg);

    declare_variable();
    if (curr_compiler_.scope_depth > 0)
      return 0;

    return identifier_constant(prev_);
  }

  std::uint8_t identifier_constant(const Token& name) {
    return make_constant(create_string(name.get_literal()));
  }

  int resolve_local(const Token& name) {
    for (int i = curr_compiler_.local_count - 1; i >= 0; --i) {
      auto& local = curr_compiler_.locals[i];
      if (name.literal_equal(local.name))
        return i;
    }
    return -1;
  }

  void add_local(const Token& name) {
    if (curr_compiler_.local_count >= kLocalLimit) {
      error("too many local variables in functions ...");
      return;
    }
    curr_compiler_.append_local(name);
  }

  void declare_variable(void) {
    // global variable are implicitly declared
    if (curr_compiler_.scope_depth == 0)
      return;

    auto& name = prev_;
    for (int i = curr_compiler_.local_count - 1; i >= 0; --i) {
      auto& local = curr_compiler_.locals[i];
      if (local.depth != -1 && local.depth < curr_compiler_.scope_depth)
        break;
      if (name.literal_equal(local.name)) {
        error("variable with this name already declared in this scope ...");
      }
    }

    add_local(name);
  }

  void enter_scope(void) {
    ++curr_compiler_.scope_depth;
  }

  void leave_scope(void) {
    --curr_compiler_.scope_depth;

    while (curr_compiler_.local_count > 0 &&
        (curr_compiler_.locals[curr_compiler_.local_count - 1].depth >
         curr_compiler_.scope_depth)) {
      emit_byte(OpCode::OP_POP);
      --curr_compiler_.local_count;
    }
  }

  void define_variable(std::uint8_t global) {
    if (curr_compiler_.scope_depth > 0)
      return;

    emit_bytes(OpCode::OP_DEFINE_GLOBAL, global);
  }

  void parse_precedence(Precedence prec) {
    advance();
    auto prefix_rule = get_rule(prev_.get_kind()).prefix;
    if (prefix_rule == nullptr) {
      error("expect expression ...");
      return;
    }
    bool can_assign = prec <= Precedence::ASSIGNMENT;
    prefix_rule(shared_from_this(), can_assign);

    while (prec <= get_rule(curr_.get_kind()).precedence) {
      advance();
      auto infix_rule = get_rule(prev_.get_kind()).infix;
      infix_rule(shared_from_this(), can_assign);
    }

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      error("invalid assignment target ...");
      expression();
    }
  }

  ParseRule& get_rule(TokenKind kind) {
    auto numeric_fn = [](const ParserPtr& p, bool) { p->numeric(); };
    auto grouping_fn = [](const ParserPtr& p, bool) { p->grouping(); };
    auto binary_fn = [](const ParserPtr& p, bool) { p->binary(); };
    auto unary_fn = [](const ParserPtr& p, bool) { p->unary(); };
    auto literal_fn = [](const ParserPtr& p, bool) { p->literal(); };
    auto string_fn = [](const ParserPtr& p, bool) { p->string(); };
    auto variable_fn = [](const ParserPtr& p, bool b) { p->variable(b); };

    static ParseRule rules[] = {
      {nullptr, nullptr, Precedence::NONE}, // TK_ERROR
      {nullptr, nullptr, Precedence::NONE}, // TK_EOF
      {nullptr, nullptr, Precedence::NONE}, // TK_UNKNOWN
      {variable_fn, nullptr, Precedence::NONE}, // TK_IDENTIFIER
      {numeric_fn, nullptr, Precedence::NONE}, // TK_NUMERICCONST
      {string_fn, nullptr, Precedence::NONE}, // TK_STRINGLITERAL
      {grouping_fn, nullptr, Precedence::CALL}, // TK_LPAREN
      {nullptr, nullptr, Precedence::NONE}, // TK_RPAREN
      {nullptr, nullptr, Precedence::NONE}, // TK_LBRACE
      {nullptr, nullptr, Precedence::NONE}, // TK_RBRACE
      {nullptr, nullptr, Precedence::NONE}, // TK_COMMA
      {nullptr, nullptr, Precedence::CALL}, // TK_DOT
      {nullptr, nullptr, Precedence::NONE}, // TK_SEMI
      {nullptr, binary_fn, Precedence::TERM}, // TK_PLUS
      {unary_fn, binary_fn, Precedence::TERM}, // TK_MINUS
      {nullptr, binary_fn, Precedence::FACTOR}, // TK_STAR
      {nullptr, binary_fn, Precedence::FACTOR}, // TK_SLASH
      {unary_fn, nullptr, Precedence::NONE}, // TK_BANG
      {nullptr, binary_fn, Precedence::EQUALITY}, // TK_BANGEQUAL
      {nullptr, nullptr, Precedence::NONE}, // TK_EQUAL
      {nullptr, binary_fn, Precedence::EQUALITY}, // TK_EQUALEQUAL
      {nullptr, binary_fn, Precedence::COMPARISON}, // TK_GREATER
      {nullptr, binary_fn, Precedence::COMPARISON}, // TK_GREATEREQUAL
      {nullptr, binary_fn, Precedence::COMPARISON}, // TK_LESS
      {nullptr, binary_fn, Precedence::COMPARISON}, // TK_LESSEQUAL
      {nullptr, nullptr, Precedence::AND}, // KW_AND
      {nullptr, nullptr, Precedence::NONE}, // KW_CLASS
      {nullptr, nullptr, Precedence::NONE}, // KW_ELSE
      {literal_fn, nullptr, Precedence::NONE}, // KW_FLASE
      {nullptr, nullptr, Precedence::NONE}, // KW_FOR
      {nullptr, nullptr, Precedence::NONE}, // KW_FUN
      {nullptr, nullptr, Precedence::NONE}, // KW_IF
      {literal_fn, nullptr, Precedence::NONE}, // KW_NIL
      {nullptr, nullptr, Precedence::OR}, // KW_OR
      {nullptr, nullptr, Precedence::NONE}, // KW_PRINT
      {nullptr, nullptr, Precedence::NONE}, // KW_RETURN
      {nullptr, nullptr, Precedence::NONE}, // KW_SUPER
      {nullptr, nullptr, Precedence::NONE}, // KW_THIS
      {literal_fn, nullptr, Precedence::NONE}, // KW_TRUE
      {nullptr, nullptr, Precedence::NONE}, // KW_VAR
      {nullptr, nullptr, Precedence::NONE}, // KW_WHILE
      {nullptr, nullptr, Precedence::NONE}, // NUM_TOKENS
    };

    return rules[static_cast<int>(kind)];
  }
public:
  Parser(Chunk& c, Scanner& s, CompilerImp& cimp)
    : compiling_chunk_(c), scan_(s), curr_compiler_(cimp) {}
  bool had_error(void) const { return had_error_; }

  void advance(void) {
    prev_ = curr_;

    for (;;) {
      curr_ = scan_.scan_token();
      if (curr_.get_kind() != TokenKind::TK_ERROR)
        break;

      error_at_current(curr_.get_literal());
    }
  }

  void consume(TokenKind kind, const std::string& message) {
    if (curr_.get_kind() == kind) {
      advance();
      return;
    }
    error_at_current(message);
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

  void end_compiler(void) {
    emit_return();
#if defined(DEBUG_PRINT_CODE)
    if (!had_error_) {
      compiling_chunk_.disassemble("code");
    }
#endif
  }

  void expression(void) {
    // expression     -> assignment ;
    // assignment     -> ( call "." )? IDENTIFIER "=" assignment | logic_or ;
    // logic_or       -> logic_and ( "or" logic_and )* ;
    // logic_and      -> equality ( "and" equality )* ;
    // equality       -> comparison ( ( "!=" | "==" ) comparison )* ;
    // comparison     -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
    // addition       -> multiplication ( ( "-" | "+" ) multiplication )* ;
    // multiplication -> unary ( ( "*" | "/" | "%" ) unary )* ;
    // unary          -> ( "!" | "-" ) unary | call ;
    // call           -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
    // primary        -> "true" | "false" | "nil" | "this"
    //                | NUMERIC | STRING | IDENTIFIER | "(" expression ")"
    //                | "super" "." IDENTIFIER ;
    //
    // arguments      -> expression ( "," expression )* ;

    parse_precedence(Precedence::ASSIGNMENT);
  }

  void binary(void) {
    // remeber the operator
    TokenKind oper_kind = prev_.get_kind();

    // compile the right operand
    auto rule = get_rule(oper_kind);
    parse_precedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

    // emit the operator instruction
    switch (oper_kind) {
    case TokenKind::TK_PLUS: emit_byte(OpCode::OP_ADD); break;
    case TokenKind::TK_MINUS: emit_byte(OpCode::OP_SUBTRACT); break;
    case TokenKind::TK_STAR: emit_byte(OpCode::OP_MULTIPLY); break;
    case TokenKind::TK_SLASH: emit_byte(OpCode::OP_DIVIDE); break;
    case TokenKind::TK_BANGEQUAL: emit_bytes(OpCode::OP_EQUAL, OpCode::OP_NOT); break;
    case TokenKind::TK_EQUALEQUAL: emit_byte(OpCode::OP_EQUAL); break;
    case TokenKind::TK_GREATER: emit_byte(OpCode::OP_GREATER); break;
    case TokenKind::TK_GREATEREQUAL: emit_bytes(OpCode::OP_LESS, OpCode::OP_NOT); break;
    case TokenKind::TK_LESS: emit_byte(OpCode::OP_LESS); break;
    case TokenKind::TK_LESSEQUAL: emit_bytes(OpCode::OP_GREATER, OpCode::OP_NOT); break;
    default:
      return; // unreachable
    }
  }

  void numeric(void) {
    // numeric -> NUMERIC ;

    double v = prev_.as_numeric();
    emit_constant(v);
  }

  void grouping(void) {
    // grouping -> "(" expression ")" ;

    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression ...");
  }

  void unary(void) {
    // unary -> ( "-" | "!" ) expression ;

    TokenKind oper_kind = prev_.get_kind();
    // compile the operand
    parse_precedence(Precedence::UNARY);
    // emit the operator instruction
    switch (oper_kind) {
    case TokenKind::TK_MINUS: emit_byte(OpCode::OP_NEGATIVE); break;
    case TokenKind::TK_BANG: emit_byte(OpCode::OP_NOT); break;
    default:
      return; // unreachable
    }
  }

  void literal(void) {
    switch (prev_.get_kind()) {
    case TokenKind::KW_TRUE: emit_byte(OpCode::OP_TRUE); break;
    case TokenKind::KW_FALSE: emit_byte(OpCode::OP_FALSE); break;
    case TokenKind::KW_NIL: emit_byte(OpCode::OP_NIL); break;
    default: return; // unreachable
    }
  }

  void string(void) {
    emit_constant(create_string(prev_.get_literal()));
  }

  void variable(bool can_assign) {
    named_variable(prev_, can_assign);
  }

  void named_variable(const Token& name, bool can_assign) {
    std::uint8_t getop{OpCode::OP_GET_LOCAL}, setop{OpCode::OP_SET_LOCAL};
    int arg = resolve_local(name);
    if (arg == -1) {
      arg = identifier_constant(name);
      getop = OpCode::OP_GET_GLOBAL;
      setop = OpCode::OP_SET_GLOBAL;
    }

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_bytes(setop, arg);
    }
    else {
      emit_bytes(getop, arg);
    }
  }

  void declaration(void) {
    // declaration -> val_decl | statement ;

    if (match(TokenKind::KW_VAR)) {
      var_decl();
    }
    else {
      statement();
    }

    if (panic_mode_)
      synchronize();
  }

  void var_decl(void) {
    std::uint8_t global = parse_variable("expect variable name ...");
    if (match(TokenKind::TK_EQUAL))
      expression();
    else
      emit_byte(OpCode::OP_NIL);
    consume(TokenKind::TK_SEMI, "expect `;` after variable declaration ...");

    define_variable(global);
  }

  void statement(void) {
    // statement -> expr_stmt | print_stmt | block_stmt ;

    if (match(TokenKind::KW_PRINT)) {
      print_stmt();
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

  void print_stmt(void) {
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after print expression ...");
    emit_byte(OpCode::OP_PRINT);
  }

  void expr_stmt(void) {
    expression();
    emit_byte(OpCode::OP_POP);
    consume(TokenKind::TK_SEMI, "expect `;` after expression ...");
  }

  void block_stmt(void) {
    while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE)) {
      declaration();
    }

    consume(TokenKind::TK_RBRACE, "expect `}` after block ...");
  }
};

bool Compiler::compile(Chunk& chunk, const std::string& source_bytes) {
  // declaration  -> var_decl | statement ;
  // statement    -> expr_stmt | print_stmt | block_stmt ;
  // block_stmt   -> "{" declaration* "}" ;

  Scanner s(source_bytes);
  CompilerImp cimp;
  auto p = std::make_shared<Parser>(chunk, s, cimp);

  p->advance();

  while (!p->match(TokenKind::TK_EOF)) {
    p->declaration();
  }
  p->end_compiler();

  return !p->had_error();
}

}
