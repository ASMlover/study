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
#include <unordered_map>
#include "../common/value.hh"
#include "../syntax/ast.hh"

namespace sage {

class ErrorReport;
class Environment;

using EnvironmentPtr = std::shared_ptr<Environment>;

class Interpreter
  : public ExprVisitor
  , public StmtVisitor
  , public std::enable_shared_from_this<Interpreter> {
  ErrorReport& err_report_;
  Value value_{};
  EnvironmentPtr globals_;
  EnvironmentPtr environment_;
  std::unordered_map<ExprPtr, int> locals_;

  Value evaluate(const ExprPtr& expr);
  void evaluate(const StmtPtr& stmt);
  void evaluate_block(
      const std::vector<StmtPtr>& stmts, const EnvironmentPtr& env);
  void check_numeric_operand(const Token& oper, const Value& value);
  void check_numeric_operands(
      const Token& oper, const Value& lvalue, const Value& rvalue);
  void check_plus_operands(
      const Token& oper, const Value& lvalue, const Value& rvalue);
  void check_modulo_operands(
      const Token& oper, const Value& lvalue, const Value& rvalue);
  Value lookup_variable(const Token& name, const ExprPtr& expr);

  virtual void visit(const AssignExprPtr& expr) override;
  virtual void visit(const BinaryExprPtr& expr) override;
  virtual void visit(const CallExprPtr& expr) override;
  virtual void visit(const SetExprPtr& expr) override;
  virtual void visit(const GetExprPtr& expr) override;
  virtual void visit(const GroupingExprPtr& expr) override;
  virtual void visit(const LiteralExprPtr& expr) override;
  virtual void visit(const LogicalExprPtr& expr) override;
  virtual void visit(const SelfExprPtr& expr) override;
  virtual void visit(const SuperExprPtr& expr) override;
  virtual void visit(const UnaryExprPtr& expr) override;
  virtual void visit(const VariableExprPtr& expr) override;
  virtual void visit(const FunctionExprPtr& expr) override;

  virtual void visit(const ExprStmtPtr& stmt) override;
  virtual void visit(const PrintStmtPtr& stmt) override;
  virtual void visit(const LetStmtPtr& stmt) override;
  virtual void visit(const BlockStmtPtr& stmt) override;
  virtual void visit(const IfStmtPtr& stmt) override;
  virtual void visit(const WhileStmtPtr& stmt) override;
  virtual void visit(const FunctionStmtPtr& stmt) override;
  virtual void visit(const ReturnStmtPtr& stmt) override;
  virtual void visit(const BreakStmtPtr& stmt) override;
  virtual void visit(const ClassStmtPtr& stmt) override;
public:
  Interpreter(ErrorReport& err_report);

  void interpret(const ExprPtr& expression);
  void interpret(const std::vector<StmtPtr>& statements);

  EnvironmentPtr get_globals(void) const {
    return globals_;
  }

  void invoke_evaluate_block(
      const std::vector<StmtPtr>& stmts, const EnvironmentPtr& envp) {
    evaluate_block(stmts, envp);
  }

  void resolve(const ExprPtr& expr, int depth) {
    locals_[expr] = depth;
  }
};

}
