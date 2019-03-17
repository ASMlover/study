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

  void parse_precedence(Precedence prec) {
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
    case TokenKind::TK_MINUS: emit_code(OpCode::OP_NEGATE); break;
    default:
      return; // unreachable
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
