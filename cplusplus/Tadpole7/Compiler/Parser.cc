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
#include <Common/Colorful.hh>
#include <Lex/Lexer.hh>
#include <Object/StringObject.hh>
#include <Core/TadpoleVM.hh>
#include <Compiler/Parser.hh>

namespace Tadpole::Compiler {

GlobalParser::GlobalParser(Core::TadpoleVM& vm, Lex::Lexer& lex) noexcept
  : vm_(vm), lex_(lex) {
}

void GlobalParser::iter_objects(Object::ObjectVisitor&& visitor) {
  // TODO:
}

Object::FunctionObject* GlobalParser::compile() {
  // TODO:
  return nullptr;
}

const ParseRule& GlobalParser::get_rule(Lex::TokenKind kind) const noexcept {
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

  return _rules[Common::as_type<int>(kind)];
}

void GlobalParser::error_at(const Lex::Token& tok, const str_t& msg) noexcept {
  if (panic_mode_)
    return;
  panic_mode_ = true;

  std::cerr
    << Common::Colorful::fg::red
    << "SyntaxError:" << std::endl
    << "  [LINE: " << tok.lineno() << "] ERROR ";
  if (tok.kind() == Lex::TokenKind::TK_EOF)
    std::cerr << "at end ";
  else if (tok.kind() == Lex::TokenKind::TK_ERR)
    TADPOLE_UNUSED(0);
  else
    std::cerr << "at `" << tok.literal() << "` ";
  std::cerr << ": " << msg << Common::Colorful::reset << std::endl;
}

void GlobalParser::advance() {
  prev_ = curr_;

  for (;;) {
    curr_ = lex_.next_token();
    if (!check(Lex::TokenKind::TK_ERR))
      break;

    error_at_current(curr_.as_string());
  }
}

void GlobalParser::consume(Lex::TokenKind kind, const str_t& msg) {
  if (check(kind))
    advance();
  else
    error_at_current(msg);
}

bool GlobalParser::match(Lex::TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

void GlobalParser::init_compiler(FunctionCompiler* compiler, int scope_depth, FunctionType fn_type) {
  Object::StringObject* fn_name{};
  if (fn_type == FunctionType::FUNCTION)
    fn_name = Object::StringObject::create(prev_.as_string());

  compiler->set_compiler(
      curr_compiler_,
      Object::FunctionObject::create(fn_name),
      fn_type,
      scope_depth);
  curr_compiler_ = compiler;

  curr_compiler_->append_local({Lex::Token::make(""), curr_compiler_->scope_depth(), false});
}

Object::FunctionObject* GlobalParser::finish_compiler() {
  emit_return();

  Object::FunctionObject* fn = curr_compiler_->fn();
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
        emit_byte(var.is_upvalue ? Core::Code::CLOSE_UPVALUE : Core::Code::POP);
      });
}

u8_t GlobalParser::identifier_constant(const Lex::Token& name) noexcept {
  return curr_chunk()->add_constant(Object::StringObject::create(name.as_string()));
}

u8_t GlobalParser::parse_variable(const str_t& msg) {
  consume(Lex::TokenKind::TK_IDENTIFIER, msg);

  curr_compiler_->declare_localvar(prev_, [this](const str_t& m) { error(m); });
  if (curr_compiler_->scope_depth() > 0)
    return 0;
  return identifier_constant(prev_);
}

}
