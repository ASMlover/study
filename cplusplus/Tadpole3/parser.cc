// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <iostream>
#include "lexer.hh"
#include "vm.hh"
#include "parser.hh"

namespace tadpole {

GlobalParser::GlobalParser(VM& vm, Lexer& lex) noexcept
  : vm_(vm), lex_(lex) {
}

void GlobalParser::mark_parser() {
  for (FunCompiler* c = curr_compiler_; c != nullptr; c = c->enclosing())
    vm_.mark_object(c->fn());
}

FunctionObject* GlobalParser::compile() {
  FunCompiler compiler;
  init_compiler(&compiler, 0, FunType::TOPLEVEL);

  advance();
  while (!check(TokenKind::TK_EOF))
    declaration();
  FunctionObject* fn = finish_compiler();

  return had_error_ ? nullptr : fn;
}

const ParseRule& GlobalParser::get_rule(TokenKind kind) const noexcept {
#define _RULE(fn) [](GlobalParser& p, bool b) { p.fn(b); }

  static const ParseRule _rules[] = {
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(LPAREN, "(")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(RPAREN, ")")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(LBRACE, "{")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(RBRACE, "}")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(COMMA, ",")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(MINUS, "-")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(PLUS, "+")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(SEMI, ";")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(SLASH, "/")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(STAR, "*")
    {nullptr, nullptr, Precedence::NONE}, // PUNCTUATOR(EQ, "=")

    {nullptr, nullptr, Precedence::NONE}, // TOKEN(IDENTIFIER, "Identifier")
    {nullptr, nullptr, Precedence::NONE}, // TOKEN(NUMERIC, "Numeric")
    {nullptr, nullptr, Precedence::NONE}, // TOKEN(STRING, "String")

    {nullptr, nullptr, Precedence::NONE}, // KEYWORD(FALSE, "false")
    {nullptr, nullptr, Precedence::NONE}, // KEYWORD(FN, "fn")
    {nullptr, nullptr, Precedence::NONE}, // KEYWORD(NIL, "nil")
    {nullptr, nullptr, Precedence::NONE}, // KEYWORD(TRUE, "true")
    {nullptr, nullptr, Precedence::NONE}, // KEYWORD(VAR, "var")

    {nullptr, nullptr, Precedence::NONE}, // TOKEN(EOF, "Eof")
    {nullptr, nullptr, Precedence::NONE}, // TOKEN(ERR, "Error")
  };

#undef _RULE
  return _rules[as_type<int>(kind)];
}

void GlobalParser::error_at(const Token& tok, const str_t& msg) noexcept {
  if (panic_mode_)
    return;
  panic_mode_ = true;

  std::cerr
    << "SyntaxError:" << std::endl
    << "  [LINE: " << tok.lineno() << "] ERROR ";
  if (tok.kind() == TokenKind::TK_EOF)
    std::cerr << "at end ";
  else if (tok.kind() == TokenKind::TK_ERR)
    (void)0;
  else
    std::cerr << "at `" << tok.literal() << "` ";
  std::cerr << ": " << msg << std::endl;

  had_error_ = true;
}

void GlobalParser::advance() {
}

void GlobalParser::consume(TokenKind kind, const str_t& msg) {
}

bool GlobalParser::match(TokenKind kind) {
  return false;
}

void GlobalParser::init_compiler(FunCompiler* compiler, int scope_depth, FunType fn_type) {}
FunctionObject* GlobalParser::finish_compiler() { return nullptr; }
void GlobalParser::enter_scope() {}
void GlobalParser::leave_scope() {}
u8_t GlobalParser::identifier_constant(const Token& name) noexcept { return 0;}
u8_t GlobalParser::parse_variable(const str_t& msg) { return 0; }
void GlobalParser::mark_initialized() {}
void GlobalParser::define_global(u8_t global) {}
u8_t GlobalParser::arguments() { return 0; }
void GlobalParser::named_variable(const Token& name, bool can_assign) {}
void GlobalParser::parse_precedence(Precedence precedence) {}
void GlobalParser::binary(bool can_assign) {}
void GlobalParser::call(bool can_assign) {}
void GlobalParser::grouping(bool can_assign) {}
void GlobalParser::literal(bool can_assign) {}
void GlobalParser::variable(bool can_assign) {}
void GlobalParser::numeric(bool can_assign) {}
void GlobalParser::string(bool can_assign) {}
void GlobalParser::block() {}
void GlobalParser::function(FunType fn_type) {}
void GlobalParser::synchronize() {}
void GlobalParser::expression() {}
void GlobalParser::declaration() {}
void GlobalParser::statement() {}
void GlobalParser::fn_decl() {}
void GlobalParser::var_decl() {}
void GlobalParser::expr_stmt() {}

}
