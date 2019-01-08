// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "error_reporter.h"
#include "value.h"
#include "native_function.h"
#include "expr.h"
#include "stmt.h"
#include "environment.h"

class Interpreter
  : public Visitor
  , public StmtVisitor
  , public std::enable_shared_from_this<Interpreter> {
  using EnvironmentPtr = std::shared_ptr<Environment>;

  ErrorReporter& err_report_;
  Value value_;
  EnvironmentPtr globals_;
  EnvironmentPtr environment_;
  std::unordered_map<ExprPtr, int> locals_;

  Value evaluate(const ExprPtr& expr);
  void evaluate(const StmtPtr& stmt);
  void evaluate_block(
      const std::vector<StmtPtr>& stmts, const EnvironmentPtr& environment);
  Value lookup_variable(const Token& name, const ExprPtr& expr);
  void check_numeric_operand(const Token& oper, const Value& operand);
  void check_numeric_operands(
      const Token& oper, const Value& left, const Value& right);
public:
  Interpreter(ErrorReporter& err_report)
    : err_report_(err_report)
    , globals_(std::make_shared<Environment>(nullptr))
    , environment_(globals_) {
    globals_->define_var("clock", Value(std::make_shared<ClockFunction>()));
    globals_->define_var("write", Value(std::make_shared<WriteFunction>()));
  }

  EnvironmentPtr get_globals(void) const {
    return globals_;
  }

  EnvironmentPtr get_envp(void) const {
    return environment_;
  }

  void invoke_evaluate_block(
      const std::vector<StmtPtr>& stmts, const EnvironmentPtr& environment) {
    evaluate_block(stmts, environment);
  }

  virtual void visit_assign_expr(const AssignPtr& expr) override;
  virtual void visit_binary_expr(const BinaryPtr& expr) override;
  virtual void visit_call_expr(const CallPtr& expr) override;
  virtual void visit_get_expr(const GetPtr& expr) override;
  virtual void visit_set_expr(const SetPtr& expr) override;
  virtual void visit_grouping_expr(const GroupingPtr& expr) override;
  virtual void visit_literal_expr(const LiteralPtr& expr) override;
  virtual void visit_logical_expr(const LogicalPtr& expr) override;
  virtual void visit_super_expr(const SuperPtr& expr) override;
  virtual void visit_this_expr(const ThisPtr& expr) override;
  virtual void visit_unary_expr(const UnaryPtr& expr) override;
  virtual void visit_variable_expr(const VariablePtr& expr) override;
  virtual void visit_function_expr(const FunctionPtr& expr) override;

  virtual void visit_expr_stmt(const ExprStmtPtr& stmt) override;
  virtual void visit_print_stmt(const PrintStmtPtr& stmt) override;
  virtual void visit_var_stmt(const VarStmtPtr& stmt) override;
  virtual void visit_block_stmt(const BlockStmtPtr& stmt) override;
  virtual void visit_if_stmt(const IfStmtPtr& stmt) override;
  virtual void visit_while_stmt(const WhileStmtPtr& stmt) override;
  virtual void visit_function_stmt(const FunctionStmtPtr& stmt) override;
  virtual void visit_return_stmt(const ReturnStmtPtr& stmt) override;
  virtual void visit_class_stmt(const ClassStmtPtr& stmt) override;

  void interpret(const ExprPtr& expr);
  void interpret(const std::vector<StmtPtr>& stmts);
  void resolve(const ExprPtr& expr, int depth);
};
