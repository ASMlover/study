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
#include "interpret_errors.hh"
#include "interpret_interpreter.hh"
#include "interpret_resolver.hh"

namespace loxcc::interpret {

void Resolver::invoke_resolve(const StmtPtr& stmt) {
  try {
    resolve(stmt);
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.token(), e.message());
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
  int n = Xt::as_type<int>(scopes_.size()) - 1;
  for (int i = n; i >= 0; --i) {
    auto& scope = scopes_[i];
    if (scope.find(name.as_string()) != scope.end()) {
      interp_->resolve(expr, n - i);
      return;
    }
  }
  // not found, assume it is global
}

void Resolver::resolve_function(const FunctionStmtPtr& fn, FunKind kind) {
  FunKind encloding_fun = curr_fun_;
  curr_fun_ = kind;

  enter_scope();
  for (auto& param : fn->params()) {
    declare(param);
    define(param);
  }
  resolve(fn->body());
  leave_scope();

  curr_fun_ = encloding_fun;
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
    str_t var_name = name.as_string();
    if (scope.find(var_name) != scope.end()) {
      throw RuntimeError(name,
          "variable `" + var_name + "` already declared in thie scope");
    }
    else {
      scope[var_name] = false;
    }
  }
}

void Resolver::define(const Token& name) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    scope[name.as_string()] = true;
  }
}

void Resolver::visit(const AssignExprPtr& expr) {
  resolve(expr->value());
  resolve_local(expr->name(), expr);
}

void Resolver::visit(const SetExprPtr& expr) {
  resolve(expr->value());
  resolve(expr->object());
}

void Resolver::visit(const LogicalExprPtr& expr) {
  resolve(expr->lhs());
  resolve(expr->rhs());
}

void Resolver::visit(const BinaryExprPtr& expr) {
  resolve(expr->lhs());
  resolve(expr->rhs());
}

void Resolver::visit(const UnaryExprPtr& expr) {
  resolve(expr->rhs());
}

void Resolver::visit(const CallExprPtr& expr) {
  resolve(expr->callee());
  resolve(expr->arguments());
}

void Resolver::visit(const GetExprPtr& expr) {
  resolve(expr->object());
}

void Resolver::visit(const LiteralExprPtr& expr) {
}

void Resolver::visit(const GroupingExprPtr& expr) {
  resolve(expr->expression());
}

void Resolver::visit(const SuperExprPtr& expr) {
  if (curr_cls_ == ClassKind::NONE) {
    throw RuntimeError(expr->keyword(),
        "cannot use `super` outside of a class");
  }
  else if (curr_cls_ != ClassKind::SUBCLASS) {
    throw RuntimeError(expr->keyword(),
        "cannot use `super` in a class without superclass");
  }

  resolve_local(expr->keyword(), expr);
}

void Resolver::visit(const ThisExprPtr& expr) {
  if (curr_cls_ != ClassKind::CLASS && curr_cls_ != ClassKind::SUBCLASS)
    throw RuntimeError(expr->keyword(), "cannot use `this` outside of a class");

  resolve_local(expr->keyword(), expr);
}

void Resolver::visit(const VariableExprPtr& expr) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    str_t var_name = expr->name().as_string();
    if (scope.find(var_name) != scope.end() && !scope[var_name]) {
      throw RuntimeError(expr->name(),
          "cannot read local variable `" +
          var_name + "` in its own initializer");
    }
  }

  resolve_local(expr->name(), expr);
}

void Resolver::visit(const FunctionExprPtr& expr) {
}

void Resolver::visit(const ClassStmtPtr& stmt) {
  ClassKind encloding_cls = curr_cls_;
  curr_cls_ = ClassKind::CLASS;
  const ExprPtr& superclass = stmt->superclass();

  declare(stmt->name());
  if (superclass) {
    curr_cls_ = ClassKind::SUBCLASS;
    resolve(superclass);
  }
  define(stmt->name());

  if (superclass) {
    enter_scope();
    scopes_.back()["super"] = true;
  }

  enter_scope();
  scopes_.back()["this"] = true;
  for (auto& meth_stmt : stmt->methods()) {
    FunKind kind = FunKind::METHOD;
    if (meth_stmt->name().as_string() == "ctor")
      kind = FunKind::CTOR;
    resolve_function(meth_stmt, kind);
  }
  leave_scope();

  if (superclass)
    leave_scope();

  curr_cls_ = encloding_cls;
}

void Resolver::visit(const FunctionStmtPtr& stmt) {
  declare(stmt->name());
  define(stmt->name());
  resolve_function(stmt, FunKind::FUNCTION);
}

void Resolver::visit(const VarStmtPtr& stmt) {
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

void Resolver::visit(const ExprStmtPtr& stmt) {
  resolve(stmt->expr());
}

void Resolver::visit(const WhileStmtPtr& stmt) {
  resolve(stmt->cond());
  resolve(stmt->body());
}

void Resolver::visit(const IfStmtPtr& stmt) {
  resolve(stmt->cond());
  resolve(stmt->then_branch());
  if (stmt->else_branch())
    resolve(stmt->else_branch());
}

void Resolver::visit(const PrintStmtPtr& stmt) {
  resolve(stmt->exprs());
}

void Resolver::visit(const ReturnStmtPtr& stmt) {
  if (curr_fun_ == FunKind::NONE)
    throw RuntimeError(stmt->keyword(), "cannot return from top-level code");

  if (stmt->value()) {
    if (curr_fun_ == FunKind::CTOR) {
      throw RuntimeError(stmt->keyword(),
          "cannot return a value from `ctor` of class");
    }
    resolve(stmt->value());
  }
}

}
