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
#include "errors.h"
#include "interpreter.h"
#include "resolver.h"

namespace lox {

void Resolver::visit_assign_expr(const AssignExprPtr& expr) {
  resolve(expr->value());
  resolve_local(expr->name(), expr);
}

void Resolver::visit_binary_expr(const BinaryExprPtr& expr) {
  resolve(expr->left());
  resolve(expr->right());
}

void Resolver::visit_call_expr(const CallExprPtr& expr) {
  resolve(expr->callee());
  resolve(expr->arguments());
}

void Resolver::visit_set_expr(const SetExprPtr& expr) {
}

void Resolver::visit_get_expr(const GetExprPtr& expr) {
  resolve(expr->object());
}

void Resolver::visit_grouping_expr(const GroupingExprPtr& expr) {
  resolve(expr->expression());
}

void Resolver::visit_literal_expr(const LiteralExprPtr& expr) {
}

void Resolver::visit_logical_expr(const LogicalExprPtr& expr) {
  resolve(expr->left());
  resolve(expr->right());
}

void Resolver::visit_self_expr(const SelfExprPtr& expr) {
}

void Resolver::visit_super_expr(const SuperExprPtr& expr) {
}

void Resolver::visit_unary_expr(const UnaryExprPtr& expr) {
  resolve(expr->right());
}

void Resolver::visit_variable_expr(const VariableExprPtr& expr) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    std::string name = expr->name().get_literal();
    if (scope.find(name) != scope.end() && !scope[name]) {
      throw RuntimeError(expr->name(),
          "cannot read local variable in its own initializer ...");
    }
  }
  resolve_local(expr->name(), expr);
}

void Resolver::visit_function_expr(const FunctionExprPtr& expr) {
}

void Resolver::visit_expr_stmt(const ExprStmtPtr& stmt) {
  resolve(stmt->expr());
}

void Resolver::visit_print_stmt(const PrintStmtPtr& stmt) {
  resolve(stmt->exprs());
}

void Resolver::visit_let_stmt(const LetStmtPtr& stmt) {
  declare(stmt->name());
  if (stmt->expr())
    resolve(stmt->expr());
  define(stmt->name());
}

void Resolver::visit_block_stmt(const BlockStmtPtr& stmt) {
  begin_scope();
  resolve(stmt->stmts());
  finish_scope();
}

void Resolver::visit_if_stmt(const IfStmtPtr& stmt) {
  resolve(stmt->cond());
  resolve(stmt->then_branch());
  if (stmt->else_branch())
    resolve(stmt->else_branch());
}

void Resolver::visit_while_stmt(const WhileStmtPtr& stmt) {
  resolve(stmt->cond());
  resolve(stmt->body());
}

void Resolver::visit_function_stmt(const FunctionStmtPtr& stmt) {
  declare(stmt->name());
  define(stmt->name());
  resolve_function(stmt, FunKind::FUNCTION);
}

void Resolver::visit_return_stmt(const ReturnStmtPtr& stmt) {
  if (curr_fun_ == FunKind::NONE) {
    throw RuntimeError(stmt->keyword(),
        "cannot return from top-level code ...");
  }

  if (stmt->value())
    resolve(stmt->value());
}

void Resolver::visit_class_stmt(const ClassStmtPtr& stmt) {
  declare(stmt->name());
  define(stmt->name());
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
      interp_->resolve(expr, n - i);
      return;
    }
  }

  // not found, assume it is global
}

void Resolver::resolve_function(const FunctionStmtPtr& fn, FunKind kind) {
  FunKind enclosing_fun = curr_fun_;
  curr_fun_ = kind;

  begin_scope();
  for (auto& param : fn->params()) {
    declare(param);
    define(param);
  }
  resolve(fn->body());
  finish_scope();

  curr_fun_ = enclosing_fun;
}

void Resolver::begin_scope(void) {
  scopes_.push_back({});
}

void Resolver::finish_scope(void) {
  scopes_.pop_back();
}

void Resolver::declare(const Token& name) {
  if (scopes_.empty())
    return;

  auto& scope = scopes_.back();
  if (scope.find(name.get_literal()) != scope.end()) {
    throw RuntimeError(name,
        "variable with this name already declared in this scope ...");
  }
  scope[name.get_literal()] = false;
}

void Resolver::define(const Token& name) {
  if (scopes_.empty())
    return;

  auto& scope = scopes_.back();
  scope[name.get_literal()] = true;
}

void Resolver::invoke_resolve(const std::vector<StmtPtr>& stmts) {
  try {
    resolve(stmts);
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
  }
}

}
