// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include "common.hh"
#include "vm.hh"
#include "token.hh"
#include "scanner.hh"
#include "chunk.hh"
#include "compiler.hh"

namespace clox {

enum class Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY,
};

template <typename N> inline Precedence operator+(Precedence a, N b) noexcept {
  return as_type<Precedence>(as_type<int>(a) + as_type<int>(b));
}

class Parser;

struct ParseRule {
  using ParseFn = std::function<void (Parser& parser)>;

  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

class Parser final : private UnCopyable {
  VM& vm_;
  Scanenr& scanner_;
  Chunk& chunk_;
  Token previous_;
  Token current_;
  bool had_error_{};
  bool panic_mode_{};

  void advance() {
    previous_ = current_;

    for (;;) {
      current_ = scanner_.scan_token();
      if (current_.type() != TokenType::TOKEN_ERROR)
        break;

      error_at_current(current_.as_string());
    }
  }

  void consume(TokenType type, const str_t& message) {
    if (current_.type() == type) {
      advance();
      return;
    }

    error_at_current(message);
  }

  inline Chunk* curr_chunk() noexcept {
    return &chunk_;
  }

  inline void error_at_current(const str_t& message) noexcept { error_at(current_, message); }
  inline void error(const str_t& message) noexcept { error_at(previous_, message); }

  void error_at(const Token& token, const str_t& message) noexcept {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "[line " << token.lineno() << "] Error";
    if (token.type() == TokenType::TOKEN_EOF) {
      std::cerr << " at end";
    }
    else if (token.type() == TokenType::TOKEN_ERROR) {
    }
    else {
      std::cerr << " at `" << token.as_string() << "`";
    }
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
  }

  inline void emit_return() noexcept { emit_byte(OpCode::OP_RETURN); }

  template <typename T> inline void emit_byte(T byte) noexcept { curr_chunk()->write(byte, previous_.lineno()); }
  template <typename T, typename U> inline void emit_bytes(T byte1, U byte2) noexcept {
    emit_byte(byte1);
    emit_byte(byte2);
  }

  inline void emit_constant(Value value) noexcept {
    emit_bytes(OpCode::OP_CONSTANT, curr_chunk()->add_constant(value));
  }

  inline void end_compiler() noexcept {
    emit_return();
  }

  inline void binary() noexcept {
    TokenType operator_type = previous_.type();

    const ParseRule& rule = get_rule(operator_type);
    parse_precedence(rule.precedence + 1);

    switch (operator_type) {
    case TokenType::TOKEN_PLUS: emit_byte(OpCode::OP_ADD); break;
    case TokenType::TOKEN_MINUS: emit_byte(OpCode::OP_SUBTRACT); break;
    case TokenType::TOKEN_STAR: emit_byte(OpCode::OP_MULTIPLY); break;
    case TokenType::TOKEN_SLASH: emit_byte(OpCode::OP_DIVIDE); break;
    default: return; // unreachable
    }
  }

  inline void grouping() noexcept {
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "expect `)` after expression");
  }

  inline void number() noexcept {
    double value = previous_.as_numeric();
    emit_constant(value);
  }

  inline void unary() noexcept {
    TokenType operator_type = previous_.type();

    // compile the operand
    parse_precedence(Precedence::PREC_UNARY);

    // emit the operator instruction
    switch (operator_type) {
    case TokenType::TOKEN_MINUS: emit_byte(OpCode::OP_NEGATE); break;
    default: return; // unreachable
    }
  }

  const ParseRule& get_rule(TokenType type) const noexcept {
#define _RULE(fn) [](Parser& p) { p.fn(); }
    static const ParseRule _rules[] = {
      {_RULE(grouping), nullptr, Precedence::PREC_NONE},      // PUNCTUATOR(LEFT_PAREN, "(")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(RIGHT_PAREN, ")")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(LEFT_BRACE, "{")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(RIGHT_BRACE, "}")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(COMMA, ",")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(DOT, ".")
      {_RULE(unary), _RULE(binary), Precedence::PREC_TERM},   // PUNCTUATOR(MINUS, "-")
      {_RULE(unary), _RULE(binary), Precedence::PREC_TERM},   // PUNCTUATOR(PLUS, "+")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(SEMICOLON, ";")
      {nullptr, _RULE(binary), Precedence::PREC_FACTOR},      // PUNCTUATOR(SLASH, "/")
      {nullptr, _RULE(binary), Precedence::PREC_FACTOR},      // PUNCTUATOR(STAR, "*")

      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(BANG, "!")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(BANG_EQUAL, "!=")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(EQUAL, "=")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(EQUAL_EQUAL, "==")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(GREATER, ">")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(GREATER_EQUAL, ">=")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(LESS, "<")
      {nullptr, nullptr, Precedence::PREC_NONE},              // PUNCTUATOR(LESS_EQUAL, "<=")

      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(IDENTIFIER, "Token-Identifier")
      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(STRING, "Token-String")
      {_RULE(number), nullptr, Precedence::PREC_NONE},        // TOKEN(NUMBER, "Token-Number")

      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(AND, "and")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(CLASS, "class")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(ELSE, "else")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(FALSE, "false")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(FOR, "for")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(FUN, "fun")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(IF, "if")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(NIL, "nil")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(OR, "or")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(PRINT, "print")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(RETURN, "return")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(SUPER, "super")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(THIS, "this")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(TRUE, "true")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(VAR, "var")
      {nullptr, nullptr, Precedence::PREC_NONE},              // KEYWORD(WHILE, "while")

      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(ERROR, "Token-Error")
      {nullptr, nullptr, Precedence::PREC_NONE},              // TOKEN(EOF, "Token-Eof")
    };
#undef _RULE

    return _rules[as_type<int>(type)];
  }

  void parse_precedence(Precedence precedence) noexcept {
    advance();
    auto prefix_rule = get_rule(previous_.type()).prefix;
    if (!prefix_rule) {
      error("expect expression");
      return;
    }

    prefix_rule(*this);
  }

  void expression() noexcept {
    parse_precedence(Precedence::PREC_ASSIGNMENT);
  }
public:
  Parser(VM& vm, Scanenr& scanner, Chunk& chunk) noexcept : vm_{vm}, scanner_{scanner}, chunk_{chunk} {}

  bool compile() {
    advance();
    expression();
    consume(TokenType::TOKEN_EOF, "expect end of expression");

    return !had_error_;
  }
};

void Compiler::compile(VM& vm, const str_t& source) noexcept {
  Scanenr scanner(source);
  Chunk chunk;

  if (parser_ = new Parser{vm, scanner, chunk}; parser_ != nullptr) {
    parser_->compile();
    delete parser_;
    parser_ = nullptr;
  }
}

}
