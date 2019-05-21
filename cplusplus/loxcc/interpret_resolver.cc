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
#include "interpret_errors.hh"
#include "interpret_interpreter.hh"
#include "interpret_resolver.hh"

namespace loxcc::interpret {

void Resolver::invoke_resolve(const std::vector<StmtPtr>& stmts) {
}

void Resolver::resolve(const ExprPtr& expr) {
}

void Resolver::resolve(const std::vector<ExprPtr>& exprs) {
}

void Resolver::resolve(const StmtPtr& stmt) {
}

void Resolver::resolve(const std::vector<StmtPtr>& stmts) {
}

void Resolver::resolve_local(const Token& name, const ExprPtr& expr) {
}

void Resolver::resolve_function(const FunctionStmtPtr& fn, FunKind kind) {
}

void Resolver::enter_scope(void) {
}

void Resolver::leave_scope(void) {
}

void Resolver::declare(const Token& name) {
}

void Resolver::define(const Token& name) {
}

void Resolver::visit(const AssignExprPtr& expr) {
}

void Resolver::visit(const SetExprPtr& expr) {
}

void Resolver::visit(const LogicalExprPtr& expr) {
}

void Resolver::visit(const BinaryExprPtr& expr) {
}

void Resolver::visit(const UnaryExprPtr& expr) {
}

void Resolver::visit(const CallExprPtr& expr) {
}

void Resolver::visit(const GetExprPtr& expr) {
}

void Resolver::visit(const LiteralExprPtr& expr) {
}

void Resolver::visit(const GroupingExprPtr& expr) {
}

void Resolver::visit(const SuperExprPtr& expr) {
}

void Resolver::visit(const ThisExprPtr& expr) {
}

void Resolver::visit(const VariableExprPtr& expr) {
}

void Resolver::visit(const FunctionExprPtr& expr) {
}

void Resolver::visit(const ClassStmtPtr& stmt) {
}

void Resolver::visit(const FunctionStmtPtr& stmt) {
}

void Resolver::visit(const VarStmtPtr& stmt) {
}

void Resolver::visit(const BlockStmtPtr& stmt) {
}

void Resolver::visit(const ExprStmtPtr& stmt) {
}

void Resolver::visit(const WhileStmtPtr& stmt) {
}

void Resolver::visit(const IfStmtPtr& stmt) {
}

void Resolver::visit(const PrintStmtPtr& stmt) {
}

void Resolver::visit(const ReturnStmtPtr& stmt) {
}

}
