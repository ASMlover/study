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
#include "Parser.hh"
#include "Token.hh"
#include "Interpreter.hh"
#include <format>
#include <memory>
#include <stdexcept>
#include <vector>

namespace ms {

Value LiteralExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value GroupingExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value UnaryExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value BinayExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value LogicalExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value VariableExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value AssignExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value CallExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value GetExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value SetExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value ThisExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

Value SuperExpr::accept(Interpreter& interpreter) const {
  return interpreter.visit(*this);
}

void ExpressionStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void PrintStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void VarStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void BlockStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void IfStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void WhileStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void FunctionStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void ReturnStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void ClassStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

void ImportStmt::accept(Interpreter& interpreter) const {
  interpreter.visit(*this);
}

// ----------------------------------- Parser ----------------------------------
StmtPtr Parser::declaration() noexcept {
  try {
    if (match({TokenType::KW_CLASS}))
      return class_declaration();
    if (match({TokenType::KW_FUN}))
      return function("function");
    if (match({TokenType::KW_VAR}))
      return var_declaration();
    return statement();
  } catch (const std::runtime_error&) {
    synchronize();
  }
  return nullptr;
}

StmtPtr Parser::class_declaration() noexcept {
  auto name = consume(TokenType::TK_IDENTIFIER, "Expect class name.");

  VariableExprPtr superclass = nullptr;
  if (match({TokenType::TK_LESS})) {
    consume(TokenType::TK_IDENTIFIER, "Expect superclass name");
    superclass = std::make_shared<VariableExpr>(prev());
  }
  consume(TokenType::TK_LBRACE, "Expect `(` before class body.");

  std::vector<FunctionStmtPtr> methods;
  while (!check(TokenType::TK_RBRACE) && !is_at_end())
    methods.push_back(function("method"));

  consume(TokenType::TK_RBRACE, "Expect `}` after class body.");
  return std::make_shared<ClassStmt>(name, std::move(superclass), methods);
}

FunctionStmtPtr Parser::function(const str_t& kind) {
  auto name = consume(TokenType::TK_IDENTIFIER, std::format("Expect {} name.", kind));
  consume(TokenType::TK_LPAREN, std::format("Expect `(` after {} name.", kind));

  std::vector<Token> parameters;
  if (!check(TokenType::TK_RPAREN)) {
    do {
      if (parameters.size() >= 255)
        throw std::runtime_error(std::format("Error at line {}: cannot have more than 255 parameters.", peek().lineno()));

      parameters.push_back(consume(TokenType::TK_IDENTIFIER, "Expect paramter name."));
    } while (match({TokenType::TK_COMMA}));
  }
  consume(TokenType::TK_RPAREN, "Expect `)` after parameters.");
  consume(TokenType::TK_LBRACE, std::format("Expect `{{` before {} body.", kind));

  auto body = block();
  return std::make_shared<FunctionStmt>(name, parameters, std::move(body));
}

StmtPtr Parser::var_declaration() noexcept {
  auto name = consume(TokenType::TK_IDENTIFIER, "Expect variable name.");

  ExprPtr initializer = nullptr;
  if (match({TokenType::TK_EQUAL}))
    initializer = expression();
  consume(TokenType::TK_SEMICOLON, "Expect `;` after variable declaration.");

  return std::make_shared<VarStmt>(name, std::move(initializer));
}

StmtPtr Parser::statement() noexcept {
  if (match({TokenType::KW_FOR}))
    return for_statement();
  if (match({TokenType::KW_IF}))
    return if_statement();
  if (match({TokenType::KW_PRINT}))
    return print_statement();
  if (match({TokenType::KW_RETURN}))
    return return_statement();
  if (match({TokenType::KW_WHILE}))
    return while_statement();
  if (match({TokenType::TK_LBRACE}))
    return std::make_shared<BlockStmt>(block());
  if (match({TokenType::KW_IMPORT}))
    return import_statement();

  return expression_statement();
}

StmtPtr Parser::for_statement() noexcept {
  consume(TokenType::TK_LPAREN, "Expect `(` after `for`.");

  StmtPtr initializer;
  if (match({TokenType::TK_SEMICOLON}))
    initializer = nullptr;
  else if (match({TokenType::KW_VAR}))
    initializer = var_declaration();
  else
    initializer = expression_statement();

  ExprPtr condition{};
  if (!check(TokenType::TK_SEMICOLON))
    condition = expression();
  consume(TokenType::TK_SEMICOLON, "Expect `;` after loop condition.");

  ExprPtr increment{};
  if (!check(TokenType::TK_RPAREN))
    increment = expression();
  consume(TokenType::TK_RPAREN, "Expect `)` after for statement.");

  auto body = statement();
  if (increment) {
    std::vector<StmtPtr> stmts{body, std::make_shared<ExpressionStmt>(increment)};
    body = std::make_shared<BlockStmt>(stmts);
  }

  if (!condition)
    condition = std::make_shared<LiteralExpr>(true);
  body = std::make_shared<WhileStmt>(std::move(condition), std::move(body));

  if (initializer) {
    std::vector<StmtPtr> stmts{initializer, body};
    body = std::make_shared<BlockStmt>(stmts);
  }

  return body;
}

StmtPtr Parser::if_statement() noexcept {
  consume(TokenType::TK_LPAREN, "Expect `(` after `if`.");
  auto condition = expression();
  consume(TokenType::TK_RPAREN, "Expect `)` after if condition.");

  auto then_branch = statement();
  StmtPtr else_branch{};
  if (match({TokenType::KW_ELSE}))
    else_branch = statement();

  return std::make_shared<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch));
}

