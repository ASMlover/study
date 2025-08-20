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
#include <fstream>
#include <filesystem>
#include <iostream>
#include <ranges>
#include "Environment.hh"
#include "Function.hh"
#include "Instance.hh"
#include "Module.hh"
#include "Value.hh"
#include "Lexer.hh"
#include "Parser.hh"
#include "Interpreter.hh"

namespace ms {

Interpreter::Interpreter() noexcept {
  globals_ = std::make_shared<Environment>();
  environment_ = globals_;

  {
    auto func_body = [](const std::vector<Value>&) { return as_type<double>(std::time(nullptr)); };
    auto func = std::make_shared<Function>(std::vector<str_t>{}, nullptr, func_body, "clock");
    globals_->define("clock", Value(func));
  }
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) noexcept {
  try {
    for (const auto& statement : statements)
      execute(*statement);
  }
  catch (const std::runtime_error& error) {
    std::cerr << "Runtime error: " << error.what() << std::endl;
  }
}

Value Interpreter::lookup_variable(const Token& name, const Expr* expr) noexcept {
  return environment_->get(name.literal());
}

void Interpreter::execute_block(const std::vector<StmtPtr>& statements, EnvironmentPtr new_env) {
  auto prev = environment_;
  try {
    environment_ = new_env;
    for (const auto& statement : statements)
      execute(*statement);
  }
  catch (...) {
    environment_ = prev;
    throw;
  }
  environment_ = prev;
}

void Interpreter::execute(const Stmt& stmt) noexcept {
  stmt.accept(*this);
}

Value Interpreter::evaluate(const Expr& expr) noexcept {
  return expr.accept(*this);
}

void Interpreter::check_number_operand(const Token& op, const Value& operand) {
  if (operand.is_number())
    return;

  throw std::runtime_error(std::format("Operand `{}` must be a number.", op.literal()));
}

void Interpreter::check_number_operands(const Token& op, const Value& left, const Value& right) {
  if ((left.is_number() && right.is_number()) || (left.is_string() && right.is_string()))
    return;

  throw std::runtime_error(std::format("Operands `{}` must be numbers or strings.", op.literal()));
}

Value Interpreter::visit(const LiteralExpr& expr) {
  return expr.value();
}

Value Interpreter::visit(const GroupingExpr& expr) {
  return evaluate(*expr.expression());
}

Value Interpreter::visit(const UnaryExpr& expr) {
  auto right = evaluate(*expr.right());
  auto op = expr.op();

  switch (op.type()) {
  case TokenType::TK_BANG:
    return !right;
  case TokenType::TK_MINUS:
    check_number_operand(op, right);
    return -right;
  default: return nullptr;
  }
}

Value Interpreter::visit(const BinayExpr& expr) {
  auto left = evaluate(*expr.left());
  auto right = evaluate(*expr.right());
  auto op = expr.op();

  switch (op.type()) {
  case TokenType::TK_MINUS:
    check_number_operands(op, left, right);
    return left - right;
  case TokenType::TK_PLUS:
    check_number_operands(op, left, right);
    return left + right;
  case TokenType::TK_SLASH:
    check_number_operands(op, left, right);
    return left / right;
  case TokenType::TK_STAR:
    check_number_operands(op, left, right);
    return left * right;
  case TokenType::TK_GREATER:
    check_number_operands(op, left, right);
    return left > right;
  case TokenType::TK_GREATER_EQUAL:
    check_number_operands(op, left, right);
    return left >= right;
  case TokenType::TK_LESS:
    check_number_operands(op, left, right);
    return left < right;
  case TokenType::TK_LESS_EQUAL:
    check_number_operands(op, left, right);
    return left <= right;
  case TokenType::TK_BANG_EQUAL: return left != right;
  case TokenType::TK_EQUAL_EQUAL: return left == right;
  default: return nullptr;
  }
}

Value Interpreter::visit(const LogicalExpr& expr) {
  auto left = evaluate(*expr.left());
  auto op = expr.op();

  if (op.type() == TokenType::KW_OR) {
    if (left.is_truthy())
      return left;
  }
  else {
    if (!left.is_truthy())
      return left;
  }
  return evaluate(*expr.right());
}

Value Interpreter::visit(const VariableExpr& expr) {
  return lookup_variable(expr.name(), &expr);
}

Value Interpreter::visit(const AssignExpr& expr) {
  auto value = evaluate(*expr.value());
  environment_->set(expr.name().literal(), value);
  return value;
}

Value Interpreter::visit(const CallExpr& expr) {
  auto callee = evaluate(*expr.callee());

  std::vector<Value> arguments;
  for (const auto& arg: expr.arguments())
    arguments.push_back(evaluate(*arg));

  if (!callee.is_function())
    throw std::runtime_error("Can only call functions and classes.");

  auto function = callee.as_function();
  if (arguments.size() != function->params_size()) {
    throw std::runtime_error(std::format("Expect {} arguments but gor {}", function->params_size(), arguments.size()));
  }
  return function->call(arguments);
}

Value Interpreter::visit(const GetExpr& expr) {
  auto object = evaluate(*expr.object());

  if (object.is_instance())
    return object.as_instance()->get(expr.name().literal());
  throw std::runtime_error("Only instances have properties.");
}

Value Interpreter::visit(const SetExpr& expr) {
  auto object = evaluate(*expr.object());

  if (!object.is_instance())
    throw std::runtime_error("Only instances have fields.");

  auto value = evaluate(*expr.value());
  object.as_instance()->set(expr.name().literal(), value);
  return value;
}

Value Interpreter::visit(const ThisExpr& expr) {
  return lookup_variable(expr.keyword(), &expr);
}

Value Interpreter::visit(const SuperExpr& expr) {
  throw std::runtime_error("Super expressions not yet implemented.");
}

void Interpreter::visit(const ExpressionStmt& stmt) {
  evaluate(*stmt.expression());
}

void Interpreter::visit(const PrintStmt& stmt) {
  auto value = evaluate(*stmt.expression());
  std::cout << value << std::endl;
}

void Interpreter::visit(const VarStmt& stmt) {
  Value value{};
  auto initializer = stmt.initializer();
  if (initializer)
    value = evaluate(*initializer);
  environment_->define(stmt.name().literal(), value);
}

void Interpreter::visit(const BlockStmt& stmt) {
  execute_block(stmt.statements(), std::make_shared<Environment>(environment_));
}

void Interpreter::visit(const IfStmt& stmt) {
  if (evaluate(*stmt.condition()).is_truthy())
    execute(*stmt.then_branch());
  else if (stmt.else_branch())
    execute(*stmt.else_branch());
}

void Interpreter::visit(const WhileStmt& stmt) {
  while (evaluate(*stmt.condition()).is_truthy())
    execute(*stmt.body());
}

void Interpreter::visit(const FunctionStmt& stmt) {
  auto function = std::make_shared<Function>(
      stmt.params() | std::views::transform([](const Token& t) { return t.literal(); }) | std::ranges::to<std::vector>(),
      environment_,
      [this, &stmt](const std::vector<Value>& args) -> Value {
        auto new_env = std::make_shared<Environment>(environment_);
        for (auto i = 0; i < args.size(); ++i)
          new_env->define(stmt.get_param_at_index(i).literal(), args[i]);

        try {
          execute_block(stmt.body(), new_env);
        }
        catch (const Interpreter::ReturnValue& rv) {
          return rv.value;
        }
        return nullptr;
      },
      stmt.name().literal());

  environment_->define(stmt.name().literal(), function);
}

void Interpreter::visit(const ReturnStmt& stmt) {
  Value value{};
  if (stmt.value())
    value = evaluate(*stmt.value());
  throw ReturnValue{value};
}

void Interpreter::visit(const ClassStmt& stmt) {
  ClassPtr superclass{};
  if (stmt.superclass()) {
    auto super_value = evaluate(*stmt.superclass());
    if (!super_value.is_class())
      throw std::runtime_error("Superclass must be a class.");
    superclass = super_value.as_class();
  }
  environment_->define(stmt.name().literal(), nullptr);

  if (superclass) {
    environment_ = std::make_shared<Environment>(environment_);
    environment_->define("super", superclass);
  }

  std::unordered_map<str_t, Value> methods;
  for (const auto& method : stmt.methods()) {
    auto is_initializer = method->name().is_equal_to("init");
    auto function = std::make_shared<Function>(
        method->params() | std::views::transform([](const Token& t) { return t.literal(); }) | std::ranges::to<std::vector>(),
        environment_,
        [this, method, is_initializer](const std::vector<Value>& args) -> Value {
          auto new_env = std::make_shared<Environment>(environment_);
          for (auto i = 0; i < args.size(); ++i)
            new_env->define(method->get_param_at_index(i).literal(), args[i]);

          try {
            execute_block(method->body(), new_env);
          }
          catch (const ReturnValue& rv) {
            if (is_initializer)
              return environment_->get("this");
            return rv.value;
          }

          if (is_initializer)
            return environment_->get("this");
          return nullptr;
        },
        method->name().literal());
    methods[method->name().literal()] = function;
  }

  auto klass = std::make_shared<Class>(stmt.name().literal(), superclass, methods);
  if (superclass)
    environment_ = environment_->parent();

  environment_->assign(stmt.name().literal(), klass);
}

void Interpreter::visit(const ImportStmt& stmt) {
  auto module_path = stmt.module_path().literal();

  if (loaded_modules_.contains(module_path))
    return;

  std::ifstream fp(module_path);
  if (!fp) {
    throw std::runtime_error(std::format("Could not open module `{}`.", module_path));
  }

  ss_t buffer;
  buffer << fp.rdbuf();
  auto source_bytes = buffer.str();

  Lexer lexer{source_bytes};
  auto tokens = lexer.scan_tokens();
  Parser parser{std::move(tokens)};
  auto statements = parser.parse();

  auto module_env = std::make_shared<Environment>(globals_);
  auto prev_env = environment_;
  environment_ = module_env;

  try {
    for (const auto& statement : statements)
      execute(*statement);
  }
  catch (...) {
    environment_ = prev_env;
    throw;
  }
  environment_ = prev_env;

  auto module = std::make_shared<Module>(std::filesystem::path(module_path).stem().string(), module_env);

  for (const auto& [name, value] : module_env->variables()) {
    if (name.starts_with("export_")) {
      auto export_name = name.substr(7);
      module->export_value(export_name, value);
    }
  }
  loaded_modules_[module_path] = module;

  environment_->define(std::filesystem::path(module_path).stem().string(), module);
}

}
