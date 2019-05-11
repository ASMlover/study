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
#include "lexer.hh"
#include "compiler.hh"

namespace wrencc {

class Compiler;
using CompileFn = std::function<void (Compiler*, const Token&)>;

enum class Precedence {
  NONE,
  LOWEST,

  EQUALITY, // == !=
  COMPARISON, // < <= > >=
  BITWISE, // | &
  TERM, // + -
  FACTOR, // * / %
  CALL, // ()
};

struct InfixCompiler {
  CompileFn fn;
  Precedence precedence;
};

class Compiler : private UnCopyable {
  Lexer& lex_;
  Token prev_;
  Token curr_;

  // the block being comipled
  Block* block_{};
  int codes_count_{};

  bool had_error_{};

  void error(const char* format, ...) {
    had_error_ = true;
    std::cerr << "Compile ERROR on " << curr_ << " : ";

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    std::cerr << std::endl;
  }

  template <typename T> inline void emit_byte(T b) {
    block_->add_code(b);
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) {
    emit_byte(b1);
    emit_byte(b2);
  }

  inline void emit_constant(Value v) {
    u8_t b = block_->add_constant(v);
    emit_bytes(Code::CONSTANT, b);
  }

  CompileFn& get_prefix(TokenKind kind) {
    auto prefix_fn = [](Compiler* p, const Token& t) { p->prefix_literal(t); };

    static CompileFn _prefixs[] = {
      nullptr, // PUNCTUATOR(LPAREN, "(")
      nullptr, // PUNCTUATOR(RPAREN, ")")
      nullptr, // PUNCTUATOR(LBRACKET, "[")
      nullptr, // PUNCTUATOR(RBRACKET, "]")
      nullptr, // PUNCTUATOR(LBRACE, "{")
      nullptr, // PUNCTUATOR(RBRACE, "}")
      nullptr, // PUNCTUATOR(COLON, ":")
      nullptr, // PUNCTUATOR(DOT, ".")
      nullptr, // PUNCTUATOR(COMMA, ",")
      nullptr, // PUNCTUATOR(STAR, "*")
      nullptr, // PUNCTUATOR(SLASH, "/")
      nullptr, // PUNCTUATOR(PERCENT, "%")
      nullptr, // PUNCTUATOR(PLUS, "+")
      nullptr, // PUNCTUATOR(MINUS, "-")
      nullptr, // PUNCTUATOR(PIPE, "|")
      nullptr, // PUNCTUATOR(AMP, "&")
      nullptr, // PUNCTUATOR(BANG, "!")
      nullptr, // PUNCTUATOR(EQ, "=")
      nullptr, // PUNCTUATOR(LT, "<")
      nullptr, // PUNCTUATOR(GT, ">")
      nullptr, // PUNCTUATOR(LTEQ, "<=")
      nullptr, // PUNCTUATOR(GTEQ, ">=")
      nullptr, // PUNCTUATOR(EQEQ, "==")
      nullptr, // PUNCTUATOR(BANGEQ, "!=")
      nullptr, // KEYWORD(ELSE, "else")
      nullptr, // KEYWORD(IF, "if")
      nullptr, // KEYWORD(VAR, "var")
      nullptr, // TOKEN(EMBED, "embed")
      prefix_fn, // TOKEN(IDENTIFIER, "identifier")
      prefix_fn, // TOKEN(NUMERIC, "numeric")
      prefix_fn, // TOKEN(STRING, "string")
      nullptr, // TOKEN(NL, "new-line")
      nullptr, // TOKEN(ERROR, "error")
      nullptr, // TOKEN(EOF, "eof")
    };
    return _prefixs[Xt::as_type<int>(kind)];
  }