StmtPtr Parser::print_statement() noexcept {
  auto value = expression();
  consume(TokenType::TK_SEMICOLON, "Expect `;` after value.");
  return std::make_shared<PrintStmt>(std::move(value));
}

StmtPtr Parser::return_statement() noexcept {
  auto keyword = prev();
  ExprPtr value{};
  if (!check(TokenType::TK_SEMICOLON))
    value = expression();
  consume(TokenType::TK_SEMICOLON, "Expect `;` after return value.");
  return std::make_shared<ReturnStmt>(keyword, std::move(value));
}

StmtPtr Parser::while_statement() noexcept {
  consume(TokenType::TK_LPAREN, "Expect `(` after `while`.");
  auto condition = expression();
  consume(TokenType::TK_RPAREN, "Expect `)` after while condition.");
  auto body = statement();
  return std::make_shared<WhileStmt>(std::move(condition), std::move(body));
}

std::vector<StmtPtr> Parser::block() noexcept {
  std::vector<StmtPtr> statements;
  while (!check(TokenType::TK_RBRACE) && !is_at_end())
    statements.push_back(declaration());
  consume(TokenType::TK_RBRACE, "Expect `}` after block.");

  return statements;
}

StmtPtr Parser::import_statement() noexcept {
  auto module_name = consume(TokenType::TK_STRING, "Expect module path string.");
  consume(TokenType::TK_SEMICOLON, "Expect `;` after import statement.");
  return std::make_shared<ImportStmt>(module_name);
}

StmtPtr Parser::expression_statement() noexcept {
  auto expr = expression();
  consume(TokenType::TK_SEMICOLON, "Expect `;` after expression.");
  return std::make_shared<ExpressionStmt>(std::move(expr));
}

ExprPtr Parser::expression() noexcept {
  return assignment();
}

ExprPtr Parser::assignment() {
  auto expr = or_expr();
  if (match({TokenType::TK_EQUAL})) {
    auto& equals = prev();
    auto value = assignment();

    if (auto var_expr = as_down<VariableExpr>(expr.get())) {
      return std::make_shared<AssignExpr>(var_expr->name(), std::move(value));
    }
    else if (auto get_expr = as_down<GetExpr>(expr.get())) {
      return std::make_shared<SetExpr>(std::move(get_expr->object()), get_expr->name(), std::move(value));
    }

    throw std::runtime_error(std::format("Error at line {}: Invalid assignment target", equals.lineno()));
  }
  return expr;
}

