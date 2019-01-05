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
#include "interpreter.h"
#include "resolver.h"

void Resolver::visit_assign_expr(const AssignPtr& expr) {
  resolve(expr->value_);
  resolve_local(expr, expr->name_);
}

void Resolver::visit_binary_expr(const BinaryPtr& expr) {
  resolve(expr->left_);
  resolve(expr->right_);
}

void Resolver::visit_call_expr(const CallPtr& expr) {
  resolve(expr->callee_);
  for (auto& arg : expr->arguments_)
    resolve(arg);
}

void Resolver::visit_get_expr(const GetPtr& expr) {
  resolve(expr->object_);
}

void Resolver::visit_set_expr(const SetPtr& expr) {
  resolve(expr->value_);
  resolve(expr->object_);
}

void Resolver::visit_grouping_expr(const GroupingPtr& expr) {
  resolve(expr->expression_);
}

void Resolver::visit_literal_expr(const LiteralPtr& expr) {
  // TODO:
}

void Resolver::visit_logical_expr(const LogicalPtr& expr) {
  resolve(expr->left_);
  resolve(expr->right_);
}

void Resolver::visit_super_expr(const SuperPtr& expr) {
  if (curr_class_ == ClassType::NONE) {
    err_report_.error(expr->keyword_,
        "cannot use `super` out of a class ...");
  }
  else if (curr_class_ != ClassType::SUBCLASS) {
    err_report_.error(expr->keyword_,
        "cannot use `super` in a class with no superclass ...");
  }

  resolve_local(expr, expr->keyword_);
}

void Resolver::visit_this_expr(const ThisPtr& expr) {
  if (curr_class_ == ClassType::NONE) {
    err_report_.error(expr->keyword_,
        "cannot use `this` outside of a class ...");
    return;
  }

  resolve_local(expr, expr->keyword_);
}

void Resolver::visit_unary_expr(const UnaryPtr& expr) {
  resolve(expr->right_);
}

void Resolver::visit_variable_expr(const VariablePtr& expr) {
  if (!scopes_.empty()) {
    auto& scope = scopes_.back();
    auto name = expr->name_.get_lexeme();
    if (scope.count(name) && !scope[name]) {
      err_report_.error(expr->name_,
        "cannot read local variable in its own initializer ...");
    }
  }

  resolve_local(expr, expr->name_);
}

void Resolver::visit_function_expr(const FunctionPtr& expr) {
  // TODO:
}

void Resolver::visit_expr_stmt(const ExprStmtPtr& stmt) {
  resolve(stmt->expr_);
}

void Resolver::visit_print_stmt(const PrintStmtPtr& stmt) {
  resolve(stmt->expr_);
}

void Resolver::visit_var_stmt(const VarStmtPtr& stmt) {
  declare(stmt->name_);
  if (stmt->expr_)
    resolve(stmt->expr_);
  define_token(stmt->name_);
}

void Resolver::visit_block_stmt(const BlockStmtPtr& stmt) {
  begin_scope();
  resolve(stmt->stmts_);
  end_scope();
}

void Resolver::visit_if_stmt(const IfStmtPtr& stmt) {
  resolve(stmt->cond_);
  resolve(stmt->then_branch_);
  if (stmt->else_branch_)
    resolve(stmt->else_branch_);
}

void Resolver::visit_while_stmt(const WhileStmtPtr& stmt) {
  resolve(stmt->cond_);
  resolve(stmt->body_);
}

void Resolver::visit_function_stmt(const FunctionStmtPtr& stmt) {
  declare(stmt->name_);
  define_token(stmt->name_);

  resolve_function(stmt, FunType::FUNCTION);
}

void Resolver::visit_return_stmt(const ReturnStmtPtr& stmt) {
  if (curr_fun_ == FunType::NONE)
    err_report_.error(stmt->keyword_, "cannot return from top-level code ...");

  if (stmt->value_) {
    if (curr_fun_ == FunType::CTOR) {
      err_report_.error(
          stmt->keyword_, "cannot return a value from an ctor ...");
    }

    resolve(stmt->value_);
  }
}

void Resolver::visit_class_stmt(const ClassStmtPtr& stmt) {
  ClassType enclosing_class = curr_class_;
  curr_class_ = ClassType::CLASS;

  declare(stmt->name_);
  if (stmt->super_class_) {
    curr_class_ = ClassType::SUBCLASS;
    resolve(stmt->super_class_);

    begin_scope();
    scopes_.back()["super"] = true;
  }

  begin_scope();
  scopes_.back()["this"] = true;
  for (auto& meth : stmt->methods_) {
    FunType ft = FunType::METHOD;
    if (meth->name_.get_lexeme() == "ctor")
      ft = FunType::CTOR;
    resolve_function(meth, ft);
  }
  end_scope();

  if (stmt->super_class_)
    end_scope();

  curr_class_ = enclosing_class;
  define_token(stmt->name_);
}

void Resolver::resolve(const ExprPtr& expr) {
  expr->accept(shared_from_this());
}

void Resolver::resolve(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Resolver::resolve(const std::vector<StmtPtr>& stmts) {
  for (auto& stmt : stmts)
    resolve(stmt);
}

void Resolver::resolve_local(const ExprPtr& expr, const Token& name) {
  int n = static_cast<int>(scopes_.size());
  for (auto i = n - 1; i >= 0; --i) {
    if (scopes_[i].count(name.get_lexeme()))
      interp_->resolve(expr, n - 1 - i);
  }
}

void Resolver::resolve_function(const FunctionStmtPtr& fun, FunType type) {
  FunType enclosing_fun = curr_fun_;
  curr_fun_ = type;

  begin_scope();
  for (auto& param : fun->function_->params_) {
    declare(param);
    define_token(param);
  }
  resolve(fun->function_->body_);
  end_scope();

  curr_fun_ = enclosing_fun;
}

void Resolver::begin_scope(void) {
  scopes_.push_back({});
}

void Resolver::end_scope(void) {
  scopes_.pop_back();
}

void Resolver::declare(const Token& name) {
  if (scopes_.empty())
    return;

  auto& scope = scopes_.back();
  if (scope.count(name.get_lexeme())) {
    err_report_.error(name,
      "variable with this name already declared in this scope ...");
  }
  scope[name.get_lexeme()] = false;
}

void Resolver::define_token(const Token& name) {
  if (scopes_.empty())
    return;

  auto& scope = scopes_.back();
  scope[name.get_lexeme()] = true;
}
