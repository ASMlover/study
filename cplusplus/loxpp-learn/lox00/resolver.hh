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

#include <memory>
#include <vector>
#include <unordered_map>
#include "common.hh"
#include "errors.hh"
#include "expr.hh"
#include "stmt.hh"
#include "interpreter.hh"

namespace loxpp::resolver {

class Resolver final
  : public expr::Expr::Visitor
  , public stmt::Stmt::Visitor
  , public std::enable_shared_from_this<Resolver> {
  using ScopeMap = std::unordered_map<str_t, bool>;
  using InterpreterPtr = std::shared_ptr<interpret::Interpreter>;

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

  inline void resolve(const expr::ExprPtr& expr) noexcept { expr->accept(shared_from_this()); }
  inline void resolve(const stmt::StmtPtr& stmt) noexcept { stmt->accept(shared_from_this()); }

  inline void resolve(const std::vector<stmt::StmtPtr>& statements) noexcept {
    for (const auto& stmt : statements)
      resolve(stmt);
  }

  inline void begin_scope() noexcept { scopes_.push_back({}); }
  inline void end_scope() noexcept { scopes_.pop_back(); }

  void declare(const Token& name) noexcept {
    if (scopes_.empty())
      return;

    auto& scope = scopes_.back();
    if (auto it = scope.find(name.literal()); it != scope.end())
      err_reporter_.error(name, "already a variable with this name in this scope");
    scope.insert({name.literal(), false});
  }

  void define(const Token& name) noexcept {
    if (scopes_.empty())
      return;

    scopes_.back().insert({name.literal(), true});
  }

  void resolve_local(const expr::ExprPtr& expr, const Token& name) noexcept {
    sz_t n = scopes_.size() - 1;
    for (sz_t i = n; i >= 0; --i) {
      auto& scope = scopes_[i];
      if (scope.find(name.literal()) != scope.end()) {
        interpreter_->resolve(expr, n - i);
        return;
      }
    }
  }

  void resolve_function(const stmt::FunctionPtr& function, FunctionType type) noexcept {
    FunctionType enclosing_function = current_function_;
    current_function_ = type;

    begin_scope();

    for (const Token& param : function->params()) {
      declare(param);
      define(param);
    }
    resolve(function->body());

    end_scope();

    current_function_ = enclosing_function;
  }
private:
  virtual void visit_assign(const expr::AssignPtr& expr) override {
    resolve(expr->value());
    resolve_local(expr, expr->name());
  }

  virtual void visit_binary(const expr::BinaryPtr& expr) override {
    resolve(expr->left());
    resolve(expr->right());
  }

  virtual void visit_call(const expr::CallPtr& expr) override {
    resolve(expr->callee());

    for (auto& argument : expr->arguments())
      resolve(argument);
  }

  virtual void visit_get(const expr::GetPtr& expr) override {
    resolve(expr->object());
  }

  virtual void visit_grouping(const expr::GroupingPtr& expr) override {
    resolve(expr->expression());
  }

  virtual void visit_literal(const expr::LiteralPtr& expr) override {}

  virtual void visit_logical(const expr::LogicalPtr& expr) override {
    resolve(expr->left());
    resolve(expr->right());
  }

  virtual void visit_set(const expr::SetPtr& expr) override {
    resolve(expr->value());
    resolve(expr->object());
  }

  virtual void visit_super(const expr::SuperPtr& expr) override {
    if (current_class_ == ClassType::NONE) {
      err_reporter_.error(expr->keyword(), "Cannot use `super` outside of a class");
    }
    else if (current_class_ != ClassType::SUBCLASS) {
      err_reporter_.error(expr->keyword(), "Cannot use `super` in a class with no superclass");
    }

    resolve_local(expr, expr->keyword());
  }

  virtual void visit_this(const expr::ThisPtr& expr) override {
    if (current_class_ == ClassType::NONE) {
      err_reporter_.error(expr->keyword(), "cannot use `this` outside of a class");
      return;
    }

    resolve_local(expr, expr->keyword());
  }

  virtual void visit_unary(const expr::UnaryPtr& expr) override {
    resolve(expr->right());
  }

  virtual void visit_variable(const expr::VariablePtr& expr) override {
    if (!scopes_.empty()) {
      auto& scope = scopes_.back();
      str_t name = expr->name().literal();
      if (scope.find(name) != scope.end() && !scope[name])
        throw RuntimeError(expr->name(), "cannot read local variable in its own initializer ...");
    }

    resolve_local(expr, expr->name());
  }

  virtual void visit_block(const stmt::BlockPtr& stmt) override {
    begin_scope();
    resolve(stmt->statements());
    end_scope();
  }

  virtual void visit_class(const stmt::ClassPtr& stmt) override {
    ClassType enclosing_class = current_class_;
    current_class_ = ClassType::CLASS;

    declare(stmt->name());
    define(stmt->name());

    expr::VariablePtr superclass = stmt->superclass();
    if (superclass) {
      if (stmt->name().is_equal_to(superclass->name()))
        err_reporter_.error(superclass->name(), "a class cannot inherit from itself");

      current_class_ = ClassType::SUBCLASS;
      resolve(superclass);
    }

    if (superclass) {
      begin_scope();
      scopes_.back().insert({"super", true});
    }

    begin_scope();
    scopes_.back().insert({"this", true});

    for (const auto& method : stmt->methods()) {
      FunctionType declaration = FunctionType::METHOD;
      if (method->name().literal() == "init")
        declaration = FunctionType::INITIALIZER;
      resolve_function(method, declaration);
    }

    end_scope();

    if (stmt->superclass())
      end_scope();

    current_class_ = enclosing_class;
  }

  virtual void visit_expression(const stmt::ExpressionPtr& stmt) override {
    resolve(stmt->expression());
  }

  virtual void visit_function(const stmt::FunctionPtr& stmt) override {
    declare(stmt->name());
    define(stmt->name());

    resolve_function(stmt, FunctionType::FUNCTION);
  }

  virtual void visit_if(const stmt::IfPtr& stmt) override {
    resolve(stmt->condition());
    resolve(stmt->then_branch());
    if (stmt->else_branch())
      resolve(stmt->else_branch());
  }

  virtual void visit_print(const stmt::PrintPtr& stmt) override {
    resolve(stmt->expression());
  }

  virtual void visit_return(const stmt::ReturnPtr& stmt) override {
    if (current_function_ == FunctionType::NONE)
      err_reporter_.error(stmt->keyword(), "cannot return from top-level code");

    if (stmt->value()) {
      if (current_function_ == FunctionType::INITIALIZER)
        err_reporter_.error(stmt->keyword(), "cannot return a value from an initializer");

      resolve(stmt->value());
    }
  }

  virtual void visit_var(const stmt::VarPtr& stmt) override {
  }

  virtual void visit_while(const stmt::WhilePtr& stmt) override {
    resolve(stmt->condition());
    resolve(stmt->body());
  }
public:
  Resolver(ErrorReporter& err_reporter, const InterpreterPtr& interpreter) noexcept
    : err_reporter_{err_reporter}
    , interpreter_{interpreter} {
  }

  inline void invoke_resolve(const std::vector<stmt::StmtPtr>& stmts) noexcept {
    resolve(stmts);
  }
};

}