ExprPtr Parser::or_expr() noexcept {
  auto expr = and_expr();

  while (match({TokenType::KW_OR})) {
    auto op = prev();
    auto right = and_expr();
    expr = std::make_shared<LogicalExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::and_expr() noexcept {
  auto expr = equality();

  while (match({TokenType::KW_AND})) {
    auto op = prev();
    auto right = equality();
    expr = std::make_shared<LogicalExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::equality() noexcept {
  auto expr = comparison();

  while (match({TokenType::TK_BANG_EQUAL, TokenType::TK_EQUAL_EQUAL})) {
    auto op = prev();
    auto right = comparison();
    expr = std::make_shared<BinayExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::comparison() noexcept {
  auto expr = term();

  while (match({TokenType::TK_GREATER, TokenType::TK_GREATER_EQUAL,
        TokenType::TK_LESS, TokenType::TK_LESS_EQUAL})) {
    auto op = prev();
    auto right = term();
    expr = std::make_shared<BinayExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::term() noexcept {
  auto expr = factor();

  while (match({TokenType::TK_MINUS, TokenType::TK_PLUS})) {
    auto op = prev();
    auto right = factor();
    expr = std::make_shared<BinayExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::factor() noexcept {
  auto expr = unary();

  while (match({TokenType::TK_SLASH, TokenType::TK_STAR})) {
    auto op = prev();
    auto right = unary();
    expr = std::make_shared<BinayExpr>(std::move(expr), op, right);
  }
  return expr;
}

ExprPtr Parser::unary() noexcept {
  if (match({TokenType::TK_BANG, TokenType::TK_MINUS})) {
    auto op = prev();
    auto right = unary();
    return std::make_shared<UnaryExpr>(op, std::move(right));
  }
  return call();
}

ExprPtr Parser::call() noexcept {
  auto expr = primary();

  for (;;) {
    if (match({TokenType::TK_LPAREN})) {
      expr = finish_call(std::move(expr));
    }
    else if (match({TokenType::TK_DOT})) {
      auto name = consume(TokenType::TK_IDENTIFIER, "Expect property name after `.`.");
      expr = std::make_shared<GetExpr>(std::move(expr), name);
    }
    else {
      break;
    }
  }
  return expr;
}

ExprPtr Parser::finish_call(ExprPtr callee) {
  std::vector<ExprPtr> arguments;

  if (!check(TokenType::TK_RPAREN)) {
    do {
      if (arguments.size() >= 255)
        throw std::runtime_error(std::format("Error at line {}: Cannot have more than 255 arguments.", peek().lineno()));
      arguments.push_back(expression());
    } while (match({TokenType::TK_COMMA}));
  }

  auto paren = consume(TokenType::TK_RPAREN, "Expect `)` after arguments.");
  return std::make_shared<CallExpr>(std::move(callee), paren, arguments);
}

ExprPtr Parser::primary() {
  if (match({TokenType::KW_FALSE}))
    return std::make_shared<LiteralExpr>(false);
  if (match({TokenType::KW_TRUE}))
    return std::make_shared<LiteralExpr>(true);
  if (match({TokenType::KW_NIL}))
    return std::make_shared<LiteralExpr>(nullptr);

  if (match({TokenType::TK_NUMBER}))
    return std::make_shared<LiteralExpr>(prev().as_number());
  if (match({TokenType::TK_STRING}))
    return std::make_shared<LiteralExpr>(prev().as_string());

  if (match({TokenType::KW_SUPER})) {
    auto keyword = prev();
    consume(TokenType::TK_DOT, "Expect `.` after `super`.");
    auto method = consume(TokenType::TK_IDENTIFIER, "Expect superclass method name.");
    return std::make_shared<SuperExpr>(keyword, method);
  }
  if (match({TokenType::KW_THIS}))
    return std::make_shared<ThisExpr>(prev());

  if (match({TokenType::TK_IDENTIFIER}))
    return std::make_shared<VariableExpr>(prev());
  if (match({TokenType::TK_LPAREN})) {
    auto expr = expression();
    consume(TokenType::TK_RPAREN, "Expect `)` after expression.");
    return std::make_shared<GroupingExpr>(std::move(expr));
  }

  throw std::runtime_error(std::format("Error at line {}: Expect expression.", prev().lineno()));
}

}
