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

#include <memory>
#include <vector>
#include <unordered_map>
#include "Common.hh"
#include "Errors.hh"
#include "Expr.hh"
#include "Stmt.hh"
#include "Interpreter.hh"

namespace ms {

class Resolver final
  : public ast::Expr::Visitor
  , public ast::Stmt::Visitor
  , public std::enable_shared_from_this<Resolver> {
  using ScopeMap        = std::unordered_map<str_t, bool>;
  using InterpreterPtr  = std::shared_ptr<Interpreter>;

  enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
  };

  enum class ClassType {
    NONE,
    CLASS,
    SUBCLASS,
  };

  ErrorReporter& err_reporter_;
  InterpreterPtr interpreter_;
  std::vector<ScopeMap> scopes_;
  FunctionType current_function_{FunctionType::NONE};
  ClassType current_class_{ClassType::NONE};

  inline void resolve(const ast::ExprPtr& expr) noexcept { expr->accept(shared_from_this()); }
  inline void resolve(const ast::StmtPtr& stmt) noexcept { stmt->accept(shared_from_this()); }

  inline void resolve(const std::vector<ast::StmtPtr>& statements) noexcept {
    for (const auto& stmt : statements)
      resolve(stmt);
  }

  inline void begin_scope() noexcept { scopes_.push_back({}); }
  inline void end_scope() noexcept { scopes_.pop_back(); }

  void declare(const Token& name) noexcept {
    if (scopes_.empty())
      return;

    auto& scope = scopes_.back();
    auto name_key = name.literal();
    if (auto it = scope.find(name_key); it != scope.end())
      err_reporter_.error(name, "Already a variable with this name in this scope.");
    scope.insert({name_key, false});
  }

  void define(const Token& name) noexcept {
    if (scopes_.empty())
      return;

    scopes_.back().insert({name.literal(), true});
  }

  void resolve_local(const ast::ExprPtr& expr, const Token& name) noexcept {
    auto n = scopes_.size() - 1;
    auto name_key = name.literal();
    for (auto i = n; i >= 0; --i) {
      auto& scope = scopes_[i];
      if (scope.find(name_key) != scope.end()) {
        interpreter_->resolve(expr, n - i);
        return;
      }
    }
  }

  void resolve_function(const ast::FunctionPtr& function, FunctionType type) noexcept {
    auto enclosing_function = current_function_;
    current_function_ = type;

    begin_scope();

    for (const auto& param : function->params()) {
      declare(param);
      define(param);
    }
    resolve(function->body());

    end_scope();

    current_function_ = enclosing_function;
  }
private:
  virtual void visit(const ast::AssignPtr& expr) override {
    resolve(expr->value());
    resolve_local(expr, expr->name());
  }

  virtual void visit(const ast::BinaryPtr& expr) override {
    resolve(expr->left());
    resolve(expr->right());
  }

  virtual void visit(const ast::CallPtr& expr) override {
    resolve(expr->callee());

    expr->iter_arguments([&](const auto& arg) {
          resolve(arg);
        });
  }

  virtual void visit(const ast::GetPtr& expr) override {
    resolve(expr->object());
  }

  virtual void visit(const ast::GroupingPtr& expr) override {
    resolve(expr->expression());
  }

  virtual void visit(const ast::LiteralPtr& expr) override {}

  virtual void visit(const ast::LogicalPtr& expr) override {
    resolve(expr->left());
    resolve(expr->right());
  }

  virtual void visit(const ast::SetPtr& expr) override {
    resolve(expr->value());
    resolve(expr->object());
  }
public:
  Resolver(ErrorReporter& err_reporter, const InterpreterPtr& interpreter) noexcept
    : err_reporter_{err_reporter}, interpreter_{interpreter} {
  }

  inline void invoke_resolve(const std::vector<ast::StmtPtr>& stmts) noexcept {
  }
};

}
