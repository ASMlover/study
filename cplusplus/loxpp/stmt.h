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
#include "expr.h"

struct StmtVisitor;
struct Stmt;

using StmtVisitorPtr = std::shared_ptr<StmtVisitor>;
using StmtPtr = std::shared_ptr<Stmt>;

struct Stmt {
  virtual void accept(const StmtVisitorPtr& visitor) = 0;
};

class ExprStmt : public Stmt, public Visitable<ExprStmt> {
public:
  ExprPtr expr_;
public:
  ExprStmt(const ExprPtr& expr);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class PrintStmt : public Stmt, public Visitable<PrintStmt> {
public:
  ExprPtr expr_;
public:
  PrintStmt(const ExprPtr& expr);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class VarStmt : public Stmt, public Visitable<VarStmt> {
public:
  Token name_;
  ExprPtr expr_;
public:
  VarStmt(const Token& name, const ExprPtr& expr);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class BlockStmt : public Stmt, public Visitable<BlockStmt> {
public:
  std::vector<StmtPtr> stmts_;
public:
  BlockStmt(const std::vector<StmtPtr>& stmts);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class IfStmt : public Stmt, public Visitable<IfStmt> {
public:
  ExprPtr cond_;
  StmtPtr then_branch_;
  StmtPtr else_branch_;
public:
  IfStmt(const ExprPtr& cond,
      const StmtPtr& then_branch, const StmtPtr& else_branch);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class WhileStmt : public Stmt, public Visitable<WhileStmt> {
public:
  ExprPtr cond_;
  StmtPtr body_;
public:
  WhileStmt(const ExprPtr& cond, const StmtPtr& body);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class FunctionStmt : public Stmt, public Visitable<FunctionStmt> {
  using FunctionPtr = std::shared_ptr<Function>;
public:
  Token name_;
  FunctionPtr function_;
public:
  FunctionStmt(const Token& name, const FunctionPtr& function);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class ReturnStmt : public Stmt, public Visitable<ReturnStmt> {
public:
  Token keyword_;
  ExprPtr value_;
public:
  ReturnStmt(const Token& keyword, const ExprPtr& value);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class ClassStmt : public Stmt, public Visitable<ClassStmt> {
  using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;
public:
  Token name_;
  ExprPtr super_class_;
  std::vector<FunctionStmtPtr> methods_;
public:
  ClassStmt(const Token& name,
      const ExprPtr& super_class, const std::vector<FunctionStmtPtr>& methods);
  virtual void accept(const StmtVisitorPtr& visitor) override;
};

struct StmtVisitor {
  using ExprStmtPtr = std::shared_ptr<ExprStmt>;
  using PrintStmtPtr = std::shared_ptr<PrintStmt>;
  using VarStmtPtr = std::shared_ptr<VarStmt>;
  using BlockStmtPtr = std::shared_ptr<BlockStmt>;
  using IfStmtPtr = std::shared_ptr<IfStmt>;
  using WhileStmtPtr = std::shared_ptr<WhileStmt>;
  using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;
  using ReturnStmtPtr = std::shared_ptr<ReturnStmt>;
  using ClassStmtPtr = std::shared_ptr<ClassStmt>;

  virtual void visit_expr_stmt(const ExprStmtPtr& stmt) = 0;
  virtual void visit_print_stmt(const PrintStmtPtr& stmt) = 0;
  virtual void visit_var_stmt(const VarStmtPtr& stmt) = 0;
  virtual void visit_block_stmt(const BlockStmtPtr& stmt) = 0;
  virtual void visit_if_stmt(const IfStmtPtr& stmt) = 0;
  virtual void visit_while_stmt(const WhileStmtPtr& stmt) = 0;
  virtual void visit_function_stmt(const FunctionStmtPtr& stmt) = 0;
  virtual void visit_return_stmt(const ReturnStmtPtr& stmt) = 0;
  virtual void visit_class_stmt(const ClassStmtPtr& stmt) = 0;
};
