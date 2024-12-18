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
#include <iostream>
#include "return.h"
#include "runtime_error.h"
#include "function.h"
#include "class.h"
#include "instance.h"
#include "interpreter.h"

Value Interpreter::evaluate(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return value_;
}

void Interpreter::evaluate(const StmtPtr& stmt) {
  stmt->accept(shared_from_this());
}

void Interpreter::evaluate_block(
    const std::vector<StmtPtr>& stmts, const EnvironmentPtr& environment) {
  EnvironmentPtr origin_env = environment_;
  try {
    environment_ = environment;
    for (auto& stmt : stmts)
      evaluate(stmt);
  }
  catch (...) {
    environment_ = origin_env;
    throw;
  }
  // catch (const RuntimeError& e) {
  //   environment_ = origin_env;
  //   throw e;
  // }
  // catch (const Return& e) {
  //   environment_ = origin_env;
  //   throw e;
  // }
  environment_ = origin_env;
}

Value Interpreter::lookup_variable(const Token& name, const ExprPtr& expr) {
  if (locals_.count(expr))
    return environment_->get_at(locals_[expr], name);
  else
    return globals_->get(name);
}

void Interpreter::check_numeric_operand(
    const Token& oper, const Value& operand) {
  if (!operand.is_numeric())
    throw new RuntimeError(oper, "operand must be a numeric object ...");
}

void Interpreter::check_numeric_operands(
    const Token& oper, const Value& left, const Value& right) {
  if (!left.is_numeric() || !right.is_numeric())
    throw new RuntimeError(oper, "operands must be numeric objects ...");
}

void Interpreter::visit_assign_expr(const AssignPtr& expr) {
  Value value = evaluate(expr->value_);

  if (locals_.count(expr))
    environment_->assign_at(locals_[expr], expr->name_, value);
  else
    environment_->assign(expr->name_, value);
}

void Interpreter::visit_binary_expr(const BinaryPtr& expr) {
  Value left = evaluate(expr->left_);
  Value right = evaluate(expr->right_);

  switch (expr->operator_.get_type()) {
  case TOKEN_GREATER:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left > right;
    break;
  case TOKEN_GREATER_EQUAL:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left >= right;
    break;
  case TOKEN_LESS:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left < right;
    break;
  case TOKEN_LESS_EQUAL:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left <= right;
    break;
  case TOKEN_PLUS:
    if ((left.is_numeric() && right.is_numeric()) ||
        (left.is_string() && right.is_string())) {
      value_ = left + right;
    }
    else {
      throw new RuntimeError(expr->operator_,
          "operands must be two numeric objects or string objects ...");
    }
    break;
  case TOKEN_MINUS:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left - right;
    break;
  case TOKEN_STAR:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left * right;
    break;
  case TOKEN_SLASH:
    check_numeric_operands(expr->operator_, left, right);
    value_ = left / right;
    break;
  case TOKEN_BANG_EQUAL: value_ = left != right; break;
  case TOKEN_EQUAL_EQUAL: value_ = left == right; break;
  }
}

void Interpreter::visit_call_expr(const CallPtr& expr) {
  Value callee = evaluate(expr->callee_);
  if (!callee.is_callable())
    throw RuntimeError(expr->paren_, "can only call functions and classes ...");

  std::vector<Value> arguments;
  for (auto& arg : expr->arguments_)
    arguments.push_back(evaluate(arg));
  auto callable = callee.to_callable();

  if (callable->name() != "write") {
    if (arguments.size() != static_cast<std::size_t>(callable->arity())) {
      throw RuntimeError(expr->paren_,
          "expected " + std::to_string(callable->arity()) +
          " arguments but got " + std::to_string(arguments.size()) + " ...");
    }
  }

  value_ = callable->call(shared_from_this(), arguments);
}

void Interpreter::visit_get_expr(const GetPtr& expr) {
  Value object = evaluate(expr->object_);
  if (object.is_instance())
    value_ = object.to_instance()->get_attr(expr->name_);
  else
    throw new RuntimeError(expr->name_, "only instances have properties ...");
}

void Interpreter::visit_set_expr(const SetPtr& expr) {
  Value object = evaluate(expr->object_);
  if (object.is_instance()) {
    Value value = evaluate(expr->value_);
    object.to_instance()->set_attr(expr->name_, value);
  }
  else {
    throw new RuntimeError(expr->name_, "only instances have fields ...");
  }
}

void Interpreter::visit_grouping_expr(const GroupingPtr& expr) {
  value_ = evaluate(expr->expression_);
}

void Interpreter::visit_literal_expr(const LiteralPtr& expr) {
  value_ = expr->value_;
}

