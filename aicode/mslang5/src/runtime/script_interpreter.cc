#include "runtime/script_interpreter.hh"

#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "frontend/lexer.hh"
#include "runtime/module.hh"
#include "runtime/object.hh"
#include "runtime/value.hh"
#include "runtime/vm.hh"

namespace ms {

namespace {

struct RuntimeSignal : public std::runtime_error {
  explicit RuntimeSignal(const std::string& message) : std::runtime_error(message) {}
};

class Environment;
struct Stmt;

using EnvironmentPtr = std::shared_ptr<Environment>;
using StmtPtr = std::shared_ptr<Stmt>;

struct Expr {
  virtual ~Expr() = default;
};
using ExprPtr = std::shared_ptr<Expr>;

struct LiteralExpr final : Expr {
  explicit LiteralExpr(Value v) : value(std::move(v)) {}
  Value value;
};
struct VariableExpr final : Expr {
  explicit VariableExpr(std::string n) : name(std::move(n)) {}
  std::string name;
};
struct AssignExpr final : Expr {
  AssignExpr(std::string n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
  std::string name;
  ExprPtr value;
};
struct GroupingExpr final : Expr {
  explicit GroupingExpr(ExprPtr e) : expression(std::move(e)) {}
  ExprPtr expression;
};
struct UnaryExpr final : Expr {
  UnaryExpr(TokenType o, ExprPtr r) : op(o), right(std::move(r)) {}
  TokenType op;
  ExprPtr right;
};
struct BinaryExpr final : Expr {
  BinaryExpr(ExprPtr l, TokenType o, ExprPtr r)
      : left(std::move(l)), op(o), right(std::move(r)) {}
  ExprPtr left;
  TokenType op;
  ExprPtr right;
};
struct CallExpr final : Expr {
  CallExpr(ExprPtr c, std::vector<ExprPtr> a) : callee(std::move(c)), arguments(std::move(a)) {}
  ExprPtr callee;
  std::vector<ExprPtr> arguments;
};
struct FunctionExpr final : Expr {
  FunctionExpr(std::vector<std::string> p, std::vector<StmtPtr> b)
      : params(std::move(p)), body(std::move(b)) {}
  std::vector<std::string> params;
  std::vector<StmtPtr> body;
};

struct Stmt {
  virtual ~Stmt() = default;
};
struct ExprStmt final : Stmt {
  explicit ExprStmt(ExprPtr e) : expression(std::move(e)) {}
  ExprPtr expression;
};
struct PrintStmt final : Stmt {
  explicit PrintStmt(ExprPtr e) : expression(std::move(e)) {}
  ExprPtr expression;
};
struct VarStmt final : Stmt {
  VarStmt(std::string n, ExprPtr i) : name(std::move(n)), initializer(std::move(i)) {}
  std::string name;
  ExprPtr initializer;
};
struct BlockStmt final : Stmt {
  explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
  std::vector<StmtPtr> statements;
};
struct FunctionStmt final : Stmt {
  FunctionStmt(std::string n, std::vector<std::string> p, std::vector<StmtPtr> b)
      : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
  std::string name;
  std::vector<std::string> params;
  std::vector<StmtPtr> body;
};
struct ReturnStmt final : Stmt {
  explicit ReturnStmt(ExprPtr v) : value(std::move(v)) {}
  ExprPtr value;
};
struct ImportStmt final : Stmt {
  explicit ImportStmt(std::string m) : module(std::move(m)) {}
  std::string module;
};
struct FromImportStmt final : Stmt {
  FromImportStmt(std::string m, std::string s, std::string a)
      : module(std::move(m)), symbol(std::move(s)), alias(std::move(a)) {}
  std::string module;
  std::string symbol;
  std::string alias;
};

class Parser {
 public:
  explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

  std::vector<StmtPtr> ParseProgram() {
    std::vector<StmtPtr> statements;
    while (!IsAtEnd()) {
      auto decl = Declaration();
      if (decl != nullptr) {
        statements.push_back(std::move(decl));
      } else {
        Synchronize();
      }
    }
    return statements;
  }

  const std::vector<std::string>& Errors() const { return errors_; }

 private:
  StmtPtr Declaration() {
    if (Match(TokenType::kFun)) {
      return FunctionDeclaration();
    }
    if (Match(TokenType::kVar)) {
      return VarDeclaration();
    }
    return StatementNode();
  }

