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
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "ast.h"

namespace lox {

class ErrorReport;
class Interpreter;
using InterpreterPtr = std::shared_ptr<Interpreter>;

class Resolver
  : public ExprVisitor
  , public StmtVisitor
  , public std::enable_shared_from_this<Resolver> {
  enum FunKind { NONE, FUNCTION, METHOD };

  ErrorReport& err_report_;
  InterpreterPtr interp_;
  std::vector<std::unordered_map<std::string, bool>> scopes_;
  FunKind curr_fun_{FunKind::NONE};

  void resolve(const ExprPtr& expr);
  void resolve(const std::vector<ExprPtr>& exprs);
  void resolve(const StmtPtr& stmt);
  void resolve(const std::vector<StmtPtr>& stmts);
  void resolve_local(const Token& name, const ExprPtr& expr);
  void resolve_function(const FunctionStmtPtr& fn, FunKind kind);
  void begin_scope(void);
  void finish_scope(void);
  void declare(const Token& name);
  void define(const Token& name);
public:
  Resolver(ErrorReport& err_report, const InterpreterPtr& interp)
    : err_report_(err_report)
    , interp_(interp) {
  }

  void invoke_resolve(const std::vector<StmtPtr>& stmts);

  virtual void visit_assign_expr(const AssignExprPtr& expr) override;
  virtual void visit_binary_expr(const BinaryExprPtr& expr) override;
  virtual void visit_call_expr(const CallExprPtr& expr) override;
  virtual void visit_set_expr(const SetExprPtr& expr) override;
  virtual void visit_get_expr(const GetExprPtr& expr) override;
  virtual void visit_grouping_expr(const GroupingExprPtr& expr) override;
  virtual void visit_literal_expr(const LiteralExprPtr& expr) override;
  virtual void visit_logical_expr(const LogicalExprPtr& expr) override;
  virtual void visit_self_expr(const SelfExprPtr& expr) override;
  virtual void visit_super_expr(const SuperExprPtr& expr) override;
  virtual void visit_unary_expr(const UnaryExprPtr& expr) override;
  virtual void visit_variable_expr(const VariableExprPtr& expr) override;
  virtual void visit_function_expr(const FunctionExprPtr& expr) override;

  virtual void visit_expr_stmt(const ExprStmtPtr& stmt) override;
  virtual void visit_print_stmt(const PrintStmtPtr& stmt) override;
  virtual void visit_let_stmt(const LetStmtPtr& stmt) override;
  virtual void visit_block_stmt(const BlockStmtPtr& stmt) override;
  virtual void visit_if_stmt(const IfStmtPtr& stmt) override;
  virtual void visit_while_stmt(const WhileStmtPtr& stmt) override;
  virtual void visit_function_stmt(const FunctionStmtPtr& stmt) override;
  virtual void visit_return_stmt(const ReturnStmtPtr& stmt) override;
  virtual void visit_class_stmt(const ClassStmtPtr& stmt) override;
};

}
