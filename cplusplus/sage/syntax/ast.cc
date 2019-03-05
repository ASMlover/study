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
#include "ast.hh"

namespace sage {

void AssignExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void BinaryExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void CallExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void SetExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void GetExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void GroupingExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void LiteralExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void LogicalExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void SelfExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void SuperExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void UnaryExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void VariableExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void FunctionExpr::accept(const ExprVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void ExprStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void PrintStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void LetStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void BlockStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void IfStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void WhileStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void FunctionStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void ReturnStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void BreakStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

void ClassStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit(shared_from_this());
}

}
