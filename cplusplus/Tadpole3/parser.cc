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
#include "string_object.hh"
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
  prev_ = curr_;

  for (;;) {
    curr_ = lex_.next_token();
    if (!check(TokenKind::TK_ERR))
      break;

    error_at_current(curr_.as_string());
  }
}

void GlobalParser::consume(TokenKind kind, const str_t& msg) {
  if (check(kind))
    advance();
  else
    error_at_current(msg);
}

bool GlobalParser::match(TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

void GlobalParser::init_compiler(FunCompiler* compiler, int scope_depth, FunType fn_type) {
  StringObject* func_name{};
  if (fn_type == FunType::FUNCTION)
    func_name = StringObject::create(vm_, prev_.as_string());

  compiler->set_compiler(
      curr_compiler_,
      FunctionObject::create(vm_, func_name),
      fn_type,
      scope_depth);
  curr_compiler_ = compiler;

  curr_compiler_->append_local(LocalVar(Token::make(""), curr_compiler_->scope_depth(), false));
}

FunctionObject* GlobalParser::finish_compiler() {
  emit_return();

  FunctionObject* fn = curr_compiler_->fn();

#if defined(_TADPOLE_DEBUG_VM)
  if (!had_error_)
    curr_chunk()->dis(fn->name_asstr());
#endif

  curr_compiler_ = curr_compiler_->enclosing();
  return fn;
}

void GlobalParser::enter_scope() {
  curr_compiler_->enter_scope();
}

void GlobalParser::leave_scope() {
  curr_compiler_->leave_scope([this](const LocalVar& var) {
        emit_byte(var.is_upvalue ? Code::CLOSE_UPVALUE : Code::POP);
      });
}

u8_t GlobalParser::identifier_constant(const Token& name) noexcept {
  return curr_chunk()->add_constant(StringObject::create(vm_, name.as_string()));
}

u8_t GlobalParser::parse_variable(const str_t& msg) {
  consume(TokenKind::TK_IDENTIFIER, msg);

  curr_compiler_->declare_localvar(prev_, [this](const str_t& m) { error(m); });
  if (curr_compiler_->scope_depth() > 0)
    return 0;
  return identifier_constant(prev_);
}

void GlobalParser::mark_initialized() {
  if (curr_compiler_->scope_depth() == 0)
    return;
  curr_compiler_->peek_local().depth = curr_compiler_->scope_depth();
}

void GlobalParser::define_global(u8_t global) {
  if (curr_compiler_->scope_depth() > 0) {
    mark_initialized();
    return;
  }
  emit_bytes(Code::DEF_GLOBAL, global);
}

u8_t GlobalParser::arguments() {
  u8_t argc = 0;
  if (!check(TokenKind::TK_RPAREN)) {
    do {
      expression();
      ++argc;

      if (argc > kMaxArgs)
        error(from_fmt("cannot have more than `%d` arguments", kMaxArgs));
    } while (match(TokenKind::TK_COMMA));
  }
  consume(TokenKind::TK_RPAREN, "expect `;` after function arguments");

  return argc;
}

void GlobalParser::named_variable(const Token& name, bool can_assign) {
  auto errfn = [this](const str_t& msg) { error(msg); };

  Code getop, setop;
  int arg = curr_compiler_->resolve_local(name, errfn);
  if (arg != -1) {
    getop = Code::GET_LOCAL;
    setop = Code::SET_LOCAL;
  }
  else if (arg = curr_compiler_->resolve_upvalue(name, errfn); arg != -1) {
    getop = Code::GET_UPVALUE;
    setop = Code::SET_UPVALUE;
  }
  else {
    arg = identifier_constant(name);
    getop = Code::GET_GLOBAL;
    setop = Code::SET_GLOBAL;
  }

  if (can_assign && match(TokenKind::TK_EQ)) {
    expression();
    emit_bytes(setop, arg);
  }
  else {
    emit_bytes(getop, arg);
  }
}

void GlobalParser::parse_precedence(Precedence precedence) {}

void GlobalParser::binary(bool can_assign) {
  TokenKind op = prev_.kind();

  parse_precedence(get_rule(op).precedence + 1);
  switch (op) {
    case TokenKind::TK_PLUS: emit_byte(Code::ADD); break;
    case TokenKind::TK_MINUS: emit_byte(Code::SUB); break;
    case TokenKind::TK_STAR: emit_byte(Code::MUL); break;
    case TokenKind::TK_SLASH: emit_byte(Code::DIV); break;
  }
}

void GlobalParser::call(bool can_assign) {
  emit_byte(Code::CALL_0 + arguments());
}

void GlobalParser::grouping(bool can_assign) {
  expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after grouping expression");
}

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
