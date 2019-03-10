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
#include "../common/errors.hh"
#include "callable.hh"
#include "builtins.hh"
#include "environment.hh"
#include "interpret_helper.hh"
#include "interpreter.hh"

namespace sage {

Interpreter::Interpreter(ErrorReport& err_report)
  : err_report_(err_report)
  , globals_(new Environment())
  , environment_(globals_) {
  globals_->define("clock", Value(std::make_shared<NatClock>()));
}

void Interpreter::interpret(const ExprPtr& expression) {
  try {
    Value value = evaluate(expression);
    std::cout << value << std::endl;
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
  }
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
  try {
    for (auto& stmt : statements)
      evaluate(stmt);
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
  }
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
  EnvironmentPtr origin_environment = environment_;
  try {
    environment_ = env;
    for (auto& stmt : stmts)
      evaluate(stmt);
  }
  catch (...) {
    environment_ = origin_environment;
    throw;
  }
  environment_ = origin_environment;
}

void Interpreter::check_numeric_operand(const Token& oper, const Value& value) {
  if (value.is_numeric())
    return;
  throw RuntimeError(oper, "operand must be a numeric");
}

void Interpreter::check_numeric_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if (lvalue.is_numeric() && rvalue.is_numeric())
    return;
  throw RuntimeError(oper, "operands must be two numerics");
}

void Interpreter::check_plus_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if ((lvalue.is_numeric() && rvalue.is_numeric())
      || (lvalue.is_string() && rvalue.is_string()))
    return;
  throw RuntimeError(oper, "operands must be two numerics or two strings");
}

void Interpreter::check_modulo_operands(
    const Token& oper, const Value& lvalue, const Value& rvalue) {
  if (lvalue.is_integer() && rvalue.is_integer())
    return;
  throw RuntimeError(oper, "operands must be two integers");
}

Value Interpreter::lookup_variable(const Token& name, const ExprPtr& expr) {
  auto distance_iter = locals_.find(expr);
  if (distance_iter != locals_.end())
    return environment_->get_at(distance_iter->second, name);
  else
    return globals_->get(name);
}

void Interpreter::visit(const AssignExprPtr& expr) {
  const Token& name = expr->name();
  const Token& oper = expr->oper();
  Value left = environment_->get(name);
  Value right = evaluate(expr->value());

  switch (oper.get_kind()) {
  case TokenKind::TK_PLUSEQUAL:
    check_plus_operands(oper, left, right);
    value_ = left + right; break;
  case TokenKind::TK_MINUSEQUAL:
    check_numeric_operands(oper, left, right);
    value_ = left - right; break;
  case TokenKind::TK_STAREQUAL:
    check_numeric_operands(oper, left, right);
    value_ = left * right; break;
  case TokenKind::TK_SLASHEQUAL:
    check_numeric_operands(oper, left, right);
    value_ = left / right; break;
  case TokenKind::TK_PERCENTEQUAL:
    check_modulo_operands(oper, left, right);
    value_ = left % right; break;
  default:
    value_ = right; break;
  }

  auto distance_iter = locals_.find(expr);
  if (distance_iter != locals_.end())
    environment_->assign_at(distance_iter->second, name, value_);
  else
    globals_->assign(name, value_);
}

void Interpreter::visit(const BinaryExprPtr& expr) {
  Value left = evaluate(expr->left());
  Value right = evaluate(expr->right());
  const Token& oper = expr->oper();

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
  case TokenKind::KW_IS:
    value_ = left.is_equal(right); break;
  case TokenKind::TK_EXCLAIMEQUAL:
    value_ = left != right; break;
  case TokenKind::TK_EQUALEQUAL:
    value_ = left == right; break;
  }
}

void Interpreter::visit(const CallExprPtr& expr) {
  Value callee = evaluate(expr->callee());
  if (!callee.is_callable())
    throw RuntimeError(expr->paren(), "can only call functions and classes");

  std::vector<Value> arguments;
  for (auto& arg : expr->arguments())
    arguments.push_back(evaluate(arg));
  auto callable = callee.to_callable();
  if (callable->check_arity() && callable->arity() != arguments.size()) {
    throw RuntimeError(expr->paren(),
        "expected " + std::to_string(callable->arity()) +
        " arguments but got " + std::to_string(arguments.size()));
  }
  value_ = callable->call(shared_from_this(), arguments);
}

void Interpreter::visit(const SetExprPtr& expr) {
  Value object = evaluate(expr->object());
  if (object.is_instance()) {
    Value value = evaluate(expr->value());
    object.to_instance()->set_attribute(expr->name(), value);
  }
  else {
    throw RuntimeError(expr->name(), "only instances have attributes");
  }
}

void Interpreter::visit(const GetExprPtr& expr) {
  Value object = evaluate(expr->object());
  if (object.is_instance())
    value_ = object.to_instance()->get_attribute(expr->name());
  else
    throw RuntimeError(expr->name(), "only instances have attributes");
}

void Interpreter::visit(const GroupingExprPtr& expr) {
  evaluate(expr->expression());
}

void Interpreter::visit(const LiteralExprPtr& expr) {
  value_ = expr->value();
}

