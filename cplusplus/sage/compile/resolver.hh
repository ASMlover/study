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
#include "../syntax/ast.hh"

namespace sage {

class ErrorReport;
class Interpreter;
using InterpreterPtr = std::shared_ptr<Interpreter>;

class Resolver
  : public ExprVisitor
  , public StmtVisitor
  , public std::enable_shared_from_this<Resolver> {
  enum class FunKind { NONE, FUNCTION, CTOR, METHOD };
  enum class ClassKind { NONE, CLASS, SUBCLASS };

  ErrorReport& err_report_;
  InterpreterPtr interp_;
  int loops_level_{0};
  std::vector<std::unordered_map<std::string, bool>> scopes_;
  FunKind curr_fn_{FunKind::NONE};
  ClassKind curr_class_{ClassKind::NONE};

  void resolve(const ExprPtr& expr);
  void resolve(const std::vector<ExprPtr>& exprs);
  void resolve(const StmtPtr& stmt);
  void resolve(const std::vector<StmtPtr>& stmts);
  void resolve_local(const Token& name, const ExprPtr& expr);
  void resolve_function(const FunctionStmtPtr& fn, FunKind kind);

  void enter_scope(void);
  void leave_scope(void);
  void declare(const Token& name);
  void define(const Token& name);

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
  Resolver(ErrorReport& err_report, const InterpreterPtr& interp);

  void invoke_resolve(const std::vector<StmtPtr>& stmts);
};

}
