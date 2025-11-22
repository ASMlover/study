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

#include <iostream>
#include <memory>
#include <unordered_map>
#include "Common.hh"
#include "Errors.hh"
#include "Expr.hh"
#include "Stmt.hh"
#include "Value.hh"
#include "Environment.hh"
#include "Callable.hh"
#include "Builtins.hh"
#include "Return.hh"

namespace ms {

class Interpreter final
  : public ast::Expr::Visitor
  , public ast::Stmt::Visitor
  , public std::enable_shared_from_this<Interpreter> {
  ErrorReporter& err_reporter_;
  Value value_{};
  EnvironmentPtr globals_;
  EnvironmentPtr environment_;
  std::unordered_map<ast::ExprPtr, int> locals_;

  inline Value evaluate(const ast::ExprPtr& expr) noexcept {
    expr->accept(shared_from_this());
    return value_;
  }

  inline void execute(const ast::StmtPtr& stmt) noexcept {
    stmt->accept(shared_from_this());
  }

  void execute_block(const std::vector<ast::StmtPtr>& statements, const EnvironmentPtr& env) noexcept {
    auto prev = environment_;

    environment_ = env;
    for (const auto& stmt : statements)
      execute(stmt);
    environment_ = prev;
  }

  void check_number_operands(const Token& oper, const Value& left, const Value& right) {
    if (left.is_number() && right.is_number())
      return;
    throw RuntimeError(oper, "Operands must be numbers.");
  }

  void check_plus_operands(const Token& oper, const Value& left, const Value& right) {
    if ((left.is_number() && right.is_number()) || (left.is_string() && right.is_string()))
      return;
    throw RuntimeError(oper, "Operands must be two numbers or two strings.");
  }

  Value lookup_variable(const Token& name, const ast::ExprPtr& expr) noexcept {
    if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
      return environment_->get_at(distance_iter->second, name);
    else
      return globals_->get(name);
  }

  virtual void visit(const ast::AssignPtr& expr) override {
    auto value = evaluate(expr->value());

    if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
      environment_->assign_at(distance_iter->second, expr->name(), value);
    else
      globals_->assign(expr->name(), value);
  }

  virtual void visit(const ast::BinaryPtr& expr) override {
    auto left = evaluate(expr->left());
    auto right = evaluate(expr->right());

    const auto& oper = expr->op();
    switch (oper.type()) {
    case TokenType::TK_GREATER:
      check_number_operands(oper, left, right);
      value_ = left > right; break;
    case TokenType::TK_GREATER_EQUAL:
      check_number_operands(oper, left, right);
      value_ = left >= right; break;
    case TokenType::TK_LESS:
      check_number_operands(oper, left, right);
      value_ = left < right; break;
    case TokenType::TK_LESS_EQUAL:
      check_number_operands(oper, left, right);
      value_ = left <= right; break;
    case TokenType::TK_PLUS:
      check_plus_operands(oper, left, right);
      value_ = left + right; break;
    case TokenType::TK_MINUS:
      check_number_operands(oper, left, right);
      value_ = left - right; break;
    case TokenType::TK_SLASH:
      check_number_operands(oper, left, right);
      value_ = left / right; break;
    case TokenType::TK_STAR:
      check_number_operands(oper, left, right);
      value_ = left * right; break;
    case TokenType::TK_BANG_EQUAL: value_ = left != right; break;
    case TokenType::TK_EQUAL_EQUAL: value_ = left == right; break;
    default: break;
    }
  }

  virtual void visit(const ast::CallPtr& expr) override {
    auto callee = evaluate(expr->callee());
    if (!callee.is_callable())
      throw RuntimeError(expr->paren(), "Can only call functions and classes.");

    std::vector<Value> arguments;
    expr->iter_arguments([this, &arguments](const auto& arg) {
          arguments.push_back(evaluate(arg));
        });

    auto function = callee.as_callable();
    if (arguments.size() != function->arity()) {
      throw RuntimeError(expr->paren(),
          "Expected " + as_string(function->arity()) + " arguments but got " + as_string(arguments.size()));
    }
    value_ = function->call(shared_from_this(), arguments);
  }

  virtual void visit(const ast::GetPtr& expr) override {
    auto object = evaluate(expr->object());
    if (object.is_instance())
      value_ = object.as_instance()->get(expr->name());

    if (object.is_module()) {
      auto module = object.as_module();
      try {
        // TODO:
      }
      catch (const std::runtime_error& /*e*/) {
        // TODO:
      }
    }

    throw RuntimeError(expr->name(), "Only instances, modules have properties");
  }

  virtual void visit(const ast::GroupingPtr& expr) override {
    evaluate(expr->expression());
  }

  virtual void visit(const ast::LiteralPtr& expr) override {
    value_ = expr->value();
  }

  virtual void visit(const ast::LogicalPtr& expr) override {
    auto left = evaluate(expr->left());

    if (expr->op().type() == TokenType::KW_OR) {
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

  virtual void visit(const ast::SetPtr& expr) override {
    auto object = evaluate(expr->object());

    if (!object.is_instance())
      throw RuntimeError(expr->name(), "Only instances have fields.");

    auto value = evaluate(expr->value());
    object.as_instance()->set(expr->name(), value);
  }

  virtual void visit(const ast::SuperPtr& expr) override {
    auto distance = 0;
    if (auto super_iter = locals_.find(expr); super_iter != locals_.end())
      distance = super_iter->second;

    auto superclass = std::static_pointer_cast<Class>(
        environment_->get_at(distance, "super").as_callable());
    auto object = std::static_pointer_cast<Instance>(
        environment_->get_at(distance - 1, "this").as_instance());
    auto method = superclass->find_method(expr->method().literal());
    if (!method)
      throw RuntimeError(expr->method(), "Undefined property `" + expr->method().literal() + "`");
    method->bind(object);

    value_ = Value{method};
  }

  virtual void visit(const ast::ThisPtr& expr) override {
    value_ = lookup_variable(expr->keyword(), expr);
  }

  virtual void visit(const ast::UnaryPtr& expr) override {
    auto right = evaluate(expr->right());

    switch (expr->op().type()) {
    case TokenType::TK_MINUS: value_ = -right; break;
    case TokenType::TK_BANG_EQUAL: value_ = !right; break;
    default: break;
    }
  }

  virtual void visit(const ast::VariablePtr& expr) override {
    value_ = lookup_variable(expr->name(), expr);
  }

  virtual void visit(const ast::BlockPtr& stmt) override {
    execute_block(stmt->statements(), std::make_shared<Environment>(environment_));
  }

  virtual void visit(const ast::ClassPtr& stmt) override {
    Value superclass_value{};
    if (stmt->superclass()) {
      superclass_value = evaluate(stmt->superclass());
      if (!(superclass_value.is_callable() &&
          std::dynamic_pointer_cast<Class>(superclass_value.as_callable())))
        throw RuntimeError(stmt->superclass()->name(), "Superclass must be a class.");
    }

    environment_->define(stmt->name().as_string(), nullptr);

    if (stmt->superclass()) {
      environment_ = std::make_shared<Environment>(environment_);
      environment_->define("super", superclass_value);
    }

    std::unordered_map<str_t, FunctionPtr> methods;
    stmt->iter_methods([&](const auto& method) {
          auto function = std::make_shared<Function>(method, environment_, method->name().is_equal_to("init"));
        });

    auto superclass = std::static_pointer_cast<Class>(superclass_value.as_callable());
    auto klass = std::make_shared<Class>(stmt->name().literal(), superclass, methods);

    if (!superclass_value.is_nil())
      environment_ = environment_->enclosing();

    environment_->assign(stmt->name(), Value{klass});
  }

  virtual void visit(const ast::ExpressionPtr& stmt) override {
    evaluate(stmt->expression());
  }

  virtual void visit(const ast::FunctionPtr& stmt) override {
    auto function = std::make_shared<Function>(stmt, environment_, false);
    environment_->define(stmt->name().literal(), Value{function});
  }

  virtual void visit(const ast::IfPtr& stmt) override {
    auto cond = evaluate(stmt->condition());
    if (cond.is_truthy())
      execute(stmt->then_branch());
    else if (stmt->else_branch())
      execute(stmt->else_branch());
  }

  virtual void visit(const ast::PrintPtr& stmt) override {
    auto value = evaluate(stmt->expression());
    std::cout << value << std::endl;
  }

  virtual void visit(const ast::ReturnPtr& stmt) override {
    auto value = stmt->value();
    throw except::Return{value ? evaluate(value) : Value{}};
  }

  virtual void visit(const ast::VarPtr& stmt) override {
    Value value{};
    if (stmt->initializer() != nullptr)
      value = evaluate(stmt->initializer());

    environment_->define(stmt->name().literal(), value);
  }
public:
  Interpreter(ErrorReporter& err_reporter) noexcept
    : err_reporter_{err_reporter}, globals_{new Environment()}, environment_{globals_} {
    globals_->define("clock", Value{std::make_shared<NativeClock>()});
  }

  inline EnvironmentPtr globals() const noexcept { return globals_; }

  inline void invoke_execute_block(
      const std::vector<ast::StmtPtr>& stmts, const EnvironmentPtr& env) noexcept {
    execute_block(stmts, env);
  }

  template <typename N> inline void resolve(const ast::ExprPtr& expr, N depth) noexcept {
    locals_.insert({expr, as_type<int>(depth)});
  }

  void interpret(const ast::ExprPtr& expression) noexcept {
    try {
      auto value = evaluate(expression);
      std::cout << value << std::endl;
    }
    catch (const RuntimeError& err) {
      err_reporter_.error(err.token(), err.message());
    }
  }

  void interpret(const std::vector<ast::StmtPtr>& statements) noexcept {
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
