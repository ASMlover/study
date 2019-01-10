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
#include <iostream>
#include "errors.h"
#include "token.h"
#include "interpreter.h"

namespace lox {

Value Interpreter::evaluate(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return value_;
}

void Interpreter::evaluate(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Interpreter::check_numeric_operand(const Token& oper, const Value& value) {
  if (value.is_numeric())
    return;
  throw RuntimeError(oper, "operand must be numeric value ...");
}

void Interpreter::check_numeric_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if (lvalue.is_numeric() && rvalue.is_numeric())
    return;
  throw RuntimeError(oper, "operands must be numeric values ...");
}

void Interpreter::check_plus_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if ((lvalue.is_numeric() && rvalue.is_numeric())
      || (lvalue.is_string() && rvalue.is_string()))
    return;
  throw RuntimeError(oper, "operands must be two numerics or two strings ...");
}

void Interpreter::check_modulo_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if (lvalue.is_integer() && rvalue.is_integer())
    return;
  throw RuntimeError(oper, "operands must be two integers ...");
}

void Interpreter::visit_assign_expr(const AssignExprPtr& expr) {
}

void Interpreter::visit_binary_expr(const BinaryExprPtr& expr) {
  const Token& oper = expr->oper();
  Value left = evaluate(expr->left());
  Value right = evaluate(expr->right());

  switch (oper.get_kind()) {
  case TokenKind::TK_PLUS:
    check_plus_operands(oper, left, right);
    value_ = left + right; break;
  case TokenKind::TK_MINUS:
    check_numeric_operands(oper, left, right);
    value_ = left - right; break;
  case TokenKind::TK_STAR:
    check_numeric_operands(oper, left, right);
    value_ = left * right; break;
  case TokenKind::TK_SLASH:
    check_numeric_operands(oper, left, right);
    value_ = left / right; break;
  case TokenKind::TK_PERCENT:
    check_modulo_operands(oper, left, right);
    value_ = left % right; break;
  case TokenKind::TK_GREATER:
    check_numeric_operands(oper, left, right);
    value_ = left > right; break;
  case TokenKind::TK_GREATEREQUAL:
    check_numeric_operands(oper, left, right);
    value_ = left >= right; break;
  case TokenKind::TK_LESS:
    check_numeric_operands(oper, left, right);
    value_ = left < right; break;
  case TokenKind::TK_LESSEQUAL:
    check_numeric_operands(oper, left, right);
    value_ = left <= right; break;
  case TokenKind::TK_EXCLAIMEQUAL:
    value_ = left != right; break;
  case TokenKind::TK_EQUALEQUAL:
    value_ = left == right; break;
  case TokenKind::KW_IS:
    value_ = expr->left() == expr->right(); break;
  }
}

void Interpreter::visit_call_expr(const CallExprPtr& expr) {
}

void Interpreter::visit_set_expr(const SetExprPtr& expr) {
}

void Interpreter::visit_get_expr(const GetExprPtr& expr) {
}

void Interpreter::visit_grouping_expr(const GroupingExprPtr& expr) {
  evaluate(expr->expression());
}

void Interpreter::visit_literal_expr(const LiteralExprPtr& expr) {
  value_ = expr->value();
}

void Interpreter::visit_logical_expr(const LogicalExprPtr& expr) {
}

void Interpreter::visit_self_expr(const SelfExprPtr& expr) {
}

void Interpreter::visit_super_expr(const SuperExprPtr& expr) {
}

void Interpreter::visit_unary_expr(const UnaryExprPtr& expr) {
  Value right = evaluate(expr->right());
  switch (expr->oper().get_kind()) {
  case TokenKind::TK_MINUS:
    value_ = -right; break;
  case TokenKind::TK_EXCLAIM:
  case TokenKind::KW_NOT:
    value_ = !right; break;
  }
}

void Interpreter::visit_variable_expr(const VariableExprPtr& expr) {
}

void Interpreter::visit_function_expr(const FunctionExprPtr& expr) {
}

void Interpreter::visit_expr_stmt(const ExprStmtPtr& stmt) {
  evaluate(stmt->expr());
}

void Interpreter::visit_print_stmt(const PrintStmtPtr& stmt) {
  auto value = evaluate(stmt->expr());
  std::cout << value << std::endl;
}

void Interpreter::visit_let_stmt(const LetStmtPtr& stmt) {
}

void Interpreter::visit_block_stmt(const BlockStmtPtr& stmt) {
}

void Interpreter::visit_if_stmt(const IfStmtPtr& stmt) {
}

void Interpreter::visit_while_stmt(const WhileStmtPtr& stmt) {
}

void Interpreter::visit_function_stmt(const FunctionStmtPtr& stmt) {
}

void Interpreter::visit_return_stmt(const ReturnStmtPtr& stmt) {
}

void Interpreter::visit_class_stmt(const ClassStmtPtr& stmt) {
}

void Interpreter::interpret(const ExprPtr& expression) {
  try {
    auto value = evaluate(expression);
    std::cout << value << std::endl;
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
  }
}

void Interpreter::interpret(const std::vector<StmtPtr>& stmts) {
  try {
    for (auto& stmt : stmts)
      evaluate(stmt);
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
  }
}

}
