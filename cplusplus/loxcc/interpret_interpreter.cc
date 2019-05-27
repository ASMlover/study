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
#include "lexer.hh"
#include "interpret_errors.hh"
#include "interpret_parser.hh"
#include "interpret_environment.hh"
#include "interpret_callable.hh"
#include "interpret_builtins.hh"
#include "interpret_resolver.hh"
#include "interpret_interpreter.hh"

namespace loxcc::interpret {

Interpreter::Interpreter(ErrorReport& err_report) noexcept
  : err_report_(err_report)
  , globals_(new Environment())
  , environment_(globals_) {
  globals_->define("clock", Value(std::make_shared<NatClock>()));
}

void Interpreter::interpret(const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Parser parser(err_report_, lex);
  auto resolver = std::make_shared<Resolver>(err_report_, shared_from_this());

  for (;;) {
    StmtPtr stmt = parser.parse();
    if (err_report_.had_error())
      std::exit(-1);
    if (!stmt)
      break;

    resolver->invoke_resolve(stmt);
    if (err_report_.had_error())
      std::exit(-2);

    try {
      evaluate(stmt);
    }
    catch (const RuntimeError& e) {
      err_report_.error(e.token(), e.message());
    }
    if (err_report_.had_error())
      std::exit(-3);
  }
}

Value Interpreter::evaluate(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return value_;
}

void Interpreter::evaluate(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Interpreter::evaluate(
    const std::vector<StmtPtr>& stmts, const EnvironmentPtr& env) {
  auto orig_env = environment_;
  try {
    environment_ = env;
    for (auto& stmt : stmts)
      evaluate(stmt);
  }
  catch (...) {
    environment_ = orig_env;
    throw;
  }
  environment_ = orig_env;
}

void Interpreter::check_numeric(const Token& oper, const Value& value) {
  if (value.is_numeric())
    return;
  throw RuntimeError(oper, "operand must be a numeric");
}

void Interpreter::check_numerics(
    const Token& oper, const Value& lhs, const Value& rhs) {
  if (lhs.is_numeric() && rhs.is_numeric())
    return;
  throw RuntimeError(oper, "operands must be two numerics");
}

void Interpreter::check_plus(
    const Token& oper, const Value& lhs, const Value& rhs) {
  if ((lhs.is_string() && rhs.is_string())
      || (lhs.is_numeric() && rhs.is_numeric()))
    return;
  throw RuntimeError(oper, "operands must be two numerics or two strings");
}

Value Interpreter::lookup_variable(const Token& name, const ExprPtr& expr) {
  if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
    return environment_->get_at(distance_iter->second, name);
  else
    return globals_->get(name);
}

void Interpreter::visit(const AssignExprPtr& expr) {
  Value value = evaluate(expr->value());
  if (auto distance_iter = locals_.find(expr); distance_iter != locals_.end())
    environment_->assign_at(distance_iter->second, expr->name(), value);
  else
    environment_->assign(expr->name(), value);
}

void Interpreter::visit(const SetExprPtr& expr) {
  if (auto object = evaluate(expr->object()); object.is_instance()) {
    Value value = evaluate(expr->value());
    object.as_instance()->set_attr(expr->name(), value);
  }
  else {
    throw RuntimeError(expr->name(), "only instances have attributes");
  }
}

void Interpreter::visit(const LogicalExprPtr& expr) {
  Value lhs = evaluate(expr->lhs());
  if (expr->oper().kind() == TokenKind::KW_OR) {
    if (lhs.is_truthy()) {
      value_ = lhs;
      return;
    }
  }
  else {
    if (!lhs.is_truthy()) {
      value_ = lhs;
      return;
    }
  }
  value_ = evaluate(expr->rhs());
}

void Interpreter::visit(const BinaryExprPtr& expr) {
  Value lhs = evaluate(expr->lhs());
  Value rhs = evaluate(expr->rhs());
  const Token& oper = expr->oper();

#define BINARYOP(op) do {\
  check_numerics(oper, lhs, rhs);\
  value_ = lhs op rhs;\
} while (false)

  switch (oper.kind()) {
  case TokenKind::TK_PLUS:
    check_plus(oper, lhs, rhs); value_ = lhs + rhs; break;
  case TokenKind::TK_MINUS: BINARYOP(-); break;
  case TokenKind::TK_STAR: BINARYOP(*); break;
  case TokenKind::TK_SLASH: BINARYOP(/); break;
  case TokenKind::TK_GT: BINARYOP(>); break;
  case TokenKind::TK_GTEQ: BINARYOP(>=); break;
  case TokenKind::TK_LT: BINARYOP(<); break;
  case TokenKind::TK_LTEQ: BINARYOP(<=); break;
  case TokenKind::TK_BANGEQ: value_ = lhs != rhs; break;
  case TokenKind::TK_EQEQ: value_ = lhs == rhs; break;
  }

#undef BINARYOP
}

void Interpreter::visit(const UnaryExprPtr& expr) {
  Value value = evaluate(expr->rhs());
  const Token& oper = expr->oper();

  switch (oper.kind()) {
  case TokenKind::TK_BANG: value_ = !value; break;
  case TokenKind::TK_MINUS: check_numeric(oper, value); value_ = -value; break;
  }
}

void Interpreter::visit(const CallExprPtr& expr) {
  Value callee = evaluate(expr->callee());
  if (!callee.is_callable())
    throw RuntimeError(expr->paren(), "can only call functions and classes");

  std::vector<Value> args;
  for (auto& arg : expr->arguments())
    args.push_back(evaluate(arg));
  CallablePtr callable = callee.as_callable();

  if (callable->check_arity() && callable->arity() != args.size()) {
    throw RuntimeError(expr->paren(),
        "expected " + std::to_string(callable->arity()) + " " +
        "arguments but got " + std::to_string(args.size()));
  }
  else {
    value_ = callable->call(shared_from_this(), args);
  }
}

void Interpreter::visit(const GetExprPtr& expr) {
  if (auto object = evaluate(expr->object()); object.is_instance())
    value_ = object.as_instance()->get_attr(expr->name());
  else
    throw RuntimeError(expr->name(), "only instances have attributes");
}

void Interpreter::visit(const LiteralExprPtr& expr) {
  value_ = expr->value();
}

void Interpreter::visit(const GroupingExprPtr& expr) {
  (void)evaluate(expr->expression());
}

void Interpreter::visit(const SuperExprPtr& expr) {
  int distance = 0;
  if (auto super_iter = locals_.find(expr); super_iter != locals_.end())
    distance = super_iter->second;

  const Token& method_name = expr->method();
  ClassPtr superclass = std::static_pointer_cast<Class>(
      environment_->get_at(distance, "super").as_callable());
  InstancePtr object = std::static_pointer_cast<Instance>(
      environment_->get_at(distance - 1, "this").as_instance());

  if (auto method = superclass->get_method(object, method_name); method) {
    value_ = Value(method);
  }
  else {
    throw RuntimeError(method_name,
        "undefined method `" + method_name.as_string() + "`");
  }
}

void Interpreter::visit(const ThisExprPtr& expr) {
  value_ = lookup_variable(expr->keyword(), expr);
}

void Interpreter::visit(const VariableExprPtr& expr) {
  value_ = environment_->get(expr->name());
}

void Interpreter::visit(const FunctionExprPtr& expr) {
}

void Interpreter::visit(const ClassStmtPtr& stmt) {
  ClassPtr superclass;
  Value superval;
  const ExprPtr& superexp = stmt->superclass();
  if (superexp) {
    superval = evaluate(superexp);
    if (superval.is_callable() &&
        std::dynamic_pointer_cast<Class>(superval.as_callable())) {
      superclass = std::static_pointer_cast<Class>(superval.as_callable());
    }
    else {
      throw RuntimeError(stmt->name(), "superclass must be a class");
    }
  }

  environment_->define(stmt->name(), nullptr);

  if (superexp) {
    environment_ = std::make_shared<Environment>(environment_);
    environment_->define("super", superval);
  }

  std::unordered_map<str_t, FunctionPtr> methods;
  for (auto& meth : stmt->methods()) {
    str_t method_name = meth->name().as_string();
    bool is_ctor = method_name == "ctor";
    methods[method_name] =
      std::make_shared<Function>(meth, environment_, is_ctor);
  }

  auto cls = std::make_shared<Class>(
      stmt->name().as_string(), superclass, methods);
  if (!superval.is_nil())
    environment_ = environment_->get_enclosing();
  environment_->assign(stmt->name(), Value(cls));
}

void Interpreter::visit(const FunctionStmtPtr& stmt) {
  Value fun = std::make_shared<Function>(stmt, environment_, false);
  environment_->define(stmt->name(), fun);
}

void Interpreter::visit(const VarStmtPtr& stmt) {
  Value value;
  if (stmt->expr())
    value = evaluate(stmt->expr());
  environment_->define(stmt->name(), value);
}

void Interpreter::visit(const BlockStmtPtr& stmt) {
  evaluate(stmt->stmts(), std::make_shared<Environment>(environment_));
}

void Interpreter::visit(const ExprStmtPtr& stmt) {
  (void)evaluate(stmt->expr());
}

void Interpreter::visit(const WhileStmtPtr& stmt) {
  while (evaluate(stmt->cond()).is_truthy())
    evaluate(stmt->body());
}

void Interpreter::visit(const IfStmtPtr& stmt) {
  if (evaluate(stmt->cond()).is_truthy())
    evaluate(stmt->then_branch());
  else if (stmt->else_branch())
    evaluate(stmt->else_branch());
}

void Interpreter::visit(const PrintStmtPtr& stmt) {
  for (auto& expr : stmt->exprs())
    std::cout << evaluate(expr) << " ";
  std::cout << std::endl;
}

void Interpreter::visit(const ReturnStmtPtr& stmt) {
  Value value;
  if (stmt->value())
    value = evaluate(stmt->value());

  throw Return(value);
}

}