  InfixCompiler& get_infix(TokenKind kind) {
    auto call_fn = [](Compiler* p, const Token& t) { p->infix_call(t); };
    auto binary_fn = [](Compiler* p, const Token& t) { p->infix_binary(t); };

    static InfixCompiler _infixs[] = {
      {call_fn, Precedence::CALL}, // PUNCTUATOR(LPAREN, "(")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(RPAREN, ")")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(LBRACKET, "[")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(RBRACKET, "]")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(LBRACE, "{")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(RBRACE, "}")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(COLON, ":")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(DOT, ".")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(COMMA, ",")
      {binary_fn, Precedence::FACTOR}, // PUNCTUATOR(STAR, "*")
      {binary_fn, Precedence::FACTOR}, // PUNCTUATOR(SLASH, "/")
      {binary_fn, Precedence::FACTOR}, // PUNCTUATOR(PERCENT, "%")
      {binary_fn, Precedence::TERM}, // PUNCTUATOR(PLUS, "+")
      {binary_fn, Precedence::TERM}, // PUNCTUATOR(MINUS, "-")
      {binary_fn, Precedence::BITWISE}, // PUNCTUATOR(PIPE, "|")
      {binary_fn, Precedence::BITWISE}, // PUNCTUATOR(AMP, "&")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(BANG, "!")
      {nullptr, Precedence::NONE}, // PUNCTUATOR(EQ, "=")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(LT, "<")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(GT, ">")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(LTEQ, "<=")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(GTEQ, ">=")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(EQEQ, "==")
      {binary_fn, Precedence::COMPARISON}, // PUNCTUATOR(BANGEQ, "!=")
      {nullptr, Precedence::NONE}, // KEYWORD(ELSE, "else")
      {nullptr, Precedence::NONE}, // KEYWORD(IF, "if")
      {nullptr, Precedence::NONE}, // KEYWORD(VAR, "var")
      {nullptr, Precedence::NONE}, // TOKEN(EMBED, "embed")
      {nullptr, Precedence::NONE}, // TOKEN(IDENTIFIER, "identifier")
      {nullptr, Precedence::NONE}, // TOKEN(NUMERIC, "numeric")
      {nullptr, Precedence::NONE}, // TOKEN(STRING, "string")
      {nullptr, Precedence::NONE}, // TOKEN(NL, "new-line")
      {nullptr, Precedence::NONE}, // TOKEN(ERROR, "error")
      {nullptr, Precedence::NONE}, // TOKEN(EOF, "eof")
    };
    return _infixs[Xt::as_type<int>(kind)];
  }

  void compile_precedence(Precedence precedence) {
    advance();

    auto prefix = get_prefix(prev_.kind());
    if (!prefix) {
      error("no prefix parser");
      std::exit(1);
    }
    prefix(this, prev_);

    while (precedence <= get_infix(curr_.kind()).precedence) {
      advance();
      auto infix = get_infix(prev_.kind()).fn;
      infix(this, prev_);
    }
  }

  void prefix_literal(const Token& tok) {
    Value constant = make_value(tok.as_numeric());

    emit_constant(constant);
  }

  void infix_call(const Token& tok) {
    std::cerr << "infix calls not implemented" << std::endl;
    std::exit(1);
  }

  void infix_binary(const Token& tok) {
    std::cerr << "infix binary operations not implemented" << std::endl;
    std::exit(1);
  }

  bool match(TokenKind expected) {
    if (curr_.kind() != expected)
      return false;

    advance();
    return true;
  }

  void consume(TokenKind expected) {
    advance();

    if (prev_.kind() != expected)
      error("expected %d, got %d", expected, prev_.kind());
  }

  void advance(void) {
    prev_ = curr_;
    curr_ = lex_.next_token();
  }

  void statement(void) {
    expression();
    consume(TokenKind::TK_NL);
  }

  void expression(void) {
    compile_precedence(Precedence::LOWEST);
  }
public:
  Compiler(Lexer& lex) noexcept
    : lex_(lex) {
    block_ = new Block();
  }

  ~Compiler(void) {
    // FIXME: fixed allocated Block
    //
    // if (block_ != nullptr)
    //   delete block_;
  }

  Block* run_compiler(void) {
    advance();
    do {
      statement();
    } while (!match(TokenKind::TK_EOF));
    emit_byte(Code::END);

    return had_error_ ? nullptr : block_;
  }
};

Block* compile(const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Compiler c(lex);

  return c.run_compiler();
}

}
