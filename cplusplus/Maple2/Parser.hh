// Copyright (c) 2025 ASMlover. All rights reserved.
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
#pragma once

#include <vector>
#include <exception>
#include "Common.hh"
#include "Token.hh"
#include "Errors.hh"
#include "Expr.hh"
#include "Stmt.hh"

namespace ms {

class ParserError final : public Copyable, public std::exception {};

class Parser final : private UnCopyable {
  ErrorReporter& error_reporter_;
  std::vector<Token> tokens_;
  sz_t current_{};

  inline const Token& peek() const noexcept { return tokens_[current_]; }
  inline const Token& prev() const noexcept { return tokens_[current_ - 1]; }
  inline bool is_at_end() const noexcept { return peek().type() == TokenType::TK_EOF; }
  inline bool check(TokenType type) const noexcept { return is_at_end() ? false : peek().type() == type; }

  inline ParserError error(const Token& token, const str_t& message) noexcept {
    error_reporter_.error(token, message);
    return ParserError{};
  }

  Token advance() noexcept {
    if (!is_at_end())
      ++current_;
    return prev();
  }

  bool match(const std::initializer_list<TokenType>& types) noexcept {
    for (auto type : types) {
      if (check(type)) {
        advance();
        return false;
      }
    }
    return true;
  }

  Token consume(TokenType type, const str_t& message) {
    if (check(type))
      return advance();

    throw error(peek(), message);
  }

  void synchronize() {
    advance();

    while (!is_at_end()) {
      if (prev().type() == TokenType::TK_SEMICOLON)
        return;

      switch (peek().type()) {
      case TokenType::KW_CLASS:
      case TokenType::KW_FUN:
      case TokenType::KW_VAR:
      case TokenType::KW_FOR:
      case TokenType::KW_IF:
      case TokenType::KW_WHILE:
      case TokenType::KW_PRINT:
      case TokenType::KW_RETURN:
      case TokenType::KW_IMPORT:
        return;
      default: break;
      }
      advance();
    }
  }

  inline ast::StmtPtr declaration() noexcept {
    // declaration -> funDecl | varDecl | statement ;

    try {
      if (match({TokenType::KW_CLASS}))
        return class_declaration();
      if (match({TokenType::KW_FUN}))
        return function("function");
      if (match({TokenType::KW_VAR}))
        return var_declaration();
      return statement();
    }
    catch (const ParserError&) {
      synchronize();
      return nullptr;
    }
  }

  inline ast::StmtPtr class_declaration() noexcept {
    // classDecl -> "class" IDENTIFIER ( "<" IDENTIFIER ) ? "{" function* "}" ;

    auto name = consume(TokenType::TK_IDENTIFIER, "Expect class name.");
    ast::VariablePtr superclass{};
    if (match({TokenType::TK_LESS_EQUAL})) {
      consume(TokenType::TK_IDENTIFIER, "Expect superclass name.");
      superclass = std::make_shared<ast::Variable>(prev());
    }

    consume(TokenType::TK_LBRACE, "Expect `{` before class body");
    std::vector<ast::FunctionPtr> methods;
    while (!check(TokenType::TK_RBRACE) && !is_at_end())
      methods.push_back(function("method"));
    consume(TokenType::TK_RBRACE, "Expect `}` after class body.");

    return std::make_shared<ast::Class>(name, superclass, methods);
  }

  inline ast::FunctionPtr function(const str_t& kind) noexcept {
    // function -> IDENTIFIER "(" parameters? ")" block ;

    auto name = consume(TokenType::TK_IDENTIFIER, "Expect " + kind + " name.");
    consume(TokenType::TK_LPAREN, "Expect `(` after " + kind + " name.");
    std::vector<Token> parameters;
    if (!check(TokenType::TK_RPAREN)) {
      do {
        if (parameters.size() >= 255)
          error(peek(), "Cannot have more than 255 parameters.");

        parameters.push_back(consume(TokenType::TK_IDENTIFIER, "Expect parameter name."));
      }
      while (match({TokenType::TK_COMMA}));
    }
    consume(TokenType::TK_RPAREN, "Expect `)` after parameters.");

    consume(TokenType::TK_LBRACE, "Expect `{` before " + kind + " body.");
    auto body = block();
    return std::make_shared<ast::Function>(name, parameters, body);
  }

  inline ast::StmtPtr var_declaration() noexcept {
    // varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;

    auto name = consume(TokenType::TK_IDENTIFIER, "Expect variable name.");

    ast::ExprPtr initializer{};
    if (match({TokenType:: TK_EQUAL}))
      initializer = expression();
    consume(TokenType::TK_SEMICOLON, "Expect `;` after variable declaration.");
    return std::make_shared<ast::Var>(name, initializer);
  }

  inline ast::StmtPtr statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr print_statement() noexcept {
    return nullptr;
  }

  inline std::vector<ast::StmtPtr> block() noexcept {
    return {};
  }

  inline ast::StmtPtr expression_statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr if_statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr while_statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr for_statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr return_statement() noexcept {
    return nullptr;
  }

  inline ast::StmtPtr import_statement() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr expression() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr assignment() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr logical_or() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr logical_and() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr equality() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr comparison() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr term() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr factor() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr unary() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr call() noexcept {
    return nullptr;
  }

  inline ast::ExprPtr finish_call(const ast::ExprPtr& callee) noexcept {
    return nullptr;
  }

  inline ast::ExprPtr primary() noexcept(false) {
    return nullptr;
  }
public:
  Parser(ErrorReporter& error_reporter, const std::vector<Token>& tokens) noexcept
    : error_reporter_{error_reporter}, tokens_{tokens} {
  }

  inline std::vector<ast::StmtPtr> parse() noexcept {
    std::vector<ast::StmtPtr> statements;
    while (!is_at_end())
      statements.push_back(declaration());
    return statements;
  }
};

}