  StmtPtr FunctionDeclaration() {
    if (!Consume(TokenType::kIdentifier, "expected function name")) {
      return nullptr;
    }
    const std::string name = Previous().lexeme;
    auto body = ParseFunctionBody();
    if (!body.has_value()) {
      return nullptr;
    }
    return std::make_shared<FunctionStmt>(name, std::move(body->first), std::move(body->second));
  }

  std::optional<std::pair<std::vector<std::string>, std::vector<StmtPtr>>> ParseFunctionBody() {
    if (!Consume(TokenType::kLeftParen, "expected '(' after function name")) {
      return std::nullopt;
    }
    std::vector<std::string> params;
    if (!Check(TokenType::kRightParen)) {
      do {
        if (!Consume(TokenType::kIdentifier, "expected parameter name")) {
          return std::nullopt;
        }
        params.push_back(Previous().lexeme);
      } while (Match(TokenType::kComma));
    }
    if (!Consume(TokenType::kRightParen, "expected ')' after parameters")) {
      return std::nullopt;
    }
    if (!Consume(TokenType::kLeftBrace, "expected '{' before function body")) {
      return std::nullopt;
    }
    return std::make_pair(params, ParseBlockStatements());
  }

  StmtPtr VarDeclaration() {
    if (!Consume(TokenType::kIdentifier, "expected variable name")) {
      return nullptr;
    }
    const std::string name = Previous().lexeme;
    ExprPtr initializer = nullptr;
    if (Match(TokenType::kEqual)) {
      initializer = ExpressionNode();
    }
    if (!Consume(TokenType::kSemicolon, "expected ';' after variable declaration")) {
      return nullptr;
    }
    return std::make_shared<VarStmt>(name, std::move(initializer));
  }

  StmtPtr StatementNode() {
    if (Match(TokenType::kPrint)) {
      auto value = ExpressionNode();
      Consume(TokenType::kSemicolon, "expected ';' after print value");
      return std::make_shared<PrintStmt>(std::move(value));
    }
    if (Match(TokenType::kReturn)) {
      ExprPtr value = nullptr;
      if (!Check(TokenType::kSemicolon)) {
        value = ExpressionNode();
      }
      Consume(TokenType::kSemicolon, "expected ';' after return value");
      return std::make_shared<ReturnStmt>(std::move(value));
    }
    if (Match(TokenType::kImport)) {
      const std::string module = ParseDottedName();
      Consume(TokenType::kSemicolon, "expected ';' after import statement");
      return std::make_shared<ImportStmt>(module);
    }
    if (Match(TokenType::kFrom)) {
      const std::string module = ParseDottedName();
      Consume(TokenType::kImport, "expected 'import' keyword");
      Consume(TokenType::kIdentifier, "expected imported symbol name");
      const std::string symbol = Previous().lexeme;
      std::string alias = symbol;
      if (Match(TokenType::kAs)) {
        Consume(TokenType::kIdentifier, "expected alias name after 'as'");
        alias = Previous().lexeme;
      }
      Consume(TokenType::kSemicolon, "expected ';' after from-import statement");
      return std::make_shared<FromImportStmt>(module, symbol, alias);
    }
    if (Match(TokenType::kLeftBrace)) {
      return std::make_shared<BlockStmt>(ParseBlockStatements());
    }
    auto expr = ExpressionNode();
    Consume(TokenType::kSemicolon, "expected ';' after expression");
    return std::make_shared<ExprStmt>(std::move(expr));
  }

  std::vector<StmtPtr> ParseBlockStatements() {
    std::vector<StmtPtr> statements;
    while (!Check(TokenType::kRightBrace) && !IsAtEnd()) {
      auto decl = Declaration();
      if (decl != nullptr) {
        statements.push_back(std::move(decl));
      } else {
        Synchronize();
      }
    }
    Consume(TokenType::kRightBrace, "expected '}' after block");
    return statements;
  }

  ExprPtr ExpressionNode() { return Assignment(); }

  ExprPtr Assignment() {
    ExprPtr expr = Term();
    if (Match(TokenType::kEqual)) {
      ExprPtr value = Assignment();
      if (auto var = std::dynamic_pointer_cast<VariableExpr>(expr); var != nullptr) {
        return std::make_shared<AssignExpr>(var->name, value);
      }
      ReportError(Previous(), "invalid assignment target");
    }
    return expr;
  }

