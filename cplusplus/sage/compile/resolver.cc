// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include "../common/errors.hh"
#include "interpreter.hh"
#include "resolver.hh"

namespace sage {

Resolver::Resolver(ErrorReport& err_report, const InterpreterPtr& interp)
  : err_report_(err_report)
  , interp_(interp) {
}

void Resolver::invoke_resolve(const std::vector<StmtPtr>& stmts) {
  try {
    resolve(stmts);
  }
  catch (const RuntimeError& r) {
    err_report_.error(r.get_token(), r.get_message());
  }
}

void Resolver::resolve(const ExprPtr& expr) {
  expr->accept(shared_from_this());
}

void Resolver::resolve(const std::vector<ExprPtr>& exprs) {
  for (auto& expr : exprs)
    resolve(expr);
}

void Resolver::resolve(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Resolver::resolve(const std::vector<StmtPtr>& stmts) {
  for (auto& stmt : stmts)
    resolve(stmt);
}

void Resolver::resolve_local(const Token& name, const ExprPtr& expr) {
  int n = static_cast<int>(scopes_.size()) - 1;
  for (int i = n; i >= 0; --i) {
    auto& scope = scopes_[i];
    if (scope.find(name.get_literal()) != scope.end()) {
      interp_->resolve(expr, n - 1);
      return;
    }
  }
  // not found, assume it is global
}

void Resolver::resolve_function(const FunctionStmtPtr& fn, FunKind kind) {
  FunKind enclosing_fn = curr_fn_;
  curr_fn_ = kind;

  enter_scope();
  for (auto& param : fn->params()) {
    declare(param);
    define(param);
  }
  resolve(fn->body());
  leave_scope();

  curr_fn_ = enclosing_fn;
}

void Resolver::enter_scope(void) {
  scopes_.push_back({});
}

void Resolver::leave_scope(void) {
  scopes_.pop_back();
}

void Resolver::declare(const Token& name) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    auto name_key = name.get_literal();
    if (scope.find(name_key) != scope.end()) {
      throw RuntimeError(name,
          "variable with this name already declared in this scope");
    }
    scope[name_key] = false;
  }
}

void Resolver::define(const Token& name) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    scope[name.get_literal()] = true;
  }
}

void Resolver::visit(const AssignExprPtr& expr) {
  resolve(expr->value());
  resolve_local(expr->name(), expr);
}

void Resolver::visit(const BinaryExprPtr& expr) {
  resolve(expr->left());
  resolve(expr->right());
}

void Resolver::visit(const CallExprPtr& expr) {
  resolve(expr->callee());
  for (auto& arg : expr->arguments())
    resolve(arg);
}

void Resolver::visit(const SetExprPtr& expr) {
  resolve(expr->value());
  resolve(expr->object());
}

void Resolver::visit(const GetExprPtr& expr) {
  resolve(expr->object());
}

void Resolver::visit(const GroupingExprPtr& expr) {
  resolve(expr->expression());
}

void Resolver::visit(const LiteralExprPtr& expr) {
}

void Resolver::visit(const LogicalExprPtr& expr) {
  resolve(expr->left());
  resolve(expr->right());
}

void Resolver::visit(const SelfExprPtr& expr) {
  if (curr_class_ != ClassKind::CLASS && curr_class_ != ClassKind::SUBCLASS)
    throw RuntimeError(expr->keyword(), "cannot use `self` outside of a class");

  resolve_local(expr->keyword(), expr);
}

void Resolver::visit(const SuperExprPtr& expr) {
  if (curr_class_ == ClassKind::NONE) {
    throw RuntimeError(expr->keyword(),
        "cannot use `super` outside of a class");
  }
  else if (curr_class_ != ClassKind::SUBCLASS) {
    throw RuntimeError(expr->keyword(),
        "cannot use `super` in a class without superclass");
  }

  resolve_local(expr->keyword(), expr);
}

void Resolver::visit(const UnaryExprPtr& expr) {
  resolve(expr->right());
}

void Resolver::visit(const VariableExprPtr& expr) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    auto name = expr->name().get_literal();
    if (scope.find(name) != scope.end() && !scope[name]) {
      throw RuntimeError(expr->name(),
          "cannot read local variable in its own initializer");
    }
  }
  resolve_local(expr->name(), expr);
}

void Resolver::visit(const FunctionExprPtr& expr) {
}

void Resolver::visit(const ExprStmtPtr& stmt) {
  resolve(stmt->expr());
}

void Resolver::visit(const PrintStmtPtr& stmt) {
  resolve(stmt->exprs());
}

void Resolver::visit(const LetStmtPtr& stmt) {
  declare(stmt->name());
  if (stmt->expr())
    resolve(stmt->expr());
  define(stmt->name());
}

void Resolver::visit(const BlockStmtPtr& stmt) {
  enter_scope();
  resolve(stmt->stmts());
  leave_scope();
}

void Resolver::visit(const IfStmtPtr& stmt) {
  resolve(stmt->cond());
  resolve(stmt->then_branch());
  if (!stmt->else_branch().empty())
    resolve(stmt->else_branch());
}

void Resolver::visit(const WhileStmtPtr& stmt) {
  resolve(stmt->cond());

  ++loops_level_;
  resolve(stmt->body());
  --loops_level_;
}

void Resolver::visit(const FunctionStmtPtr& stmt) {
  declare(stmt->name());
  define(stmt->name());
  resolve_function(stmt, FunKind::FUNCTION);
}

void Resolver::visit(const ReturnStmtPtr& stmt) {
  if (curr_fn_ == FunKind::NONE)
    throw RuntimeError(stmt->keyword(), "cannot return from top-level code");

  if (stmt->value()) {
    if (curr_fn_ == FunKind::CTOR) {
      throw RuntimeError(stmt->keyword(),
          "cannot return a value from `ctor` of a class");
    }
    resolve(stmt->value());
  }
}

void Resolver::visit(const BreakStmtPtr& stmt) {
  if (loops_level_ == 0) {
    throw RuntimeError(stmt->keyword(),
        "a `break` can only appear within the body of a loop");
  }
}

void Resolver::visit(const ClassStmtPtr& stmt) {
  ClassKind enclosing_class = curr_class_;
  curr_class_ = ClassKind::CLASS;
  const auto& superclass = stmt->superclass();

  declare(stmt->name());
  if (superclass) {
    curr_class_ = ClassKind::SUBCLASS;
    resolve(superclass);
  }
  define(stmt->name());

  if (superclass) {
    enter_scope();
    scopes_.back()["super"] = true;
  }
  enter_scope();
  scopes_.back()["self"] = true;

  for (auto& meth : stmt->methods()) {
    FunKind kind = FunKind::METHOD;
    if (meth->name().get_literal() == "ctor")
      kind = FunKind::CTOR;
    resolve_function(meth, kind);
  }
  leave_scope();
  if (superclass)
    leave_scope();

  curr_class_ = enclosing_class;
}

}