void Interpreter::visit_logical_expr(const LogicalPtr& expr) {
  Value left = evaluate(expr->left_);

  if (expr->operator_.get_type() == TOKEN_OR) {
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
  value_ = evaluate(expr->right_);
}

void Interpreter::visit_super_expr(const SuperPtr& expr) {
  using LoxClassPtr = std::shared_ptr<LoxClass>;
  using LoxInstancePtr = std::shared_ptr<LoxInstance>;
  using LoxFunctionPtr = std::shared_ptr<LoxFunction>;

  int distance = locals_[expr];
  LoxClassPtr super_class = std::static_pointer_cast<LoxClass>(
      environment_->get_at(distance, "super").to_callable());
  LoxInstancePtr object = std::static_pointer_cast<LoxInstance>(
      environment_->get_at(distance - 1, "this").to_instance());
  LoxFunctionPtr method =
    super_class->get_method(object, expr->method_.get_lexeme());

  if (!method) {
    throw new RuntimeError(expr->method_,
        "undefined property `" + expr->method_.get_lexeme() + "` ...");
  }
  value_ = Value(method);
}

void Interpreter::visit_this_expr(const ThisPtr& expr) {
  value_ = lookup_variable(expr->keyword_, expr);
}

void Interpreter::visit_unary_expr(const UnaryPtr& expr) {
  Value right = evaluate(expr->right_);

  switch (expr->operator_.get_type()) {
  case TOKEN_MINUS:
    check_numeric_operand(expr->operator_, right);
    value_ = -right; break;
  case TOKEN_NOT: case TOKEN_BANG: value_ = !right; break;
  }
}

void Interpreter::visit_variable_expr(const VariablePtr& expr) {
  value_ = lookup_variable(expr->name_, expr);
}

void Interpreter::visit_function_expr(const FunctionPtr& expr) {
  // TODO:
}

void Interpreter::visit_expr_stmt(const ExprStmtPtr& stmt) {
  evaluate(stmt->expr_);
}

void Interpreter::visit_print_stmt(const PrintStmtPtr& stmt) {
  Value val = evaluate(stmt->expr_);
  std::cout << val.stringify() << std::endl;
}

void Interpreter::visit_var_stmt(const VarStmtPtr& stmt) {
  Value value;
  if (stmt->expr_)
    value = evaluate(stmt->expr_);
  environment_->define_var(stmt->name_.get_lexeme(), value);
}

void Interpreter::visit_block_stmt(const BlockStmtPtr& stmt) {
  auto env = std::make_shared<Environment>(environment_);
  evaluate_block(stmt->stmts_, env);
}

void Interpreter::visit_if_stmt(const IfStmtPtr& stmt) {
  Value cond = evaluate(stmt->cond_);
  if (cond.is_truthy())
    evaluate(stmt->then_branch_);
  else if (stmt->else_branch_)
    evaluate(stmt->else_branch_);
}

void Interpreter::visit_while_stmt(const WhileStmtPtr& stmt) {
  while(evaluate(stmt->cond_).is_truthy())
    evaluate(stmt->body_);
}

void Interpreter::visit_function_stmt(const FunctionStmtPtr& stmt) {
  auto fun = std::make_shared<LoxFunction>(stmt, environment_);
  environment_->define_var(stmt->name_.get_lexeme(), Value(fun));
}

void Interpreter::visit_return_stmt(const ReturnStmtPtr& stmt) {
  Value value;
  if (stmt->value_)
    value = evaluate(stmt->value_);

  throw Return(value);
}

void Interpreter::visit_class_stmt(const ClassStmtPtr& stmt) {
  using LoxFunctionPtr = std::shared_ptr<LoxFunction>;
  using LoxClassPtr = std::shared_ptr<LoxClass>;

  Value super_class_val;
  LoxClassPtr super_class;
  if (stmt->super_class_) {
    super_class_val = evaluate(stmt->super_class_);
    if (super_class_val.is_callable()
        && std::dynamic_pointer_cast<LoxClass>(super_class_val.to_callable())) {
      super_class = std::static_pointer_cast<LoxClass>(super_class_val.to_callable());
    }
    else {
      throw new RuntimeError(stmt->name_, "super class must be a class ...");
    }
  }

  std::string class_name = stmt->name_.get_lexeme();
  environment_->define_var(class_name, Value());

  if (stmt->super_class_) {
    environment_ = std::make_shared<Environment>(environment_);
    environment_->define_var("super", super_class_val);
  }

  std::unordered_map<std::string, LoxFunctionPtr> methods;
  for (auto& meth : stmt->methods_) {
    bool is_ctor = meth->name_.get_lexeme() == "ctor";
    LoxFunctionPtr fun = std::make_shared<LoxFunction>(meth, environment_, is_ctor);
    methods[meth->name_.get_lexeme()] = fun;
  }

  auto klass = std::make_shared<LoxClass>(class_name, super_class, methods);
  if (!super_class_val.is_nil())
    environment_ = environment_->get_enclosing();
  environment_->assign(stmt->name_, Value(klass));
}

void Interpreter::interpret(const ExprPtr& expr) {
  try {
    Value v = evaluate(expr);
    std::cout << v.stringify() << std::endl;
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

void Interpreter::resolve(const ExprPtr& expr, int depth) {
  locals_[expr] = depth;
}