  ExprPtr Term() {
    ExprPtr expr = Factor();
    while (Match(TokenType::kPlus) || Match(TokenType::kMinus)) {
      TokenType op = Previous().type;
      ExprPtr right = Factor();
      expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
  }

  ExprPtr Factor() {
    ExprPtr expr = Unary();
    while (Match(TokenType::kStar) || Match(TokenType::kSlash)) {
      TokenType op = Previous().type;
      ExprPtr right = Unary();
      expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
  }

  ExprPtr Unary() {
    if (Match(TokenType::kMinus)) {
      return std::make_shared<UnaryExpr>(Previous().type, Unary());
    }
    return Call();
  }

  ExprPtr Call() {
    ExprPtr expr = Primary();
    while (Match(TokenType::kLeftParen)) {
      std::vector<ExprPtr> args;
      if (!Check(TokenType::kRightParen)) {
        do {
          args.push_back(ExpressionNode());
        } while (Match(TokenType::kComma));
      }
      Consume(TokenType::kRightParen, "expected ')' after arguments");
      expr = std::make_shared<CallExpr>(expr, std::move(args));
    }
    return expr;
  }

  ExprPtr Primary() {
    if (Match(TokenType::kNumber)) {
      return std::make_shared<LiteralExpr>(Value(std::strtod(Previous().lexeme.c_str(), nullptr)));
    }
    if (Match(TokenType::kString)) {
      return std::make_shared<LiteralExpr>(Value(Previous().lexeme));
    }
    if (Match(TokenType::kTrue)) {
      return std::make_shared<LiteralExpr>(Value(true));
    }
    if (Match(TokenType::kFalse)) {
      return std::make_shared<LiteralExpr>(Value(false));
    }
    if (Match(TokenType::kNil)) {
      return std::make_shared<LiteralExpr>(Value::Nil());
    }
    if (Match(TokenType::kIdentifier)) {
      return std::make_shared<VariableExpr>(Previous().lexeme);
    }
    if (Match(TokenType::kFun)) {
      auto body = ParseFunctionBody();
      if (!body.has_value()) {
        return nullptr;
      }
      return std::make_shared<FunctionExpr>(std::move(body->first), std::move(body->second));
    }
    if (Match(TokenType::kLeftParen)) {
      ExprPtr expr = ExpressionNode();
      Consume(TokenType::kRightParen, "expected ')' after expression");
      return std::make_shared<GroupingExpr>(expr);
    }
    ReportError(Current(), "expected expression");
    return nullptr;
  }

  bool Match(TokenType type) {
    if (!Check(type)) {
      return false;
    }
    Advance();
    return true;
  }
  bool Check(TokenType type) const { return !IsAtEnd() ? Current().type == type : type == TokenType::kEof; }
  bool Consume(TokenType type, const std::string& message) {
    if (Check(type)) {
      Advance();
      return true;
    }
    ReportError(Current(), message);
    return false;
  }
  std::string ParseDottedName() {
    Consume(TokenType::kIdentifier, "expected identifier");
    std::string name = Previous().lexeme;
    while (Match(TokenType::kDot)) {
      Consume(TokenType::kIdentifier, "expected identifier after '.'");
      name += ".";
      name += Previous().lexeme;
    }
    return name;
  }
  const Token& Advance() {
    if (!IsAtEnd()) ++current_;
    return Previous();
  }
  bool IsAtEnd() const { return Current().type == TokenType::kEof; }
  const Token& Current() const { return tokens_[current_]; }
  const Token& Previous() const { return tokens_[current_ - 1]; }
  void ReportError(const Token& token, const std::string& message) {
    errors_.push_back("[line " + std::to_string(token.line) + "] parse error: " + message);
  }
  void Synchronize() {
    Advance();
    while (!IsAtEnd()) {
      if (Previous().type == TokenType::kSemicolon) return;
      switch (Current().type) {
        case TokenType::kFun:
        case TokenType::kVar:
        case TokenType::kPrint:
        case TokenType::kReturn:
        case TokenType::kImport:
        case TokenType::kFrom:
          return;
        default:
          break;
      }
      Advance();
    }
  }

  std::vector<Token> tokens_;
  std::size_t current_ = 0;
  std::vector<std::string> errors_;
};

class RuntimeFunction;
class Executor;

class Environment {
 public:
  Environment(Vm& vm, EnvironmentPtr enclosing)
      : vm_(vm), enclosing_(std::move(enclosing)), global_(enclosing_ == nullptr) {}

  void Define(const std::string& name, const Value& value) {
    if (global_) {
      vm_.DefineGlobal(name, value);
      return;
    }
    values_[name] = value;
  }

  bool Assign(const std::string& name, const Value& value) {
    if (!global_) {
      auto it = values_.find(name);
      if (it != values_.end()) {
        it->second = value;
        return true;
      }
    }
    if (enclosing_ != nullptr) return enclosing_->Assign(name, value);
    return vm_.SetGlobal(name, value);
  }

  bool Get(const std::string& name, Value* out) const {
    if (!global_) {
      auto it = values_.find(name);
      if (it != values_.end()) {
        if (out != nullptr) *out = it->second;
        return true;
      }
    }
    if (enclosing_ != nullptr) return enclosing_->Get(name, out);
    return vm_.GetGlobal(name, out);
  }

 private:
  Vm& vm_;
  EnvironmentPtr enclosing_;
  bool global_;
  std::unordered_map<std::string, Value> values_;
};

class RuntimeCallable : public RuntimeObject {
 public:
  ~RuntimeCallable() override = default;
  virtual std::size_t Arity() const = 0;
  virtual bool Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) = 0;
};

struct ReturnSignal {
  Value value;
};

class RuntimeFunction final : public RuntimeCallable {
 public:
  RuntimeFunction(std::string name, std::vector<std::string> params, std::vector<StmtPtr> body,
                  EnvironmentPtr closure)
      : name_(std::move(name)),
        params_(std::move(params)),
        body_(std::move(body)),
        closure_(std::move(closure)) {}

