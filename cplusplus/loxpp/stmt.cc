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
#include "stmt.h"

ExprStmt::ExprStmt(const ExprPtr& expr)
  : expr_(expr) {
}

void ExprStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_expr_stmt(shared_from_this());
}

PrintStmt::PrintStmt(const ExprPtr& expr)
  : expr_(expr) {
}

void PrintStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_print_stmt(shared_from_this());
}

VarStmt::VarStmt(const Token& name, const ExprPtr& expr)
  : name_(name)
  , expr_(expr) {
}

void VarStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_var_stmt(shared_from_this());
}

BlockStmt::BlockStmt(const std::vector<StmtPtr>& stmts)
  : stmts_(stmts) {
}

void BlockStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_block_stmt(shared_from_this());
}

IfStmt::IfStmt(const ExprPtr& cond,
    const StmtPtr& then_branch, const StmtPtr& else_branch)
  : cond_(cond)
  , then_branch_(then_branch)
  , else_branch_(else_branch) {
}

void IfStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_if_stmt(shared_from_this());
}

WhileStmt::WhileStmt(const ExprPtr& cond, const StmtPtr& body)
  : cond_(cond)
  , body_(body) {
}

void WhileStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_while_stmt(shared_from_this());
}

FunctionStmt::FunctionStmt(const Token& name, const FunctionPtr& function)
  : name_(name)
  , function_(function) {
}

void FunctionStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_function_stmt(shared_from_this());
}

ReturnStmt::ReturnStmt(const Token& keyword, const ExprPtr& value)
  : keyword_(keyword)
  , value_(value) {
}

void ReturnStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_return_stmt(shared_from_this());
}

ClassStmt::ClassStmt(const Token& name,
    const ExprPtr& super_class, const std::vector<FunctionStmtPtr>& methods)
  : name_(name)
  , super_class_(super_class)
  , methods_(methods) {
}

void ClassStmt::accept(const StmtVisitorPtr& visitor) {
  visitor->visit_class_stmt(shared_from_this());
}
