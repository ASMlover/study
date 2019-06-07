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
#include "vm.hh"
#include "compiler.hh"

namespace wrencc {

class Compiler;
using ParseFn = std::function<void (Compiler*)>;

enum class Precedence {
  NONE,
  LOWEST,

  EQUALITY,   // == !=
  COMPARISON, // < <= > >=
  BITWISE,    // | &
  TERM,       // + -
  FACTOR,     // * / %
  CALL,       // ()
};

inline Precedence operator+(Precedence a, int b) {
  return Xt::as_type<Precedence>(Xt::as_type<int>(a) + b);
}

struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
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
      case TokenKind::TK_AMP:
      case TokenKind::TK_BANG:
      case TokenKind::TK_EQ:
      case TokenKind::TK_LT:
      case TokenKind::TK_GT:
      case TokenKind::TK_LTEQ:
      case TokenKind::TK_GTEQ:
      case TokenKind::TK_EQEQ:
      case TokenKind::TK_BANGEQ:
      case TokenKind::KW_CLASS:
      case TokenKind::KW_META:
      case TokenKind::KW_VAR:
        skip_newlines_ = true; return;
      default: skip_newlines_ = false; return;
      }
    }
  }
};

class Compiler : private UnCopyable {
  Parser& parser_;
  Compiler* parent_{};
  BlockObject* block_{};
  int num_codes_{};

  SymbolTable locals_;

  inline SymbolTable& vm_symbols(void) { return parser_.get_vm().symbols(); }
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
    return vm_symbols().ensure(parser_.prev().as_string());
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