  std::size_t Arity() const override { return params_.size(); }
  std::string ToString() const override { return name_.empty() ? "<fn anonymous>" : "<fn " + name_ + ">"; }
  bool Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) override;

 private:
  std::string name_;
  std::vector<std::string> params_;
  std::vector<StmtPtr> body_;
  EnvironmentPtr closure_;
};

class Executor {
 public:
  explicit Executor(Vm& vm)
      : vm_(vm), globals_(std::make_shared<Environment>(vm_, nullptr)), env_(globals_) {}

  bool Run(const std::vector<StmtPtr>& stmts, std::string* error) {
    try {
      for (const auto& s : stmts) ExecStmt(s);
      return true;
    } catch (const RuntimeSignal& e) {
      if (error != nullptr) *error = e.what();
      return false;
    }
  }

  bool RunFunction(const std::vector<StmtPtr>& stmts, EnvironmentPtr scope, Value* out, std::string* error) {
    auto prev = env_;
    env_ = std::move(scope);
    try {
      for (const auto& s : stmts) ExecStmt(s);
      env_ = prev;
      if (out != nullptr) *out = Value::Nil();
      return true;
    } catch (const ReturnSignal& signal) {
      env_ = prev;
      if (out != nullptr) *out = signal.value;
      return true;
    } catch (const RuntimeSignal& e) {
      env_ = prev;
      if (error != nullptr) *error = e.what();
      return false;
    }
  }

 private:
  static std::string LastSegment(const std::string& dotted) {
    auto pos = dotted.find_last_of('.');
    return pos == std::string::npos ? dotted : dotted.substr(pos + 1);
  }

  void ExecStmt(const StmtPtr& stmt) {
    if (auto s = std::dynamic_pointer_cast<ExprStmt>(stmt); s != nullptr) {
      (void)Eval(s->expression);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<PrintStmt>(stmt); s != nullptr) {
      vm_.Output() << Eval(s->expression).ToString() << '\n';
      return;
    }
    if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt); s != nullptr) {
      Value v = s->initializer ? Eval(s->initializer) : Value::Nil();
      env_->Define(s->name, v);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<BlockStmt>(stmt); s != nullptr) {
      ExecBlock(s->statements, std::make_shared<Environment>(vm_, env_));
      return;
    }
    if (auto s = std::dynamic_pointer_cast<FunctionStmt>(stmt); s != nullptr) {
      auto fn = std::make_shared<RuntimeFunction>(s->name, s->params, s->body, env_);
      env_->Define(s->name, Value(std::static_pointer_cast<RuntimeObject>(fn)));
      return;
    }
    if (auto s = std::dynamic_pointer_cast<ReturnStmt>(stmt); s != nullptr) {
      throw ReturnSignal{s->value ? Eval(s->value) : Value::Nil()};
    }
    if (auto s = std::dynamic_pointer_cast<ImportStmt>(stmt); s != nullptr) {
      std::string load_error;
      auto module = vm_.Modules().Load(s->module, vm_, &load_error);
      if (!module) throw RuntimeSignal(load_error);
      env_->Define(LastSegment(s->module), Value(module));
      return;
    }
    if (auto s = std::dynamic_pointer_cast<FromImportStmt>(stmt); s != nullptr) {
      std::string load_error;
      auto module = vm_.Modules().Load(s->module, vm_, &load_error);
      if (!module) throw RuntimeSignal(load_error);
      Value exported;
      if (!module->exports.Get(s->symbol, &exported)) {
        throw RuntimeSignal("module '" + s->module + "' has no symbol '" + s->symbol + "'");
      }
      env_->Define(s->alias, exported);
      return;
    }
    throw RuntimeSignal("unsupported statement");
  }

