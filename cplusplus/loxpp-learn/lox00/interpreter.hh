// Copyright (c) 2022 ASMlover. All rights reserved.
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

#include <iostream>
#include <memory>
#include <unordered_map>
#include "common.hh"
#include "errors.hh"
#include "expr.hh"
#include "stmt.hh"
#include "value.hh"
#include "environment.hh"
#include "callable.hh"
#include "builtins.hh"
#include "return.hh"

namespace loxpp::interpret {

class Interpreter final
  : public expr::Expr::Visitor
  , public stmt::Stmt::Visitor
  , public std::enable_shared_from_this<Interpreter> {
  ErrorReporter& err_reporter_;
  value::Value value_{};
  env::EnvironmentPtr globals_;
  env::EnvironmentPtr environment_;
  std::unordered_map<expr::ExprPtr, int> locals_;

  inline value::Value evaluate(const expr::ExprPtr& expr) noexcept {
    expr->accept(shared_from_this());
    return value_;
  }

  inline void execute(const stmt::StmtPtr& stmt) noexcept {
    stmt->accept(shared_from_this());
  }

  void execute_block(const std::vector<stmt::StmtPtr>& statements, const env::EnvironmentPtr& env) {
    env::EnvironmentPtr prev = environment_;

    environment_ = env;
    for (const auto& stmt : statements)
      execute(stmt);

    environment_ = prev;
  }

  void check_numeric_operands(const Token& oper, const value::Value& left, const value::Value& right) {
    if (left.is_numeric() && right.is_numeric())
      return;
    throw RuntimeError(oper, "operands must be numerics.");
  }

  void check_plus_operands(const Token& oper, const value::Value& left, const value::Value& right) {
    if ((left.is_numeric() && right.is_numeric()) || (left.is_string() && right.is_string()))
      return;
    throw RuntimeError(oper, "operands must be two numerics or two strings.");
  }

  value::Value lookup_variable(const Token& name, const expr::ExprPtr& expr) noexcept {
    if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
      return environment_->get_at(distance_iter->second, name);
    else
      return globals_->get(name);
  }

  virtual void visit_assign(const expr::AssignPtr& expr) override {
    value::Value value = evaluate(expr->value());

    if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
      environment_->assign_at(distance_iter->second, expr->name(), value);
    else
      globals_->assign(expr->name(), value);
  }

  virtual void visit_binary(const expr::BinaryPtr& expr) override {
    value::Value left = evaluate(expr->left());
    value::Value right = evaluate(expr->right());

    const Token& oper = expr->oper();
    switch (oper.type()) {
    case TokenType::TK_GT:
      check_numeric_operands(oper, left, right);
      value_ = left > right; break;
    case TokenType::TK_GE:
      check_numeric_operands(oper, left, right);
      value_ = left >= right; break;
    case TokenType::TK_LT:
      check_numeric_operands(oper, left, right);
      value_ = left < right; break;
    case TokenType::TK_LE:
      check_numeric_operands(oper, left, right);
      value_ = left <= right; break;
    case TokenType::TK_PLUS:
      check_plus_operands(oper, left, right);
      value_ = left + right; break;
    case TokenType::TK_MINUS:
      check_numeric_operands(oper, left, right);
      value_ = left - right; break;
    case TokenType::TK_SLASH:
      check_numeric_operands(oper, left, right);
      value_ = left / right; break;
    case TokenType::TK_STAR:
      check_numeric_operands(oper, left, right);
      value_ = left * right; break;
    case TokenType::TK_NE: value_ = left != right; break;
    case TokenType::TK_EQEQ: value_ = left == right; break;
    }
  }

  virtual void visit_call(const expr::CallPtr& expr) override {
    value::Value callee = evaluate(expr->callee());
    if (!callee.is_callable())
      throw RuntimeError(expr->paren(), "can only call functions and classes");

    std::vector<value::Value> arguments;
    for (const auto& argument : expr->arguments())
      arguments.push_back(evaluate(argument));

    auto function = callee.as_callable();
    if (arguments.size() != function->arity()) {
      throw RuntimeError(expr->paren(), "expected " + as_string(function->arity())
          + " arguments but got " + as_string(arguments.size()));
    }
    value_ = function->call(shared_from_this(), arguments);
  }

  virtual void visit_get(const expr::GetPtr& expr) override {}

  virtual void visit_grouping(const expr::GroupingPtr& expr) override {
    evaluate(expr->expression());
  }

  virtual void visit_literal(const expr::LiteralPtr& expr) override {
    value_ = expr->value();
  }

  virtual void visit_logical(const expr::LogicalPtr& expr) override {
    value::Value left = evaluate(expr->left());

    if (expr->oper().type() == TokenType::KW_OR) {
      if (left.is_truthy()) {
        value_ = left;
        return;
      }
    }
    else {
      if (!left.is_truthy()) {
        value_ = left;
        return;
      }
    }

    value_ = evaluate(expr->right());
  }

  virtual void visit_set(const expr::SetPtr& expr) override {}
  virtual void visit_super(const expr::SuperPtr& expr) override {}
  virtual void visit_this(const expr::ThisPtr& expr) override {}

  virtual void visit_unary(const expr::UnaryPtr& expr) override {
    value::Value right = evaluate(expr->right());

    switch (expr->oper().type()) {
    case TokenType::TK_MINUS:
      value_ = -right; break;
    case TokenType::TK_NE:
      value_ = !right; break;
    }
  }

  virtual void visit_variable(const expr::VariablePtr& expr) override {
    value_ = lookup_variable(expr->name(), expr);
  }

  virtual void visit_block(const stmt::BlockPtr& stmt) override {
    execute_block(stmt->statements(), std::make_shared<env::Environment>(environment_));
  }

  virtual void visit_class(const stmt::ClassPtr& stmt) override {
    environment_->define(stmt->name().as_string(), nullptr);
    auto klass = std::make_shared<callable::Class>(stmt->name().as_string());
    environment_->assign(stmt->name(), value::Value{klass});
  }

  virtual void visit_expression(const stmt::ExpressionPtr& stmt) override {
    evaluate(stmt->expression());
  }

  virtual void visit_function(const stmt::FunctionPtr& stmt) override {
    callable::FunctionPtr function = std::make_shared<callable::Function>(stmt, environment_);
    environment_->define(stmt->name().literal(), value::Value(function));
  }

  virtual void visit_if(const stmt::IfPtr& stmt) override {
    value::Value cond = evaluate(stmt->condition());
    if (cond.is_truthy())
      execute(stmt->then_branch());
    else if (stmt->else_branch())
      execute(stmt->else_branch());
  }

  virtual void visit_print(const stmt::PrintPtr& stmt) override {
    value::Value value = evaluate(stmt->expression());
    std::cout << value << std::endl;
  }

  virtual void visit_return(const stmt::ReturnPtr& stmt) override {
    expr::ExprPtr value = stmt->value();
    throw except::Return(value ?  evaluate(value) : value::Value{});
  }

  virtual void visit_var(const stmt::VarPtr& stmt) override {
    value::Value value = nullptr;
    if (stmt->initializer() != nullptr)
      value = evaluate(stmt->initializer());

    environment_->define(stmt->name().literal(), value);
  }

  virtual void visit_while(const stmt::WhilePtr& stmt) override {
    while (evaluate(stmt->condition()).is_truthy())
      execute(stmt->body());
  }
public:
  Interpreter(ErrorReporter& err_reporter) noexcept
    : err_reporter_{err_reporter}
    , globals_{new env::Environment()}
    , environment_{globals_} {
    globals_->define("clock", value::Value(std::make_shared<builtins::NativeClock>()));
  }

  inline env::EnvironmentPtr globals() const noexcept { return globals_; }

  inline void invoke_execute_block(
      const std::vector<stmt::StmtPtr>& stmts, const env::EnvironmentPtr& env) noexcept {
    execute_block(stmts, env);
  }

  template <typename N> inline void resolve(const expr::ExprPtr& expr, N depth) noexcept {
    locals_.insert({expr, as_type<int>(depth)});
  }

  void interpret(const expr::ExprPtr& expression) noexcept {
    try {
      value::Value value = evaluate(expression);
      std::cout << value << std::endl;
    }
    catch (const RuntimeError& err) {
      err_reporter_.error(err.token(), err.message());
    }
  }

  void interpret(const std::vector<stmt::StmtPtr>& statements) noexcept {
    try {
      for (auto& stmt : statements)
        execute(stmt);
    }
    catch (const RuntimeError& err) {
      err_reporter_.error(err.token(), err.message());
    }
  }
};

}
