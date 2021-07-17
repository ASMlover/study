// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include "colorful.hh"
#include "lexer.hh"
#include "string_object.hh"
#include "vm.hh"
#include "parser.hh"

namespace tadpole {

GlobalParser::GlobalParser(TadpoleVM& vm, Lexer& lex) noexcept
  : vm_(vm), lex_(lex) {
}

void GlobalParser::iter_objects(ObjectVisitor&& visitor) {
  for (FnCompiler* c = curr_compiler_; c != nullptr; c = c->enclosing())
    visitor(c->fn());
}

FunctionObject* GlobalParser::compile() {
  FnCompiler compiler;
  init_compiler(&compiler, 0, FnType::TOPLEVEL);

  advance();
  while (!check(TokenKind::TK_EOF))
    declaration();
  FunctionObject* fn = finish_compiler();

  return had_error_ ? nullptr : fn;
}

const ParseRule& GlobalParser::get_rule(TokenKind kind) const noexcept {
#define _RULE(fn) [](GlobalParser& p, bool b) { p.fn(b); }
  static const ParseRule _rules[] = {
    {_RULE(grouping), _RULE(call), Precedence::CALL}, // PUNCTUATOR(LPAREN, "(")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(RPAREN, ")")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(LBRACE, "{")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(RBRACE, "}")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(COMMA, ",")
    {nullptr, _RULE(binary), Precedence::TERM},       // PUNCTUATOR(MINUS, "-")
    {nullptr, _RULE(binary), Precedence::TERM},       // PUNCTUATOR(PLUS, "+")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(SEMI, ";")
    {nullptr, _RULE(binary), Precedence::FACTOR},     // PUNCTUATOR(SLASH, "/")
    {nullptr, _RULE(binary), Precedence::FACTOR},     // PUNCTUATOR(STAR, "*")
    {nullptr, nullptr, Precedence::NONE},             // PUNCTUATOR(EQ, "=")

    {_RULE(variable), nullptr, Precedence::NONE},     // TOKEN(IDENTIFIER, "Identifier")
    {_RULE(numeric), nullptr, Precedence::NONE},      // TOKEN(NUMERIC, "Numeric")
    {_RULE(string), nullptr, Precedence::NONE},       // TOKEN(STRING, "String")

    {_RULE(literal), nullptr, Precedence::NONE},      // KEYWORD(FALSE, "false")
    {nullptr, nullptr, Precedence::NONE},             // KEYWORD(FN, "fn")
    {_RULE(literal), nullptr, Precedence::NONE},      // KEYWORD(NIL, "nil")
    {_RULE(literal), nullptr, Precedence::NONE},      // KEYWORD(TRUE, "true")
    {nullptr, nullptr, Precedence::NONE},             // KEYWORD(VAR, "var")

    {nullptr, nullptr, Precedence::NONE},             // TOKEN(EOF, "Eof")
    {nullptr, nullptr, Precedence::NONE},             // TOKEN(ERR, "Error")
  };
#undef _RULE

  return _rules[as_type<int>(kind)];
}

void GlobalParser::error_at(const Token& tok, const str_t& msg) noexcept {
  if (panic_mode_)
    return;
  panic_mode_ = true;

  std::cerr
    << colorful::fg::red
    << "SyntaxError:" << std::endl
    << "  [LINE: " << tok.lineno() << "] ERROR ";
  if (tok.kind() == TokenKind::TK_EOF)
    std::cerr << "at end ";
  else if (tok.kind() == TokenKind::TK_ERR)
    TADPOLE_UNUSED(0);
  else
    std::cerr << "at `" << tok.literal() << "` ";
  std::cerr << ": " << msg << colorful::reset << std::endl;
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

void GlobalParser::init_compiler(FnCompiler* compiler, int scope_depth, FnType fn_type) {
  StringObject* fn_name{};
  if (fn_type == FnType::FUNCTION)
    fn_name = StringObject::create(prev_.as_string());

  compiler->set_compiler(
      curr_compiler_,
      FunctionObject::create(fn_name),
      fn_type,
      scope_depth);
  curr_compiler_ = compiler;

  curr_compiler_->append_local({Token::make(""), curr_compiler_->scope_depth(), false});
}

FunctionObject* GlobalParser::finish_compiler() {
  emit_return();

  FunctionObject* fn =  curr_compiler_->fn();
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
  return curr_chunk()->add_constant(StringObject::create(name.as_string()));
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
  u8_t nargs = 0;
  if (!check(TokenKind::TK_RPAREN)) {
    do {
      expression();
      ++nargs;

      if (nargs > kMaxArgs)
        error(from_fmt("cannot have more than `%d` arguments", kMaxArgs));
    } while (match(TokenKind::TK_COMMA));
  }
  consume(TokenKind::TK_RPAREN, "expect `)` after function arguments");

  return nargs;
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

void GlobalParser::parse_precedence(Precedence precedence) {
  advance();
  auto& prefix_fn = get_rule(prev_.kind()).prefix;
  if (!prefix_fn) {
    error("expect expression");
    return;
  }

  bool can_assign = precedence <= Precedence::ASSIGN;
  prefix_fn(*this, can_assign);

  while (precedence <= get_rule(curr_.kind()).precedence) {
    advance();
    auto& infix_fn = get_rule(prev_.kind()).infix;

    if (infix_fn)
      infix_fn(*this, can_assign);
  }

  if (can_assign && match(TokenKind::TK_EQ)) {
    error("invalid assignment target");
    expression();
  }
}

void GlobalParser::binary(bool can_assign) {
  TokenKind op = prev_.kind();

  parse_precedence(get_rule(op).precedence + 1);
  switch (op) {
  case TokenKind::TK_PLUS: emit_byte(Code::ADD); break;
  case TokenKind::TK_MINUS: emit_byte(Code::SUB); break;
  case TokenKind::TK_STAR: emit_byte(Code::MUL); break;
  case TokenKind::TK_SLASH: emit_byte(Code::DIV); break;
  default: break;
  }
}

void GlobalParser::call(bool can_assign) {
  emit_byte(Code::CALL_0 + arguments());
}

void GlobalParser::grouping(bool can_assign) {
  expression();
  consume( TokenKind::TK_RPAREN, "expect `)` after grouping expression");
}

void GlobalParser::literal(bool can_assign) {
  switch (prev_.kind()) {
  case TokenKind::KW_NIL: emit_byte(Code::NIL); break;
  case TokenKind::KW_FALSE: emit_byte(Code::FALSE); break;
  case TokenKind::KW_TRUE: emit_byte(Code::TRUE); break;
  default: break;
  }
}

void GlobalParser::variable(bool can_assign) {
  named_variable(prev_, can_assign);
}

void GlobalParser::numeric(bool can_assign) {
  emit_constant(prev_.as_numeric());
}

void GlobalParser::string(bool can_assign) {
  emit_constant(StringObject::create(prev_.as_string()));
}

void GlobalParser::block() {
  while (!check(TokenKind::TK_EOF) && !check(TokenKind::TK_RBRACE))
    declaration();
  consume(TokenKind::TK_RBRACE, "expect `}` after block body");
}

void GlobalParser::function(FnType fn_type) {
  FnCompiler fn_compiler;
  init_compiler(&fn_compiler, 1, fn_type);

  consume(TokenKind::TK_LPAREN, "expect `(` after function name");
  if (!check(TokenKind::TK_RPAREN)) {
    do {
      u8_t param_constant = parse_variable("expect function parameters' name");
      define_global(param_constant);

      curr_compiler_->fn()->inc_arity();
      if (curr_compiler_->fn()->arity() > kMaxArgs)
        error(from_fmt("cannot have more than `%d` parameters", kMaxArgs));
    } while (match(TokenKind::TK_COMMA));
  }
  consume(TokenKind::TK_RPAREN, "expect `)` after function parameters");

  consume(TokenKind::TK_LBRACE, "expect `{` before function body");
  block();

  leave_scope();
  FunctionObject* fn = finish_compiler();

  emit_bytes(Code::CLOSURE, curr_chunk()->add_constant(fn));
  for (sz_t i = 0; i < fn->upvalues_count(); ++i) {
    auto& upvalue = fn_compiler.get_upvalue(i);
    emit_bytes(upvalue.is_local ? 1 : 0, upvalue.index);
  }
}

void GlobalParser::synchronize() {
  panic_mode_ = false;

  while (!check(TokenKind::TK_EOF)) {
    if (prev_.kind() == TokenKind::TK_SEMI)
      break;

    switch (curr_.kind()) {
    case TokenKind::KW_FN:
    case TokenKind::KW_VAR:
      return;
    default: break;
    }
    advance();
  }
}

void GlobalParser::expression() {
  parse_precedence(Precedence::ASSIGN);
}

void GlobalParser::declaration() {
  if (match(TokenKind::KW_FN))
    fn_decl();
  else if (match(TokenKind::KW_VAR))
    var_decl();
  else
    statement();

  if (panic_mode_)
    synchronize();
}

void GlobalParser::statement() {
  if (match(TokenKind::TK_LPAREN)) {
    enter_scope();
    block();
    leave_scope();
  }
  else {
    expr_stmt();
  }
}

void GlobalParser::fn_decl() {
  u8_t fn_constant = parse_variable("expect function name");
  mark_initialized();
  function(FnType::FUNCTION);

  define_global(fn_constant);
}

void GlobalParser::var_decl() {
  u8_t var_constant = parse_variable("expect variable name");

  if (match(TokenKind::TK_EQ))
    expression();
  else
    emit_byte(Code::NIL);
  consume(TokenKind::TK_SEMI, "expect `;` after variable declaration");

  define_global(var_constant);
}

void GlobalParser::expr_stmt() {}

}
