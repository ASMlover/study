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
#include "chunk.h"
#include "scanner.h"
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
  FACTOR, // * / %
  UNARY, // ! - +
  CALL, // . () []
  PRIMARY
};

struct ParseRule {
  std::function<void ()> prefix;
  std::function<void ()> infix;
  Precedence precedence;
};

class Parser
  : private UnCopyable, public std::enable_shared_from_this<Parser> {
  Chunk& compiling_chunk_;
  Scanner& scan_;
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

  void parse_precedence(Precedence prec) {
    advance();
    auto prefix_rule = get_rule(prev_.get_kind()).prefix;
    if (prefix_rule == nullptr) {
      error("expect expression ...");
      return;
    }
    prefix_rule();

    while (prec <= get_rule(curr_.get_kind()).precedence) {
      advance();
      auto infix_rule = get_rule(prev_.get_kind()).infix;
      infix_rule();
    }
  }

  ParseRule& get_rule(TokenKind kind) {
#define BFN(fn) std::bind(&Parser::fn, this)

    static ParseRule rules[] = {
      {nullptr, nullptr, Precedence::NONE}, // TK_ERROR
      {nullptr, nullptr, Precedence::NONE}, // TK_EOF
      {nullptr, nullptr, Precedence::NONE}, // TK_UNKNOWN
      {nullptr, nullptr, Precedence::NONE}, // TK_IDENTIFIER
      {BFN(numeric), nullptr, Precedence::NONE}, // TK_NUMERICCONST
      {nullptr, nullptr, Precedence::NONE}, // TK_STRINGLITERAL
      {BFN(grouping), nullptr, Precedence::CALL}, // TK_LPAREN
      {nullptr, nullptr, Precedence::NONE}, // TK_RPAREN
      {nullptr, nullptr, Precedence::NONE}, // TK_LBRACE
      {nullptr, nullptr, Precedence::NONE}, // TK_RBRACE
      {nullptr, nullptr, Precedence::NONE}, // TK_COMMA
      {nullptr, nullptr, Precedence::CALL}, // TK_DOT
      {nullptr, nullptr, Precedence::NONE}, // TK_SEMI
      {nullptr, BFN(binary), Precedence::TERM}, // TK_PLUS
      {BFN(unary), BFN(binary), Precedence::TERM}, // TK_MINUS
      {nullptr, BFN(binary), Precedence::FACTOR}, // TK_STAR
      {nullptr, BFN(binary), Precedence::FACTOR}, // TK_SLASH
      {nullptr, BFN(binary), Precedence::FACTOR}, // TK_PERCENT
      {nullptr, nullptr, Precedence::NONE}, // TK_BANG
      {nullptr, nullptr, Precedence::EQUALITY}, // TK_BANGEQUAL
      {nullptr, nullptr, Precedence::NONE}, // TK_EQUAL
      {nullptr, nullptr, Precedence::EQUALITY}, // TK_EQUALEQUAL
      {nullptr, nullptr, Precedence::COMPARISON}, // TK_GREATER
      {nullptr, nullptr, Precedence::COMPARISON}, // TK_GREATEREQUAL
      {nullptr, nullptr, Precedence::COMPARISON}, // TK_LESS
      {nullptr, nullptr, Precedence::COMPARISON}, // TK_LESSEQUAL
      {nullptr, nullptr, Precedence::AND}, // KW_AND
      {nullptr, nullptr, Precedence::NONE}, // KW_CLASS
      {nullptr, nullptr, Precedence::NONE}, // KW_ELSE
      {nullptr, nullptr, Precedence::NONE}, // KW_FLASE
      {nullptr, nullptr, Precedence::NONE}, // KW_FOR
      {nullptr, nullptr, Precedence::NONE}, // KW_FUN
      {nullptr, nullptr, Precedence::NONE}, // KW_IF
      {nullptr, nullptr, Precedence::NONE}, // KW_NIL
      {nullptr, nullptr, Precedence::OR}, // KW_OR
      {nullptr, nullptr, Precedence::NONE}, // KW_PRINT
      {nullptr, nullptr, Precedence::NONE}, // KW_RETURN
      {nullptr, nullptr, Precedence::NONE}, // KW_SUPER
      {nullptr, nullptr, Precedence::NONE}, // KW_THIS
      {nullptr, nullptr, Precedence::NONE}, // KW_TRUE
      {nullptr, nullptr, Precedence::NONE}, // KW_VAR
      {nullptr, nullptr, Precedence::NONE}, // KW_WHILE
      {nullptr, nullptr, Precedence::NONE}, // NUM_TOKENS
    };

    return rules[static_cast<int>(kind)];
#undef BFN
  }
public:
  Parser(Chunk& c, Scanner& s) : compiling_chunk_(c), scan_(s) {}
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
    // assignment     -> ( call "." )? IDENTIFILER "=" assignment | logic_or ;
    // logic_or       -> logic_and ( "or" logic_and )* ;
    // logic_and      -> equality ( "and" equality )* ;
    // equality       -> comparison ( ( "!=" | "==" ) comparison )* ;
    // comparison     -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
    // addition       -> multiplication ( ( "-" | "+" ) multiplication )* ;
    // multiplication -> unary ( ( "*" | "/" | "%" ) unary )* ;
    // unary          -> ( "!" | "-" ) unary | call ;
    // call           -> primary ( "(" arguments? ")" | "." IDENTIFILER )* ;
    // primary        -> "true" | "false" | "nil" | "this"
    //                | NUMERIC | STRING | IDENTIFILER | "(" expression ")"
    //                | "super" "." IDENTIFILER ;
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
    case TokenKind::TK_PERCENT: emit_byte(OpCode::OP_MODULO); break;
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
    case TokenKind::TK_MINUS:
      emit_byte(OpCode::OP_NEGATIVE); break;
    default:
      return; // unreachable
    }
  }
};

bool Compiler::compile(Chunk& chunk, const std::string& source_bytes) {
  Scanner s(source_bytes);

  // int lineno = -1;
  // for (;;) {
  //   Token token = s.scan_token();
  //   if (token.get_lineno() != lineno) {
  //     fprintf(stdout, "%4d ", token.get_lineno());
  //     lineno = token.get_lineno();
  //   }
  //   else {
  //     std::cout << "   | ";
  //   }
  //   fprintf(stdout, "%2d %-16s `%s`\n",
  //       token.get_kind(),
  //       get_token_name(token.get_kind()), token.get_literal().c_str());
  //   if (token.get_kind() == TokenKind::TK_EOF)
  //     break;
  // }

  auto p = std::make_shared<Parser>(chunk, s);
  p->advance();
  p->expression();
  p->consume(TokenKind::TK_EOF, "expect end of expression ...");
  p->end_compiler();

  return !p->had_error();
}

}
