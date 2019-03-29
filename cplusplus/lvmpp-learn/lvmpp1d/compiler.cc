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
#include "scanner.hh"
#include "chunk.hh"
#include "compiler.hh"

namespace lvm {

enum class Precedence {
  NONE,
  ASSIGNMENT, // =
  OR, // or
  AND, // and
  EQUALITY, // == !=
  COMPARISON, // < > <= >=
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

class ParseCompiler : private UnCopyable {
public:
  struct Local {
    Token name;
    int depth{};
  };
private:
  static constexpr std::size_t kLocalLimit = 256;
  std::vector<Local> locals_{kLocalLimit};
  int scope_depth_{};
public:
  ParseCompiler(void) {}

  inline Local& get(std::size_t i) { return locals_[i]; }
  inline Local& peek(void) { return locals_.back(); }
  inline int local_count(void) const { return static_cast<int>(locals_.size()); }
  inline int scope_depth(void) const { return scope_depth_; }

  inline int inc_depth(void) { return ++scope_depth_; }
  inline int dec_depth(void) { return --scope_depth_; }
};

// [G R A M M E R]
//
// declaration  -> var_decl | statement ;
//
// statement    -> print_stmt | expr_stmt ;

class Parser
  : private UnCopyable, public std::enable_shared_from_this<Parser> {
  Chunk& compiling_chunk_;
  Scanner& scanner_;
  ParseCompiler& curr_compiler_;
  Token prev_; // previous token
  Token curr_; // current token
  bool had_error_{};
  bool panic_mode_{};

  void error_at(const Token& tok, const std::string& message) {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "line(" << tok.get_lineno() << "): ERROR: ";
    auto kind = tok.get_kind();
    if (kind == TokenKind::TK_EOF) {
      std::cerr << "at end";
    }
    else if (kind == TokenKind::TK_ERROR) {
      // do nothing
    }
    else {
      std::cerr << "at " << tok.get_literal();
    }
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
  }

  void error_at_current(const std::string& message) {
    error_at(curr_, message);
  }

  void error(const std::string& message) {
    error_at(prev_, message);
  }

  OpCode make_constant(const Value& value) {
    auto constant = compiling_chunk_.add_constant(value);
    if (static_cast<std::uint8_t>(constant) > UINT8_MAX) {
      error("too many constants in one chunk");
      return EnumUtil<OpCode>::as_enum(0);
    }
    return constant;
  }

  void emit_code(OpCode code) {
    compiling_chunk_.write(code, prev_.get_lineno());
  }

  void emit_codes(OpCode code1, OpCode code2) {
    emit_code(code1);
    emit_code(code2);
  }

  void emit_codes(const std::initializer_list<OpCode>& codes) {
    for (auto c : codes)
      emit_code(c);
  }

  void emit_return(void) {
    emit_code(OpCode::OP_RETURN);
  }

  void emit_constant(const Value& v) {
    emit_codes(OpCode::OP_CONSTANT, make_constant(v));
  }

  const ParseRule& get_rule(TokenKind kind) const {
    static auto numeric_fn = [](const ParserPtr& p, bool) { p->numeric(); };
    static auto string_fn = [](const ParserPtr& p, bool) { p->string(); };
    static auto variable_fn = [](const ParserPtr& p, bool b) { p->variable(b); };
    static auto literal_fn = [](const ParserPtr& p, bool) { p->literal(); };
    static auto binary_fn = [](const ParserPtr& p, bool) { p->binary(); };
    static auto unary_fn = [](const ParserPtr& p, bool) { p->unary(); };
    static auto grouping_fn = [](const ParserPtr& p, bool) { p->grouping(); };

    static const ParseRule _rules[] = {
      {nullptr, nullptr, Precedence::NONE}, // TK_ERROR
      {nullptr, nullptr, Precedence::NONE}, // TK_EOF
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
      {literal_fn, nullptr, Precedence::NONE}, // KW_FALSE
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
    };

    return _rules[EnumUtil<TokenKind>::as_int(kind)];
  }

  void parse_precedence(Precedence prec) {
    advance();

    auto prefix_rule = get_rule(prev_.get_kind()).prefix;
    if (!prefix_rule) {
      error("expect expression");
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
      error("invalid assignment target");
      expression();
    }
  }

  void synchronize(void) {
    panic_mode_ = false;
    while (curr_.get_kind() == TokenKind::TK_EOF) {
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
      default: break;
      }
      advance();
    }
  }

  OpCode parse_variable(const std::string& message) {
    consume(TokenKind::TK_IDENTIFIER, message);
    return identifier_constant(prev_);
  }

  OpCode identifier_constant(const Token& name) {
    return make_constant(Object::create_string(name.get_literal()));
  }

  void define_variable(OpCode global) {
    emit_codes(OpCode::OP_DEFINE_GLOBAL, global);
  }
public:
  Parser(Chunk& c, Scanner& s, ParseCompiler& p)
    : compiling_chunk_(c), scanner_(s), curr_compiler_(p) {}
  bool had_error(void) const { return had_error_; }

