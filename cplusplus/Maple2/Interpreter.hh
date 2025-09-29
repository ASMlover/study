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