void Interpreter::visit(const LogicalExprPtr& expr) {
  Value left = evaluate(expr->left());
  if (expr->oper().get_kind() == TokenKind::KW_OR) {
    if (left.is_truthy()) {
      value_ = left;
      return;
    }
  }
  else {
    if (!left.is_truthy()) {
      value_ = left;
      return;
    }
  }
  value_ = evaluate(expr->right());
}

void Interpreter::visit(const SelfExprPtr& expr) {
  value_ = lookup_variable(expr->keyword(), expr);
}

void Interpreter::visit(const SuperExprPtr& expr) {
  int distance = 0;
  auto super_iter = locals_.find(expr);
  if (super_iter != locals_.end())
    distance = super_iter->second;

  const Token& meth = expr->method();
  ClassPtr superclass = std::static_pointer_cast<Class>(
      environment_->get_at(distance, "super").to_callable());
  InstancePtr object = std::static_pointer_cast<Instance>(
      environment_->get_at(distance - 1, "self").to_instance());
  FunctionPtr method = superclass->get_method(object, meth);

  if (!method) {
    throw RuntimeError(expr->method(),
        "undefined method `" + meth.get_literal() + "`");
  }
  value_ = method;
}

void Interpreter::visit(const UnaryExprPtr& expr) {
  Value right = evaluate(expr->right());
  const Token& oper = expr->oper();

  switch (oper.get_kind()) {
  case TokenKind::KW_NOT:
  case TokenKind::TK_EXCLAIM:
    value_ = !right.is_truthy(); break;
  case TokenKind::TK_MINUS:
    check_numeric_operand(oper, right);
    value_ = -right; break;
  }
}

void Interpreter::visit(const VariableExprPtr& expr) {
  value_ = environment_->get(expr->name());
}

void Interpreter::visit(const FunctionExprPtr& expr) {
}

void Interpreter::visit(const ExprStmtPtr& stmt) {
  evaluate(stmt->expr());
}

void Interpreter::visit(const PrintStmtPtr& stmt) {
  for (auto& expr : stmt->exprs())
    std::cout << evaluate(expr) << " ";
  std::cout << std::endl;
}

void Interpreter::visit(const LetStmtPtr& stmt) {
  Value value;
  if (stmt->expr())
    value = evaluate(stmt->expr());
  environment_->define(stmt->name(), value);
}

void Interpreter::visit(const BlockStmtPtr& stmt) {
  evaluate_block(stmt->stmts(), std::make_shared<Environment>(environment_));
}

void Interpreter::visit(const IfStmtPtr& stmt) {
  Value cond = evaluate(stmt->cond());
  if (cond.is_truthy()) {
    const auto& then_branch = stmt->then_branch();
    if (!then_branch.empty())
      evaluate_block(then_branch, std::make_shared<Environment>(environment_));
  }
  else {
    const auto& else_branch = stmt->else_branch();
    if (!else_branch.empty())
      evaluate_block(else_branch, std::make_shared<Environment>(environment_));
  }
}

void Interpreter::visit(const WhileStmtPtr& stmt) {
  EnvironmentPtr envp;
  while (evaluate(stmt->cond()).is_truthy()) {
    if (!envp)
      envp = std::make_shared<Environment>(environment_);

    try {
      evaluate_block(stmt->body(), envp);
    }
    catch (const Break&) {
      break;
    }
  }
}

void Interpreter::visit(const FunctionStmtPtr& stmt) {
  auto fn = std::make_shared<Function>(stmt, environment_, false);
  environment_->define(stmt->name(), Value(fn));
}

void Interpreter::visit(const ReturnStmtPtr& stmt) {
  Value value;
  if (stmt->value())
    value = evaluate(stmt->value());

  throw Return(value);
}

void Interpreter::visit(const BreakStmtPtr& stmt) {
  throw Break();
}

void Interpreter::visit(const ClassStmtPtr& stmt) {
  ClassPtr superclass;
  Value superval;
  const auto& superexp = stmt->superclass();
  if (superexp) {
    superval = evaluate(superexp);
    if (superval.is_callable() &&
        std::dynamic_pointer_cast<Class>(superval.to_callable())) {
      superclass = std::static_pointer_cast<Class>(superval.to_callable());
    }
    else {
      throw RuntimeError(stmt->name(), "superclass must be a class");
    }
  }

  environment_->define(stmt->name(), Value());

  if (superexp) {
    environment_ = std::make_shared<Environment>(environment_);
    environment_->define("super", superval);
  }

  std::unordered_map<std::string, FunctionPtr> methods;
  for (auto& meth : stmt->methods()) {
    bool is_ctor = meth->name().get_literal() == "ctor";
    auto method_fn = std::make_shared<Function>(meth, environment_, is_ctor);
    methods[meth->name().get_literal()] = method_fn;
  }

  auto cls = std::make_shared<Class>(
      stmt->name().get_literal(), superclass, methods);
  if (!superval.is_nil())
    environment_ = environment_->get_enclosing();
  environment_->assign(stmt->name(), Value(cls));
}

}