  void ExecBlock(const std::vector<StmtPtr>& stmts, const EnvironmentPtr& scope) {
    auto prev = env_;
    env_ = scope;
    try {
      for (const auto& s : stmts) ExecStmt(s);
      env_ = prev;
    } catch (...) {
      env_ = prev;
      throw;
    }
  }

  Value Eval(const ExprPtr& expr) {
    if (expr == nullptr) return Value::Nil();
    if (auto e = std::dynamic_pointer_cast<LiteralExpr>(expr); e != nullptr) return e->value;
    if (auto e = std::dynamic_pointer_cast<VariableExpr>(expr); e != nullptr) {
      Value v;
      if (!env_->Get(e->name, &v)) throw RuntimeSignal("undefined variable: " + e->name);
      return v;
    }
    if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr); e != nullptr) {
      Value v = Eval(e->value);
      if (!env_->Assign(e->name, v)) throw RuntimeSignal("undefined variable: " + e->name);
      return v;
    }
    if (auto e = std::dynamic_pointer_cast<GroupingExpr>(expr); e != nullptr) return Eval(e->expression);
    if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr); e != nullptr) {
      Value r = Eval(e->right);
      if (!r.IsNumber()) throw RuntimeSignal("operand must be number");
      return Value(-r.AsNumber());
    }
    if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr); e != nullptr) {
      Value l = Eval(e->left);
      Value r = Eval(e->right);
      if (e->op == TokenType::kPlus) {
        if (l.IsNumber() && r.IsNumber()) return Value(l.AsNumber() + r.AsNumber());
        if (l.IsString() && r.IsString()) return Value(l.AsString() + r.AsString());
        throw RuntimeSignal("operands must be two numbers or two strings");
      }
      if (!l.IsNumber() || !r.IsNumber()) throw RuntimeSignal("operands must be numbers");
      if (e->op == TokenType::kMinus) return Value(l.AsNumber() - r.AsNumber());
      if (e->op == TokenType::kStar) return Value(l.AsNumber() * r.AsNumber());
      return Value(l.AsNumber() / r.AsNumber());
    }
    if (auto e = std::dynamic_pointer_cast<FunctionExpr>(expr); e != nullptr) {
      auto fn = std::make_shared<RuntimeFunction>("", e->params, e->body, env_);
      return Value(std::static_pointer_cast<RuntimeObject>(fn));
    }
    if (auto e = std::dynamic_pointer_cast<CallExpr>(expr); e != nullptr) {
      Value callee = Eval(e->callee);
      if (!callee.IsObject() || callee.AsObject() == nullptr) {
        throw RuntimeSignal("can only call functions and classes");
      }
      auto callable = std::dynamic_pointer_cast<RuntimeCallable>(callee.AsObject());
      if (callable == nullptr) throw RuntimeSignal("can only call functions and classes");
      std::vector<Value> args;
      for (const auto& arg : e->arguments) args.push_back(Eval(arg));
      if (args.size() != callable->Arity()) {
        throw RuntimeSignal("expected " + std::to_string(callable->Arity()) + " arguments but got " +
                            std::to_string(args.size()));
      }
      Value out = Value::Nil();
      std::string call_error;
      if (!callable->Call(*this, args, &out, &call_error)) throw RuntimeSignal(call_error);
      return out;
    }
    throw RuntimeSignal("unsupported expression");
  }

  Vm& vm_;
  EnvironmentPtr globals_;
  EnvironmentPtr env_;

  friend class RuntimeFunction;
};

bool RuntimeFunction::Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) {
  auto scope = std::make_shared<Environment>(executor.vm_, closure_);
  for (std::size_t i = 0; i < params_.size(); ++i) scope->Define(params_[i], args[i]);
  return executor.RunFunction(body_, scope, out, error);
}

}  // namespace

bool ScriptInterpreter::Execute(Vm& vm, const std::string& source, std::string* error) {
  Lexer lexer(source);
  Parser parser(lexer.ScanAllTokens());
  auto program = parser.ParseProgram();
  if (!parser.Errors().empty()) {
    if (error != nullptr) {
      *error = parser.Errors().front();
    }
    return false;
  }
  Executor executor(vm);
  return executor.Run(program, error);
}

}  // namespace ms