  const ParseRule& get_rule(TokenKind kind) const {
    auto grouping_fn = [](Compiler* p) { p->grouping(); };
    auto block_fn = [](Compiler* p) { p->block(); };
    auto call_fn = [](Compiler* p) { p->call(); };
    auto infix_oper_fn = [](Compiler* p) { p->infix_oper(); };
    auto variable_fn = [](Compiler* p) { p->variable(); };
    auto numeric_fn = [](Compiler* p) { p->numeric(); };
    auto string_fn = [](Compiler* p) { p->string(); };

    static const ParseRule _rules[] = {
      {grouping_fn, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(LPAREN, "(")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(RPAREN, ")")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(LBRACKET, "[")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(RBRACKET, "]")
      {block_fn, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(LBRACE, "{")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(RBRACE, "}")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(COLON, ":")
      {nullptr, call_fn, Precedence::CALL, nullptr}, // PUNCTUATOR(DOT, ".")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(COMMA, ",")
      {nullptr, infix_oper_fn, Precedence::FACTOR, "* "}, // PUNCTUATOR(STAR, "*")
      {nullptr, infix_oper_fn, Precedence::FACTOR, "/ "}, // PUNCTUATOR(SLASH, "/")
      {nullptr, infix_oper_fn, Precedence::NONE, "% "}, // PUNCTUATOR(PERCENT, "%")
      {nullptr, infix_oper_fn, Precedence::TERM, "+ "}, // PUNCTUATOR(PLUS, "+")
      {nullptr, infix_oper_fn, Precedence::TERM, "- "}, // PUNCTUATOR(MINUS, "-")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(PIPE, "|")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(AMP, "&")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(BANG, "!")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(EQ, "=")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(LT, "<")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(GT, ">")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(LTEQ, "<=")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(GTEQ, ">=")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(EQEQ, "==")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // PUNCTUATOR(BANGEQ, "!=")

      {nullptr, nullptr, Precedence::NONE, nullptr}, // KEYWORD(CLASS, "class")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // KEYWORD(META, "meta")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // KEYWORD(VAR, "var")

      {variable_fn, nullptr, Precedence::NONE, nullptr}, // TOKEN(IDENTIFIER, "identifier")
      {numeric_fn, nullptr, Precedence::NONE, nullptr}, // TOKEN(NUMERIC, "numeric")
      {string_fn, nullptr, Precedence::NONE, nullptr}, // TOKEN(STRING, "string")

      {nullptr, nullptr, Precedence::NONE, nullptr}, // TOKEN(NL, "new-line")

      {nullptr, nullptr, Precedence::NONE, nullptr}, // TOKEN(ERROR, "error")
      {nullptr, nullptr, Precedence::NONE, nullptr}, // TOKEN(EOF, "eof")
    };

    return _rules[Xt::as_type<int>(kind)];
  }

  void parse_precedence(Precedence precedence) {
    parser_.advance();
    auto& prefix_fn = get_rule(parser_.prev().kind()).prefix;
    if (!prefix_fn) {
      error("no prefix parser");
      return;
    }

    prefix_fn(this);

    while (precedence <= get_rule(parser_.curr().kind()).precedence) {
      parser_.advance();

      auto& infix_fn = get_rule(parser_.prev().kind()).infix;
      if (infix_fn)
        infix_fn(this);
    }
  }

  int define_identifier(void) {
    consume(TokenKind::TK_IDENTIFIER);

    int symbol;
    str_t name = parser_.prev().as_string();
    if (parent_ != nullptr)
      symbol = locals_.add(name);
    else
      symbol = vm_gsymbols().add(name);

    if (symbol == -1)
      error("variable is already defined");
    return symbol;
  }

  void store_variable(int symbol) {
    emit_byte(parent_ != nullptr ? Code::STORE_LOCAL : Code::STORE_GLOBAL);
    emit_byte(symbol);
  }

  void numeric(void) {
    auto& tok = parser_.prev();
    Value constant = NumericObject::make_numeric(tok.as_numeric());

    emit_constant(constant);
  }

  void string(void) {
    auto& tok = parser_.prev();

    sz_t sz = tok.as_string().size();
    char* s = new char [sz + 1];
    memcpy(s, tok.as_string().c_str(), sz);
    s[sz] = 0;

    Value constant = StringObject::make_string(s);

    emit_constant(constant);
  }

  void variable(void) {
    int local = locals_.get(parser_.prev().as_string());
    if (local != -1) {
      emit_bytes(Code::LOAD_LOCAL, local);
      return;
    }

    int global = vm_gsymbols().get(parser_.prev().as_string());
    if (global != -1) {
      emit_bytes(Code::LOAD_GLOBAL, global);
      return;
    }

    error("undefined variable");
  }

  bool match(TokenKind expected) {
    if (parser_.curr().kind() != expected)
      return false;

    parser_.advance();
    return true;
  }

  void consume(TokenKind expected) {
    parser_.advance();

    if (parser_.prev().kind() != expected)
      error("expected %d, got %d", expected, parser_.prev().kind());
  }

  void statement(void) {
    if (match(TokenKind::KW_CLASS)) {
      int symbol = define_identifier();

      // create the empty class
      emit_byte(Code::CLASS);

      // store it in it's name
      store_variable(symbol);

      // compile the method definitions
      consume(TokenKind::TK_LBRACE);
      while (!match(TokenKind::TK_RBRACE)) {
        // method name
        consume(TokenKind::TK_IDENTIFIER);
        int symbol = intern_symbol();
        consume(TokenKind::TK_LBRACE);
        BlockObject* method = compile_block(parser_, this, TokenKind::TK_RBRACE);

        consume(TokenKind::TK_NL);

        // add the block to the constant table
        u8_t constant = block_->add_constant(method);

        // compile the code to define the method
        emit_byte(Code::METHOD);
        emit_byte(symbol);
        emit_byte(constant);
      }
      return;
    }

    if (match(TokenKind::KW_VAR)) {
      int symbol = define_identifier();

      // allow uninitialized vars
      consume(TokenKind::TK_EQ);

      // compile the initializer
      expression();

      store_variable(symbol);
      return;
    }

    expression();
  }

  void expression(void) {
    parse_precedence(Precedence::LOWEST);
  }

  void grouping(void) {
    expression();
    consume(TokenKind::TK_RPAREN);
  }

  void block(void) {
    BlockObject* block = compile_block(parser_, this, TokenKind::TK_RBRACE);
    u8_t constant = block_->add_constant(block);

    emit_bytes(Code::CONSTANT, constant);
  }

  void call(void) {
    str_t name;
    int argc = 0;

    consume(TokenKind::TK_IDENTIFIER);
    for (;;) {
      name += parser_.prev().as_string();
      if (match(TokenKind::TK_LPAREN)) {
        for (;;) {
          expression();
          ++argc;

          name.push_back(' ');
          if (!match(TokenKind::TK_COMMA))
            break;
        }
        consume(TokenKind::TK_RPAREN);

        // if there isn't another part name after the argument list, stop
        if (!match(TokenKind::TK_IDENTIFIER))
          break;
      }
      else {
        break;
      }
    }
    int symbol = vm_symbols().ensure(name);

    // compile the method call
    emit_bytes(Code::CALL_0 + argc, symbol);
  }

  void infix_oper(void) {
    auto& rule = get_rule(parser_.prev().kind());

    // compile the right hand side
    parse_precedence(rule.precedence + 1);

    // call the operator method on the left-hand side
    int symbol = vm_symbols().ensure(rule.name);
    emit_bytes(Code::CALL_1, symbol);
  }
public:
  Compiler(Parser& parser, Compiler* parent = nullptr) noexcept
    : parser_(parser)
    , parent_(parent) {
    block_ = BlockObject::make_block();
  }

  ~Compiler(void) {
    // FIXME: fixed deallocate BlockObject by GC
  }

  BlockObject* compile_block(TokenKind end_kind) {
    for (;;) {
      statement();

      if (!match(TokenKind::TK_NL))
        break;
      if (match(end_kind))
        break;
      // emit_byte(Code::POP);
    }
    emit_byte(Code::END);
    block_->set_num_locals(locals_.count());

    return parser_.had_error() ? nullptr : block_;
  }

  BlockObject* compile_block(Parser& p, Compiler* parent, TokenKind end_kind) {
    Compiler c(p, parent);
    return c.compile_block(end_kind);
  }
};

BlockObject* compile(VM& vm, const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Parser p(vm, lex);

  p.advance();
  Compiler c(p, nullptr);

  return c.compile_block(TokenKind::TK_EOF);
}

}
