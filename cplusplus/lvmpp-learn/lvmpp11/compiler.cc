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
  std::function<void (const ParserPtr&)> prefix;
  std::function<void (const ParserPtr&)> infix;
  Precedence precedence;
};

class Parser
  : private UnCopyable, public std::enable_shared_from_this<Parser> {
  Chunk& compiling_chunk_;
  Scanner& scanner_;
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
    static auto numeric_fn = [](const ParserPtr& p) { p->numeric(); };
    static auto literal_fn = [](const ParserPtr& p) { p->literal(); };
    static auto binary_fn = [](const ParserPtr& p) { p->binary(); };
    static auto unary_fn = [](const ParserPtr& p) { p->unary(); };
    static auto grouping_fn = [](const ParserPtr& p) { p->grouping(); };

    static const ParseRule _rules[] = {
      {nullptr, nullptr, Precedence::NONE}, // TK_ERROR
      {nullptr, nullptr, Precedence::NONE}, // TK_EOF
      {nullptr, nullptr, Precedence::NONE}, // TK_IDENTIFIER
      {numeric_fn, nullptr, Precedence::NONE}, // TK_NUMERICCONST
      {nullptr, nullptr, Precedence::NONE}, // TK_STRINGLITERAL
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
    prefix_rule(shared_from_this());

    while (prec <= get_rule(curr_.get_kind()).precedence) {
      advance();
      auto infix_rule = get_rule(prev_.get_kind()).infix;
      infix_rule(shared_from_this());
    }
  }
public:
  Parser(Chunk& c, Scanner& s) : compiling_chunk_(c), scanner_(s) {}
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

  void literal(void) {
    switch (prev_.get_kind()) {
    case TokenKind::KW_NIL: emit_code(OpCode::OP_NIL); break;
    case TokenKind::KW_TRUE: emit_code(OpCode::OP_TRUE); break;
    case TokenKind::KW_FALSE: emit_code(OpCode::OP_FALSE); break;
    default: return; // unreachable
    }
  }

  void expression(void) {
    parse_precedence(Precedence::ASSIGNMENT);
  }

  void numeric(void) {
    emit_constant(prev_.as_numeric());
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
  auto p = std::make_shared<Parser>(chunk, scanner);

  p->advance();
  p->expression();
  p->consume(TokenKind::TK_EOF, "expect end of expression");
  p->end_compiler();

  return !p->had_error();
}

}
