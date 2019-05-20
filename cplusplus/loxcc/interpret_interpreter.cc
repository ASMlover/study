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
#include <exception>
#include "interpret_errors.hh"
#include "interpret_interpreter.hh"

namespace loxcc::interpret {

class Return : public std::exception {
  Value value_;
public:
  Return(const Value& v) noexcept : value_(v) {}
  inline const Value& value(void) const { return value_; }
};

Interpreter::Interpreter(ErrorReport& err_report) noexcept
  : err_report_(err_report)
  , globals_()
  , environment_(globals_) {
  // TODO:
}

void Interpreter::interpret(const std::vector<StmtPtr>& stmts) {
  // TODO:
}

Value Interpreter::evaluate(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return value_;
}

void Interpreter::evaluate(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Interpreter::evaluate_block(
    const std::vector<StmtPtr>& stmts, const EnvironmentPtr& env) {
  // TODO:
}

void Interpreter::check_numeric(const Token& oper, const Value& value) {
  // TODO:
}
void Interpreter::check_numerics(
    const Token& oper, const Value& lhs, const Value& rhs) {
  // TODO:
}

Value lookup_variable(const Token& name, const ExprPtr& expr) {
  // TODO:
  return nullptr;
}

void Interpreter::visit(const AssignExprPtr& expr) {
}

void Interpreter::visit(const SetExprPtr& expr) {
}

void Interpreter::visit(const LogicalExprPtr& expr) {
}

void Interpreter::visit(const BinaryExprPtr& expr) {
}

void Interpreter::visit(const UnaryExprPtr& expr) {
}

void Interpreter::visit(const CallExprPtr& expr) {
}

void Interpreter::visit(const GetExprPtr& expr) {
}

void Interpreter::visit(const LiteralExprPtr& expr) {
}

void Interpreter::visit(const GroupingExprPtr& expr) {
}

void Interpreter::visit(const SuperExprPtr& expr) {
}

void Interpreter::visit(const ThisExprPtr& expr) {
}

void Interpreter::visit(const VariableExprPtr& expr) {
}

void Interpreter::visit(const FunctionExprPtr& expr) {
}

void Interpreter::visit(const ClassStmtPtr& stmt) {
}

void Interpreter::visit(const FunctionStmtPtr& stmt) {
}

void Interpreter::visit(const VarStmtPtr& stmt) {
}

void Interpreter::visit(const BlockStmtPtr& stmt) {
}

void Interpreter::visit(const ExprStmtPtr& stmt) {
}

void Interpreter::visit(const WhileStmtPtr& stmt) {
}

void Interpreter::visit(const IfStmtPtr& stmt) {
}

void Interpreter::visit(const PrintStmtPtr& stmt) {
}

void Interpreter::visit(const ReturnStmtPtr& stmt) {
}

}