  void advance(void) {
    prev_ = curr_;

    for (;;) {
      curr_ = scanner_.scan_token();
      if (curr_.get_kind() != TokenKind::TK_ERROR)
        break;

      error_at_current(curr_.get_literal());
    }
  }

  void consume(TokenKind kind, const std::string& message) {
    if (curr_.get_kind() == kind)
      advance();
    else
      error_at_current(message);
  }

  bool check(TokenKind kind) const {
    return curr_.get_kind() == kind;
  }

  bool match(TokenKind kind) {
    if (check(kind)) {
      advance();
      return true;
    }
    return false;
  }

  void end_compiler(void) {
    emit_return();
#if defined(LVM_TRACE_CODE)
    if (!had_error())
      compiling_chunk_.disassemble("code");
#endif
  }

  void binary(void) {
    // remember the operator
    auto oper_kind = prev_.get_kind();

    // compile the right operand
    const auto& rule = get_rule(oper_kind);
    parse_precedence(EnumUtil<Precedence>::as_enum(
        EnumUtil<Precedence>::as_int(rule.precedence) + 1));

    // emit the operator instruction
    switch (oper_kind) {
    case TokenKind::TK_EQUALEQUAL: emit_code(OpCode::OP_EQ); break;
    case TokenKind::TK_BANGEQUAL: emit_code(OpCode::OP_NE); break;
    case TokenKind::TK_GREATER: emit_code(OpCode::OP_GT); break;
    case TokenKind::TK_GREATEREQUAL: emit_code(OpCode::OP_GE); break;
    case TokenKind::TK_LESS: emit_code(OpCode::OP_LT); break;
    case TokenKind::TK_LESSEQUAL: emit_code(OpCode::OP_LE); break;
    case TokenKind::TK_PLUS: emit_code(OpCode::OP_ADD); break;
    case TokenKind::TK_MINUS: emit_code(OpCode::OP_SUB); break;
    case TokenKind::TK_STAR: emit_code(OpCode::OP_MUL); break;
    case TokenKind::TK_SLASH: emit_code(OpCode::OP_DIV); break;
    default: return; // unreachable
    }
  }

  void declaration(void) {
    if (match(TokenKind::KW_VAR))
      var_decl();
    else
      statement();

    if (panic_mode_)
      synchronize();
  }

  void var_decl(void) {
    OpCode global = parse_variable("expect variable name");

    if (match(TokenKind::TK_EQUAL))
      expression();
    else
      emit_code(OpCode::OP_NIL);
    consume(TokenKind::TK_SEMI, "expect `;` after variable declaration");

    define_variable(global);
  }

  void statement(void) {
    if (match(TokenKind::KW_PRINT))
      print_stmt();
    else
      expr_stmt();
  }

  void print_stmt(void) {
    expression();
    consume(TokenKind::TK_SEMI, "expect `;` after value");
    emit_code(OpCode::OP_PRINT);
  }

  void expr_stmt(void) {
    expression();
    emit_code(OpCode::OP_POP);
    consume(TokenKind::TK_SEMI, "expect `;` after expression");
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT);
  }

  void literal(void) {
    switch (prev_.get_kind()) {
    case TokenKind::KW_NIL: emit_code(OpCode::OP_NIL); break;
    case TokenKind::KW_TRUE: emit_code(OpCode::OP_TRUE); break;
    case TokenKind::KW_FALSE: emit_code(OpCode::OP_FALSE); break;
    default: return; // unreachable
    }
  }

  void numeric(void) {
    emit_constant(prev_.as_numeric());
  }

  void string(void) {
    emit_constant(Object::create_string(prev_.get_literal()));
  }

  void variable(bool can_assign) {
    named_varibale(prev_, can_assign);
  }

  void named_varibale(const Token& name, bool can_assign) {
    OpCode arg = identifier_constant(name);

    if (can_assign && match(TokenKind::TK_EQUAL)) {
      expression();
      emit_codes(OpCode::OP_SET_GLOBAL, arg);
    }
    else {
      emit_codes(OpCode::OP_GET_GLOBAL, arg);
    }
  }

  void grouping(void) {
    expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
  }

  void unary(void) {
    auto oper_kind = prev_.get_kind();
    parse_precedence(Precedence::UNARY);
    switch (oper_kind) {
    case TokenKind::TK_BANG: emit_code(OpCode::OP_NOT); break;
    case TokenKind::TK_MINUS: emit_code(OpCode::OP_NEGATE); break;
    default: return; // unreachable
    }
  }
};

bool Compiler::compile(Chunk& chunk, const std::string& source_bytes) {
#if defined(LVM_TRACE_SCANNING)
  {
    Scanner s(source_bytes);
    for (;;) {
      Token t = s.scan_token();
      std::cout << t << std::endl;

      if (t.get_kind() == TokenKind::TK_EOF)
        break;
    }
  }
#endif

  Scanner scanner(source_bytes);
  ParseCompiler compiler;
  auto p = std::make_shared<Parser>(chunk, scanner, compiler);

  p->advance();

  while (!p->match(TokenKind::TK_EOF))
    p->declaration();
  p->end_compiler();

  return !p->had_error();
}

}
