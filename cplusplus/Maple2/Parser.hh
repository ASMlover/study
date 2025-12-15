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
        return true;
      }
    }
    return false;
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
    // statement -> exprStmt | ifStmt | whileStmt | forStmt | printStmt | returnStmt | importStmt | block ;

    if (match({TokenType::KW_IF}))
      return if_statement();
    if (match({TokenType::KW_WHILE}))
      return while_statement();
    if (match({TokenType::KW_FOR}))
      return for_statement();
    if (match({TokenType::KW_PRINT}))
      return print_statement();
    if (match({TokenType::KW_RETURN}))
      return return_statement();
    if (match({TokenType::KW_IMPORT}))
      return import_statement();
    if (match({TokenType::TK_LBRACE}))
      return std::make_shared<ast::Block>(block());
    return expression_statement();
  }

  inline ast::StmtPtr print_statement() noexcept {
    // printStmt -> "print" expression ";" ;

    auto value = expression();
    consume(TokenType::TK_SEMICOLON, "Expect `;` after value ...");
    return std::make_shared<ast::Print>(value);
  }

  inline std::vector<ast::StmtPtr> block() noexcept {
    // block -> "{" declaration* "}" ;

    std::vector<ast::StmtPtr> statements;
    while (!is_at_end() && !check(TokenType::TK_LBRACE))
      statements.push_back(declaration());
    consume(TokenType::TK_RBRACE, "Expect `}` after block.");

    return statements;
  }

  inline ast::StmtPtr expression_statement() noexcept {
    // exprStmt -> expression ;

    auto expr = expression();
    consume(TokenType::TK_SEMICOLON, "Expect `;` after expression.");
    return std::make_shared<ast::Expression>(expr);
  }

  inline ast::StmtPtr if_statement() noexcept {
    // ifStmt -> "if" "(" expression ")" statement ( "else" statement )? ;

    consume(TokenType::TK_LPAREN, "Expect `(` after key `if`.");
    auto condition = expression();
    consume(TokenType::TK_RPAREN, "Expect `)` after if condition.");
    auto then_branch = statement();

    ast::StmtPtr else_branch{};
    if (match({TokenType::KW_ELSE}))
      else_branch = statement();

    return std::make_shared<ast::If>(condition, then_branch, else_branch);
  }

  inline ast::StmtPtr while_statement() noexcept {
    // whileStmt -> "while" "(" expression ")" statement ;

    consume(TokenType::TK_LPAREN, "Expect `(` after key `while`.");
    auto condition = expression();
    consume(TokenType::TK_RPAREN, "Expect `)` after while condition.");
    auto body = statement();

    return std::make_shared<ast::While>(condition, body);
  }

  inline ast::StmtPtr for_statement() noexcept {
    // forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;

    consume(TokenType::TK_LPAREN, "Expect `(` after key `for`.");
    ast::StmtPtr initializer;
    if (match({TokenType::TK_SEMICOLON}))
      initializer = nullptr;
    else if (match({TokenType::KW_VAR}))
      initializer = var_declaration();
    else
      initializer = expression_statement();

    ast::ExprPtr condition;
    if (!check(TokenType::TK_SEMICOLON))
      condition = expression();
    consume(TokenType::TK_SEMICOLON, "Expect `;` after loop condition.");

    ast::ExprPtr increment;
    if (!check(TokenType::TK_RPAREN))
      increment = expression();
    consume(TokenType::TK_RPAREN, "Expect `)` after for clauses.");

    auto body = statement();
    if (increment) {
      std::vector<ast::StmtPtr> stmts{body, std::make_shared<ast::Expression>(increment)};
      body = std::make_shared<ast::Block>(stmts);
    }
    if (!condition)
      condition = std::make_shared<ast::Literal>(true);
    body = std::make_shared<ast::While>(condition, body);

    if (!initializer) {
      std::vector<ast::StmtPtr> stmts{initializer, body};
      body = std::make_shared<ast::Block>(stmts);
    }

    return body;
  }

  inline ast::StmtPtr return_statement() noexcept {
    // returnStmt -> "return" expression? ";" ;

    auto keyword = prev();
    ast::ExprPtr value{};
    if (!check(TokenType::TK_SEMICOLON))
      value = expression();

    consume(TokenType::TK_SEMICOLON, "Expect `;` after return value.");
    return std::make_shared<ast::Return>(keyword, value);
  }

  inline ast::StmtPtr import_statement() noexcept {
    // importStmt -> "import" module_name_str ";" ;

    auto module_name = consume(TokenType::TK_STRING, "Expect module path string.");
    consume(TokenType::TK_SEMICOLON, "Expect `;` after import statement.");
    return std::make_shared<ast::Import>(module_name);
  }

  inline ast::ExprPtr expression() noexcept {
    //  expression -> assignment ;

    return assignment();
  }

  inline ast::ExprPtr assignment() noexcept {
    // assignment -> ( call "." )? IDENTIFIER "=" assignment | logical_or ;

    auto expr = logical_or();
    if (match({TokenType::TK_EQUAL})) {
      const auto& equals = prev();
      auto value = assignment();

      if (std::dynamic_pointer_cast<ast::Variable>(expr)) {
        const auto& name = std::static_pointer_cast<ast::Variable>(expr)->name();
        return std::make_shared<ast::Assign>(name, value);
      }
      else if (std::dynamic_pointer_cast<ast::Get>(expr)) {
        const auto& get = std::static_pointer_cast<ast::Get>(expr);
        return std::make_shared<ast::Set>(get->object(), get->name(), value);
      }

      error(equals, "Invalid assignment target.");
    }
    return expr;
  }

  inline ast::ExprPtr logical_or() noexcept {
    // logical_or -> logical_and ( "or" logical_and )* ;

    auto expr = logical_and();
    while (match({TokenType::KW_OR})) {
       const auto& oper = prev();
       auto right = logical_and();
       expr = std::make_shared<ast::Logical>(expr, oper, right);
    }
    return expr;
  }

  inline ast::ExprPtr logical_and() noexcept {
    // logical_and -> equality ( "and" equality )* ;

    auto expr = equality();
    while (match({TokenType::KW_AND})) {
      const auto& oper = prev();
      auto right = equality();
      expr = std::make_shared<ast::Logical>(expr, oper, right);
    }
    return expr;
  }

  inline ast::ExprPtr equality() noexcept {
    // equality -> comparison ( ( "!=" | "==" ) comparison )* ;

    auto left = comparison();
    while (match({TokenType::TK_BANG_EQUAL, TokenType::TK_EQUAL_EQUAL})) {
      const auto& oper = prev();
      auto right = comparison();
      left = std::make_shared<ast::Binary>(left, oper, right);
    }
    return left;
  }

  inline ast::ExprPtr comparison() noexcept {
    // comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;

    auto left = term();
    while (match({TokenType::TK_GREATER, TokenType::TK_GREATER_EQUAL, TokenType::TK_LESS, TokenType::TK_LESS_EQUAL})) {
      const auto& oper = prev();
      auto right = term();
      left = std::make_shared<ast::Binary>(left, oper, right);
    }
    return left;
  }

  inline ast::ExprPtr term() noexcept {
    // term -> factor ( ( "-" | "+" ) factor )* ;

    auto left = factor();
    while (match({TokenType::TK_MINUS, TokenType::TK_PLUS})) {
      const auto& oper = prev();
      auto right = factor();
      left = std::make_shared<ast::Binary>(left, oper, right);
    }
    return left;
  }

  inline ast::ExprPtr factor() noexcept {
    // factor -> unary ( ( "/" | "*" ) unary )* ;

    auto left = unary();
    while (match({TokenType::TK_SLASH, TokenType::TK_STAR})) {
      const auto& oper = prev();
      auto right = unary();
      left = std::make_shared<ast::Binary>(left, oper, right);
    }
    return left;
  }

  inline ast::ExprPtr unary() noexcept {
    // unary -> ( "!" | "-" ) unary | call ;

    if (match({TokenType::TK_BANG, TokenType::TK_MINUS})) {
      const auto& oper = prev();
      auto right = unary();
      return std::make_shared<ast::Unary>(oper, right);
    }
    return call();
  }

  inline ast::ExprPtr call() noexcept {
    // call -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;

    auto expr = primary();
    for (;;) {
      if (match({TokenType::TK_LPAREN})) {
        expr = finish_call(expr);
      }
      else if (match({TokenType::TK_DOT})) {
        auto name = consume(TokenType::TK_IDENTIFIER, "Expect property name after .");
        expr = std::make_shared<ast::Get>(expr, name);

        // TODO: module getting ...
      }
      else {
        break;
      }
    }
    return expr;
  }

  inline ast::ExprPtr finish_call(const ast::ExprPtr& callee) noexcept {
    std::vector<ast::ExprPtr> arguments;
    if (!check(TokenType::TK_RPAREN)) {
      do {
        if (arguments.size() >= 255)
          error(peek(), "Cannot have more than 255 arguments.");
        arguments.push_back(expression());
      } while (match({TokenType::TK_COMMA}));
    }
    auto paren = consume(TokenType::TK_RPAREN, "Expect `)` after arguments.");

    return std::make_shared<ast::Call>(callee, paren, arguments);
  }

  inline ast::ExprPtr primary() noexcept(false) {
    // primary -> NUMBER | STRING | IDENTIFIER | "true" | "false" | "nil" | "this"
    //          | "(" expression ")" | "super" "." IDENTIFIER ;

    if (match({TokenType::KW_TRUE}))
      return std::make_shared<ast::Literal>(true);
    if (match({TokenType::KW_FALSE}))
      return std::make_shared<ast::Literal>(false);
    if (match({TokenType::KW_NIL}))
      return std::make_shared<ast::Literal>(nullptr);

    if (match({TokenType::TK_NUMBER}))
      return std::make_shared<ast::Literal>(prev().as_number());
    if (match({TokenType::TK_STRING}))
      return std::make_shared<ast::Literal>(prev().as_string());

    if (match({TokenType::KW_SUPER})) {
      auto keyword = prev();
      consume(TokenType::TK_DOT, "Expect `.` after `super`.");
      auto method = consume(TokenType::TK_IDENTIFIER, "Expect superclass method name.");
      return std::make_shared<ast::Super>(keyword, method);
    }
    if (match({TokenType::KW_THIS}))
      return std::make_shared<ast::This>(prev());

    if (match({TokenType::TK_IDENTIFIER}))
      return std::make_shared<ast::Variable>(prev());

    if (match({TokenType::TK_LPAREN})) {
      auto expr = expression();
      consume(TokenType::TK_RPAREN, "Expect `)` after expression.");
      return std::make_shared<ast::Grouping>(expr);
    }

    throw error(peek(), "Expect expression.");
  }
public:
  Parser(ErrorReporter& error_reporter, const std::vector<Token>& tokens) noexcept
    : error_reporter_{error_reporter}, tokens_{tokens} {
  }

  inline std::vector<ast::StmtPtr> parse() noexcept {
    // program -> declaration* EOF ;

    std::vector<ast::StmtPtr> statements;
    while (!is_at_end())
      statements.push_back(declaration());
    return statements;
  }
};

}
