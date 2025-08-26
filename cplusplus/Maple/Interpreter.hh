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

#include <unordered_map>
#include "Types.hh"
#include "ValueHelper.hh"
#include "Parser.hh"

namespace ms {

class Interpreter final : public Expr::Visitor, public Stmt::Visitor {
  struct ReturnValue {
    Value value;

    ReturnValue(Value v) : value{std::move(v)} {}
  };

  EnvironmentPtr globals_;
  EnvironmentPtr environment_;
  std::unordered_map<str_t, ModulePtr> loaded_modules_;

  Value lookup_variable(const Token& name, const Expr* expr) noexcept;
  void execute_block(const std::vector<StmtPtr>& statements, EnvironmentPtr new_env);

  void execute(const Stmt& stmt) noexcept;
  Value evaluate(const Expr& expr) noexcept;

  void check_number_operand(const Token& op, const Value& operand);
  void check_number_operands(const Token& op, const Value& left, const Value& right);

  void collect_exportes(ModulePtr module, EnvironmentPtr module_env) noexcept;
public:
  virtual Value visit(const LiteralExpr&) override;
  virtual Value visit(const GroupingExpr&) override;
  virtual Value visit(const UnaryExpr&) override;
  virtual Value visit(const BinayExpr&) override;
  virtual Value visit(const LogicalExpr&) override;
  virtual Value visit(const VariableExpr&) override;
  virtual Value visit(const AssignExpr&) override;
  virtual Value visit(const CallExpr&) override;
  virtual Value visit(const GetExpr&) override;
  virtual Value visit(const SetExpr&) override;
  virtual Value visit(const ThisExpr&) override;
  virtual Value visit(const SuperExpr&) override;

  virtual void visit(const ExpressionStmt&) override;
  virtual void visit(const PrintStmt&) override;
  virtual void visit(const VarStmt&) override;
  virtual void visit(const BlockStmt&) override;
  virtual void visit(const IfStmt&) override;
  virtual void visit(const WhileStmt&) override;
  virtual void visit(const FunctionStmt&) override;
  virtual void visit(const ReturnStmt&) override;
  virtual void visit(const ClassStmt&) override;
  virtual void visit(const ImportStmt&) override;
public:
  Interpreter() noexcept;

  void interpret(const std::vector<StmtPtr>& statements) noexcept;
};

}
