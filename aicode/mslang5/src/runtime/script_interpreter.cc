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

std::string RuntimeError(const std::string& code, const std::string& message) {
  return "runtime error (" + code + "): " + message;
}

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
  VariableExpr(std::string n, std::size_t ln) : name(std::move(n)), line(ln) {}
  std::string name;
  std::size_t line;
};
struct AssignExpr final : Expr {
  AssignExpr(std::string n, ExprPtr v, std::size_t ln)
      : name(std::move(n)), value(std::move(v)), line(ln) {}
  std::string name;
  ExprPtr value;
  std::size_t line;
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
struct GetExpr final : Expr {
  GetExpr(ExprPtr o, std::string n) : object(std::move(o)), name(std::move(n)) {}
  ExprPtr object;
  std::string name;
};
struct SetExpr final : Expr {
  SetExpr(ExprPtr o, std::string n, ExprPtr v)
      : object(std::move(o)), name(std::move(n)), value(std::move(v)) {}
  ExprPtr object;
  std::string name;
  ExprPtr value;
};
struct ThisExpr final : Expr {
  ThisExpr(std::string n, std::size_t ln) : name(std::move(n)), line(ln) {}
  std::string name;
  std::size_t line;
};
struct SuperExpr final : Expr {
  SuperExpr(std::string m, std::size_t ln) : method(std::move(m)), line(ln) {}
  std::string method;
  std::size_t line;
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
  ReturnStmt(ExprPtr v, std::size_t ln) : value(std::move(v)), line(ln) {}
  ExprPtr value;
  std::size_t line;
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
struct ClassStmt final : Stmt {
  ClassStmt(std::string n, std::size_t ln, std::string s, std::size_t sln,
            std::vector<std::shared_ptr<FunctionStmt>> m)
      : name(std::move(n)),
        line(ln),
        superclass(std::move(s)),
        superclass_line(sln),
        methods(std::move(m)) {}
  std::string name;
  std::size_t line;
  std::string superclass;
  std::size_t superclass_line;
  std::vector<std::shared_ptr<FunctionStmt>> methods;
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
    if (Match(TokenType::kClass)) {
      return ClassDeclaration();
    }
    if (Match(TokenType::kFun)) {
      return FunctionDeclaration("function");
    }
    if (Match(TokenType::kVar)) {
      return VarDeclaration();
    }
    return StatementNode();
  }

  StmtPtr FunctionDeclaration(const std::string& kind) {
    if (!Consume(TokenType::kIdentifier, "expected " + kind + " name")) {
      return nullptr;
    }
    const std::string name = Previous().lexeme;
    auto body = ParseFunctionBody(kind);
    if (!body.has_value()) {
      return nullptr;
    }
    return std::make_shared<FunctionStmt>(name, std::move(body->first), std::move(body->second));
  }

  StmtPtr ClassDeclaration() {
    if (!Consume(TokenType::kIdentifier, "expected class name")) {
      return nullptr;
    }
    const std::string class_name = Previous().lexeme;
    const std::size_t class_line = Previous().line;

    std::string superclass;
    std::size_t superclass_line = 0;
    if (Match(TokenType::kLess)) {
      if (!Consume(TokenType::kIdentifier, "expected superclass name")) {
        return nullptr;
      }
      superclass = Previous().lexeme;
      superclass_line = Previous().line;
    }

    if (!Consume(TokenType::kLeftBrace, "expected '{' before class body")) {
      return nullptr;
    }
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    while (!Check(TokenType::kRightBrace) && !IsAtEnd()) {
      auto method = ParseMethod();
      if (method != nullptr) {
        methods.push_back(method);
      } else {
        Synchronize();
      }
    }
    Consume(TokenType::kRightBrace, "expected '}' after class body");
    return std::make_shared<ClassStmt>(class_name, class_line, superclass, superclass_line,
                                       std::move(methods));
  }

  std::shared_ptr<FunctionStmt> ParseMethod() {
    if (!Consume(TokenType::kIdentifier, "expected method name")) {
      return nullptr;
    }
    const std::string name = Previous().lexeme;
    auto body = ParseFunctionBody("method");
    if (!body.has_value()) {
      return nullptr;
    }
    return std::make_shared<FunctionStmt>(name, std::move(body->first), std::move(body->second));
  }

  std::optional<std::pair<std::vector<std::string>, std::vector<StmtPtr>>> ParseFunctionBody(
      const std::string& kind) {
    if (!Consume(TokenType::kLeftParen, "expected '(' after " + kind + " name")) {
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
    if (!Consume(TokenType::kLeftBrace, "expected '{' before " + kind + " body")) {
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
      const std::size_t line = Previous().line;
      ExprPtr value = nullptr;
      if (!Check(TokenType::kSemicolon)) {
        value = ExpressionNode();
      }
      Consume(TokenType::kSemicolon, "expected ';' after return value");
      return std::make_shared<ReturnStmt>(std::move(value), line);
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
      const Token equals = Previous();
      ExprPtr value = Assignment();
      if (auto var = std::dynamic_pointer_cast<VariableExpr>(expr); var != nullptr) {
        return std::make_shared<AssignExpr>(var->name, value, equals.line);
      }
      if (auto object = std::dynamic_pointer_cast<GetExpr>(expr); object != nullptr) {
        return std::make_shared<SetExpr>(object->object, object->name, value);
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
    while (true) {
      if (Match(TokenType::kLeftParen)) {
        std::vector<ExprPtr> args;
        if (!Check(TokenType::kRightParen)) {
          do {
            args.push_back(ExpressionNode());
          } while (Match(TokenType::kComma));
        }
        Consume(TokenType::kRightParen, "expected ')' after arguments");
        expr = std::make_shared<CallExpr>(expr, std::move(args));
        continue;
      }
      if (Match(TokenType::kDot)) {
        Consume(TokenType::kIdentifier, "expected property name after '.'");
        expr = std::make_shared<GetExpr>(expr, Previous().lexeme);
        continue;
      }
      break;
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
      return std::make_shared<VariableExpr>(Previous().lexeme, Previous().line);
    }
    if (Match(TokenType::kFun)) {
      auto body = ParseFunctionBody("anonymous function");
      if (!body.has_value()) {
        return nullptr;
      }
      return std::make_shared<FunctionExpr>(std::move(body->first), std::move(body->second));
    }
    if (Match(TokenType::kThis)) {
      return std::make_shared<ThisExpr>("this", Previous().line);
    }
    if (Match(TokenType::kSuper)) {
      const std::size_t line = Previous().line;
      Consume(TokenType::kDot, "expected '.' after 'super'");
      Consume(TokenType::kIdentifier, "expected superclass method name");
      return std::make_shared<SuperExpr>(Previous().lexeme, line);
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
        case TokenType::kClass:
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
class RuntimeClass;
class RuntimeInstance;
class RuntimeBoundMethod;
class Executor;

struct ResolveResult {
  std::unordered_map<const Expr*, std::size_t> local_depths;
  std::vector<std::string> errors;
  std::size_t captured_bindings = 0;
};

class Resolver {
 public:
  ResolveResult Resolve(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
      ResolveStmt(stmt);
    }
    return std::move(result_);
  }

 private:
  enum class FunctionType { kNone, kFunction, kMethod, kInitializer };
  enum class ClassType { kNone, kClass, kSubclass };

  struct Binding {
    bool defined = false;
    bool captured = false;
  };

  static std::string MakeResolveError(const std::size_t line, const std::string& code,
                                      const std::string& message) {
    return "[line " + std::to_string(line) + "] resolve error (" + code + "): " + message;
  }

  void Report(const std::size_t line, const std::string& code, const std::string& message) {
    result_.errors.push_back(MakeResolveError(line, code, message));
  }

  void BeginScope() { scopes_.push_back({}); }

  void EndScope() { scopes_.pop_back(); }

  void Declare(const std::string& name) {
    if (scopes_.empty()) {
      return;
    }
    auto& scope = scopes_.back();
    if (scope.contains(name)) {
      Report(current_line_, "MS3005", "variable '" + name + "' already declared in this scope");
      return;
    }
    scope[name] = Binding{false, false};
  }

  void Define(const std::string& name) {
    if (scopes_.empty()) {
      return;
    }
    auto& scope = scopes_.back();
    auto it = scope.find(name);
    if (it != scope.end()) {
      it->second.defined = true;
    }
  }

  void ResolveLocal(const ExprPtr& expr, const std::string& name) {
    for (std::size_t i = scopes_.size(); i > 0; --i) {
      auto& scope = scopes_[i - 1];
      auto it = scope.find(name);
      if (it == scope.end()) {
        continue;
      }
      const std::size_t depth = scopes_.size() - i;
      result_.local_depths[expr.get()] = depth;
      if (depth > 0 && !it->second.captured) {
        it->second.captured = true;
        ++result_.captured_bindings;
      }
      return;
    }
  }

  void ResolveFunction(const std::vector<std::string>& params, const std::vector<StmtPtr>& body,
                       const FunctionType type) {
    const FunctionType enclosing_function = current_function_;
    current_function_ = type;
    BeginScope();
    for (const auto& param : params) {
      Declare(param);
      Define(param);
    }
    for (const auto& stmt : body) {
      ResolveStmt(stmt);
    }
    EndScope();
    current_function_ = enclosing_function;
  }

  void ResolveStmt(const StmtPtr& stmt) {
    if (stmt == nullptr) {
      return;
    }
    if (auto s = std::dynamic_pointer_cast<ExprStmt>(stmt); s != nullptr) {
      ResolveExpr(s->expression);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<PrintStmt>(stmt); s != nullptr) {
      ResolveExpr(s->expression);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<VarStmt>(stmt); s != nullptr) {
      current_line_ = 0;
      Declare(s->name);
      ResolveExpr(s->initializer);
      Define(s->name);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<BlockStmt>(stmt); s != nullptr) {
      BeginScope();
      for (const auto& child : s->statements) {
        ResolveStmt(child);
      }
      EndScope();
      return;
    }
    if (auto s = std::dynamic_pointer_cast<FunctionStmt>(stmt); s != nullptr) {
      Declare(s->name);
      Define(s->name);
      ResolveFunction(s->params, s->body, FunctionType::kFunction);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<ReturnStmt>(stmt); s != nullptr) {
      current_line_ = s->line;
      if (current_function_ == FunctionType::kNone) {
        Report(s->line, "MS3001", "cannot return from top-level code");
      }
      if (current_function_ == FunctionType::kInitializer && s->value != nullptr) {
        Report(s->line, "MS3007", "cannot return a value from initializer");
      }
      ResolveExpr(s->value);
      return;
    }
    if (auto s = std::dynamic_pointer_cast<ClassStmt>(stmt); s != nullptr) {
      current_line_ = s->line;
      const ClassType enclosing_class = current_class_;
      current_class_ = ClassType::kClass;

      Declare(s->name);
      Define(s->name);

      if (!s->superclass.empty()) {
        if (s->superclass == s->name) {
          Report(s->superclass_line, "MS3004", "a class cannot inherit from itself");
        }
        current_class_ = ClassType::kSubclass;
        auto super_ref = std::make_shared<VariableExpr>(s->superclass, s->superclass_line);
        ResolveExpr(super_ref);
        BeginScope();
        scopes_.back()["super"] = Binding{true, false};
      }

      BeginScope();
      scopes_.back()["this"] = Binding{true, false};
      for (const auto& method : s->methods) {
        const FunctionType method_type =
            method->name == "init" ? FunctionType::kInitializer : FunctionType::kMethod;
        ResolveFunction(method->params, method->body, method_type);
      }
      EndScope();
      if (!s->superclass.empty()) {
        EndScope();
      }
      current_class_ = enclosing_class;
      return;
    }
  }

  void ResolveExpr(const ExprPtr& expr) {
    if (expr == nullptr) {
      return;
    }
    if (auto e = std::dynamic_pointer_cast<VariableExpr>(expr); e != nullptr) {
      current_line_ = e->line;
      if (!scopes_.empty()) {
        auto it = scopes_.back().find(e->name);
        if (it != scopes_.back().end() && !it->second.defined) {
          Report(e->line, "MS3006",
                 "cannot read local variable '" + e->name + "' in its own initializer");
        }
      }
      ResolveLocal(expr, e->name);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr); e != nullptr) {
      current_line_ = e->line;
      ResolveExpr(e->value);
      ResolveLocal(expr, e->name);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<GroupingExpr>(expr); e != nullptr) {
      ResolveExpr(e->expression);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr); e != nullptr) {
      ResolveExpr(e->right);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr); e != nullptr) {
      ResolveExpr(e->left);
      ResolveExpr(e->right);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<CallExpr>(expr); e != nullptr) {
      ResolveExpr(e->callee);
      for (const auto& arg : e->arguments) {
        ResolveExpr(arg);
      }
      return;
    }
    if (auto e = std::dynamic_pointer_cast<FunctionExpr>(expr); e != nullptr) {
      ResolveFunction(e->params, e->body, FunctionType::kFunction);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<GetExpr>(expr); e != nullptr) {
      ResolveExpr(e->object);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<SetExpr>(expr); e != nullptr) {
      ResolveExpr(e->value);
      ResolveExpr(e->object);
      return;
    }
    if (auto e = std::dynamic_pointer_cast<ThisExpr>(expr); e != nullptr) {
      current_line_ = e->line;
      if (current_class_ == ClassType::kNone) {
        Report(e->line, "MS3002", "cannot use 'this' outside of a class");
        return;
      }
      ResolveLocal(expr, "this");
      return;
    }
    if (auto e = std::dynamic_pointer_cast<SuperExpr>(expr); e != nullptr) {
      current_line_ = e->line;
      if (current_class_ == ClassType::kNone || current_class_ == ClassType::kClass) {
        Report(e->line, "MS3003", "cannot use 'super' outside of a subclass");
        return;
      }
      ResolveLocal(expr, "super");
      return;
    }
  }

  ResolveResult result_;
  std::vector<std::unordered_map<std::string, Binding>> scopes_;
  FunctionType current_function_ = FunctionType::kNone;
  ClassType current_class_ = ClassType::kNone;
  std::size_t current_line_ = 0;
};

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

  bool AssignAt(const std::size_t depth, const std::string& name, const Value& value) {
    Environment* scope = this;
    for (std::size_t i = 0; i < depth; ++i) {
      if (scope->enclosing_ == nullptr) {
        return false;
      }
      scope = scope->enclosing_.get();
    }
    if (scope->global_) {
      return scope->vm_.SetGlobal(name, value);
    }
    auto it = scope->values_.find(name);
    if (it == scope->values_.end()) {
      return false;
    }
    it->second = value;
    return true;
  }

  bool GetAt(const std::size_t depth, const std::string& name, Value* out) const {
    const Environment* scope = this;
    for (std::size_t i = 0; i < depth; ++i) {
      if (scope->enclosing_ == nullptr) {
        return false;
      }
      scope = scope->enclosing_.get();
    }
    if (scope->global_) {
      return scope->vm_.GetGlobal(name, out);
    }
    auto it = scope->values_.find(name);
    if (it == scope->values_.end()) {
      return false;
    }
    if (out != nullptr) {
      *out = it->second;
    }
    return true;
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
                  EnvironmentPtr closure, bool is_initializer = false)
      : name_(std::move(name)),
        params_(std::move(params)),
        body_(std::move(body)),
        closure_(std::move(closure)),
        is_initializer_(is_initializer) {}

  std::size_t Arity() const override { return params_.size(); }
  std::string ToString() const override { return name_.empty() ? "<fn anonymous>" : "<fn " + name_ + ">"; }
  bool Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) override;
  std::shared_ptr<RuntimeFunction> Bind(const Value& instance, Vm& vm) const;
  bool IsInitializer() const { return is_initializer_; }

 private:
  std::string name_;
  std::vector<std::string> params_;
  std::vector<StmtPtr> body_;
  EnvironmentPtr closure_;
  bool is_initializer_;
};

class RuntimeClass final : public RuntimeCallable, public std::enable_shared_from_this<RuntimeClass> {
 public:
  RuntimeClass(std::string name, std::shared_ptr<RuntimeClass> superclass,
               std::unordered_map<std::string, std::shared_ptr<RuntimeFunction>> methods)
      : name_(std::move(name)), superclass_(std::move(superclass)), methods_(std::move(methods)) {}

  std::string ToString() const override { return "<class " + name_ + ">"; }
  std::size_t Arity() const override;
  bool Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) override;
  std::shared_ptr<RuntimeFunction> FindMethod(const std::string& name) const;
  const std::string& Name() const { return name_; }

 private:
  std::string name_;
  std::shared_ptr<RuntimeClass> superclass_;
  std::unordered_map<std::string, std::shared_ptr<RuntimeFunction>> methods_;
};

class RuntimeInstance final : public RuntimeObject, public std::enable_shared_from_this<RuntimeInstance> {
 public:
  explicit RuntimeInstance(std::shared_ptr<RuntimeClass> klass) : klass_(std::move(klass)) {}

  std::string ToString() const override { return "<" + klass_->Name() + " instance>"; }
  bool Get(const std::string& name, Value* out);
  void Set(const std::string& name, const Value& value) { fields_[name] = value; }

 private:
  std::shared_ptr<RuntimeClass> klass_;
  std::unordered_map<std::string, Value> fields_;
};

class RuntimeBoundMethod final : public RuntimeCallable {
 public:
  RuntimeBoundMethod(Value receiver, std::shared_ptr<RuntimeFunction> method)
      : receiver_(std::move(receiver)), method_(std::move(method)) {}

  std::string ToString() const override { return method_->ToString(); }
  std::size_t Arity() const override { return method_->Arity(); }
  bool Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) override;

 private:
  Value receiver_;
  std::shared_ptr<RuntimeFunction> method_;
};

class Executor {
 public:
  Executor(Vm& vm, const std::unordered_map<const Expr*, std::size_t>* local_depths)
      : vm_(vm),
        globals_(std::make_shared<Environment>(vm_, nullptr)),
        env_(globals_),
        local_depths_(local_depths) {}

  bool Run(const std::vector<StmtPtr>& stmts, std::string* error) {
    try {
      for (const auto& s : stmts) ExecStmt(s);
      return true;
    } catch (const ReturnSignal&) {
      if (error != nullptr) *error = "cannot return from top-level code";
      return false;
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
      auto fn = std::make_shared<RuntimeFunction>(s->name, s->params, s->body, env_, false);
      env_->Define(s->name, Value(std::static_pointer_cast<RuntimeObject>(fn)));
      return;
    }
    if (auto s = std::dynamic_pointer_cast<ClassStmt>(stmt); s != nullptr) {
      std::shared_ptr<RuntimeClass> superclass = nullptr;
      if (!s->superclass.empty()) {
        Value super_value;
        if (!env_->Get(s->superclass, &super_value)) {
          throw RuntimeSignal(RuntimeError("MS4001", "undefined variable: " + s->superclass));
        }
        if (!super_value.IsObject() || super_value.AsObject() == nullptr) {
          throw RuntimeSignal(RuntimeError("MS4003", "superclass must be a class"));
        }
        superclass = std::dynamic_pointer_cast<RuntimeClass>(super_value.AsObject());
        if (superclass == nullptr) {
          throw RuntimeSignal(RuntimeError("MS4003", "superclass must be a class"));
        }
      }

      env_->Define(s->name, Value::Nil());

      auto method_env = env_;
      if (superclass != nullptr) {
        method_env = std::make_shared<Environment>(vm_, env_);
        method_env->Define("super", Value(std::static_pointer_cast<RuntimeObject>(superclass)));
      }

      std::unordered_map<std::string, std::shared_ptr<RuntimeFunction>> methods;
      for (const auto& method : s->methods) {
        const bool is_initializer = method->name == "init";
        methods[method->name] = std::make_shared<RuntimeFunction>(
            method->name, method->params, method->body, method_env, is_initializer);
      }

      auto klass = std::make_shared<RuntimeClass>(s->name, superclass, std::move(methods));
      if (!env_->Assign(s->name, Value(std::static_pointer_cast<RuntimeObject>(klass)))) {
        throw RuntimeSignal(RuntimeError("MS4001", "failed to bind class: " + s->name));
      }
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
        throw RuntimeSignal("module error (MS5002): module '" + s->module + "' has no symbol '" +
                            s->symbol + "'");
      }
      env_->Define(s->alias, exported);
      return;
    }
    throw RuntimeSignal(RuntimeError("MS4003", "unsupported statement"));
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
      if (const auto depth = LookupDepth(expr); depth.has_value()) {
        if (!env_->GetAt(*depth, e->name, &v)) {
          throw RuntimeSignal(RuntimeError("MS4001", "undefined variable: " + e->name));
        }
      } else {
        if (!env_->Get(e->name, &v)) {
          throw RuntimeSignal(RuntimeError("MS4001", "undefined variable: " + e->name));
        }
      }
      return v;
    }
    if (auto e = std::dynamic_pointer_cast<AssignExpr>(expr); e != nullptr) {
      Value v = Eval(e->value);
      if (const auto depth = LookupDepth(expr); depth.has_value()) {
        if (!env_->AssignAt(*depth, e->name, v)) {
          throw RuntimeSignal(RuntimeError("MS4001", "undefined variable: " + e->name));
        }
      } else {
        if (!env_->Assign(e->name, v)) {
          throw RuntimeSignal(RuntimeError("MS4001", "undefined variable: " + e->name));
        }
      }
      return v;
    }
    if (auto e = std::dynamic_pointer_cast<GroupingExpr>(expr); e != nullptr) return Eval(e->expression);
    if (auto e = std::dynamic_pointer_cast<UnaryExpr>(expr); e != nullptr) {
      Value r = Eval(e->right);
      if (!r.IsNumber()) throw RuntimeSignal(RuntimeError("MS4003", "operand must be number"));
      return Value(-r.AsNumber());
    }
    if (auto e = std::dynamic_pointer_cast<BinaryExpr>(expr); e != nullptr) {
      Value l = Eval(e->left);
      Value r = Eval(e->right);
      if (e->op == TokenType::kPlus) {
        if (l.IsNumber() && r.IsNumber()) return Value(l.AsNumber() + r.AsNumber());
        if (l.IsString() && r.IsString()) return Value(l.AsString() + r.AsString());
        throw RuntimeSignal(RuntimeError("MS4003", "operands must be two numbers or two strings"));
      }
      if (!l.IsNumber() || !r.IsNumber()) {
        throw RuntimeSignal(RuntimeError("MS4003", "operands must be numbers"));
      }
      if (e->op == TokenType::kMinus) return Value(l.AsNumber() - r.AsNumber());
      if (e->op == TokenType::kStar) return Value(l.AsNumber() * r.AsNumber());
      return Value(l.AsNumber() / r.AsNumber());
    }
    if (auto e = std::dynamic_pointer_cast<FunctionExpr>(expr); e != nullptr) {
      auto fn = std::make_shared<RuntimeFunction>("", e->params, e->body, env_, false);
      return Value(std::static_pointer_cast<RuntimeObject>(fn));
    }
    if (auto e = std::dynamic_pointer_cast<GetExpr>(expr); e != nullptr) {
      Value object = Eval(e->object);
      if (!object.IsObject() || object.AsObject() == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4003", "only instances have properties"));
      }
      auto instance = std::dynamic_pointer_cast<RuntimeInstance>(object.AsObject());
      if (instance == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4003", "only instances have properties"));
      }
      Value out;
      if (!instance->Get(e->name, &out)) {
        throw RuntimeSignal(RuntimeError("MS4004", "undefined property: " + e->name));
      }
      return out;
    }
    if (auto e = std::dynamic_pointer_cast<SetExpr>(expr); e != nullptr) {
      Value object = Eval(e->object);
      if (!object.IsObject() || object.AsObject() == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4003", "only instances have fields"));
      }
      auto instance = std::dynamic_pointer_cast<RuntimeInstance>(object.AsObject());
      if (instance == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4003", "only instances have fields"));
      }
      Value value = Eval(e->value);
      instance->Set(e->name, value);
      return value;
    }
    if (auto e = std::dynamic_pointer_cast<ThisExpr>(expr); e != nullptr) {
      Value v;
      if (!ResolveScopedRead(expr, e->name, &v)) {
        throw RuntimeSignal(RuntimeError("MS4003", "cannot use 'this' outside of a class"));
      }
      return v;
    }
    if (auto e = std::dynamic_pointer_cast<SuperExpr>(expr); e != nullptr) {
      Value super_value;
      if (!ResolveScopedRead(expr, "super", &super_value)) {
        throw RuntimeSignal(RuntimeError("MS4003", "cannot use 'super' outside of a subclass"));
      }
      auto superclass = std::dynamic_pointer_cast<RuntimeClass>(super_value.AsObject());
      if (superclass == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4003", "invalid superclass reference"));
      }
      Value this_value;
      if (!env_->Get("this", &this_value)) {
        throw RuntimeSignal(RuntimeError("MS4003", "missing receiver for 'super' call"));
      }
      auto method = superclass->FindMethod(e->method);
      if (method == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4004", "undefined property: " + e->method));
      }
      auto bound = std::make_shared<RuntimeBoundMethod>(this_value, method);
      return Value(std::static_pointer_cast<RuntimeObject>(bound));
    }
    if (auto e = std::dynamic_pointer_cast<CallExpr>(expr); e != nullptr) {
      Value callee = Eval(e->callee);
      if (!callee.IsObject() || callee.AsObject() == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4005", "can only call functions and classes"));
      }
      auto callable = std::dynamic_pointer_cast<RuntimeCallable>(callee.AsObject());
      if (callable == nullptr) {
        throw RuntimeSignal(RuntimeError("MS4005", "can only call functions and classes"));
      }
      std::vector<Value> args;
      for (const auto& arg : e->arguments) args.push_back(Eval(arg));
      if (args.size() != callable->Arity()) {
        throw RuntimeSignal(RuntimeError("MS4002", "expected " + std::to_string(callable->Arity()) +
                                                       " arguments but got " +
                                                       std::to_string(args.size())));
      }
      Value out = Value::Nil();
      std::string call_error;
      if (!callable->Call(*this, args, &out, &call_error)) throw RuntimeSignal(call_error);
      return out;
    }
    throw RuntimeSignal(RuntimeError("MS4003", "unsupported expression"));
  }

  std::optional<std::size_t> LookupDepth(const ExprPtr& expr) const {
    if (expr == nullptr || local_depths_ == nullptr) {
      return std::nullopt;
    }
    const auto it = local_depths_->find(expr.get());
    if (it == local_depths_->end()) {
      return std::nullopt;
    }
    return it->second;
  }

  bool ResolveScopedRead(const ExprPtr& expr, const std::string& name, Value* out) const {
    if (const auto depth = LookupDepth(expr); depth.has_value()) {
      return env_->GetAt(*depth, name, out);
    }
    return env_->Get(name, out);
  }

  Vm& vm_;
  EnvironmentPtr globals_;
  EnvironmentPtr env_;
  const std::unordered_map<const Expr*, std::size_t>* local_depths_;

  friend class RuntimeFunction;
  friend class RuntimeClass;
  friend class RuntimeBoundMethod;
};

bool RuntimeFunction::Call(Executor& executor, const std::vector<Value>& args, Value* out, std::string* error) {
  auto scope = std::make_shared<Environment>(executor.vm_, closure_);
  for (std::size_t i = 0; i < params_.size(); ++i) scope->Define(params_[i], args[i]);
  Value call_result = Value::Nil();
  if (!executor.RunFunction(body_, scope, &call_result, error)) {
    return false;
  }
  if (is_initializer_) {
    Value receiver;
    if (!closure_->Get("this", &receiver)) {
      if (error != nullptr) {
        *error = "initializer is missing receiver";
      }
      return false;
    }
    if (out != nullptr) {
      *out = receiver;
    }
    return true;
  }
  if (out != nullptr) {
    *out = call_result;
  }
  return true;
}

std::shared_ptr<RuntimeFunction> RuntimeFunction::Bind(const Value& instance, Vm& vm) const {
  auto bound_env = std::make_shared<Environment>(vm, closure_);
  bound_env->Define("this", instance);
  return std::make_shared<RuntimeFunction>(name_, params_, body_, bound_env, is_initializer_);
}

std::shared_ptr<RuntimeFunction> RuntimeClass::FindMethod(const std::string& name) const {
  const auto it = methods_.find(name);
  if (it != methods_.end()) {
    return it->second;
  }
  if (superclass_ != nullptr) {
    return superclass_->FindMethod(name);
  }
  return nullptr;
}

std::size_t RuntimeClass::Arity() const {
  auto init = FindMethod("init");
  return init != nullptr ? init->Arity() : 0;
}

bool RuntimeClass::Call(Executor& executor, const std::vector<Value>& args, Value* out,
                        std::string* error) {
  auto instance = std::make_shared<RuntimeInstance>(shared_from_this());
  Value receiver(std::static_pointer_cast<RuntimeObject>(instance));

  auto init = FindMethod("init");
  if (init != nullptr) {
    auto bound = init->Bind(receiver, executor.vm_);
    Value ignored = Value::Nil();
    if (!bound->Call(executor, args, &ignored, error)) {
      return false;
    }
  }

  if (out != nullptr) {
    *out = receiver;
  }
  return true;
}

bool RuntimeInstance::Get(const std::string& name, Value* out) {
  const auto field = fields_.find(name);
  if (field != fields_.end()) {
    if (out != nullptr) {
      *out = field->second;
    }
    return true;
  }
  auto method = klass_->FindMethod(name);
  if (method == nullptr) {
    return false;
  }
  auto bound = std::make_shared<RuntimeBoundMethod>(
      Value(std::static_pointer_cast<RuntimeObject>(shared_from_this())), method);
  if (out != nullptr) {
    *out = Value(std::static_pointer_cast<RuntimeObject>(bound));
  }
  return true;
}

bool RuntimeBoundMethod::Call(Executor& executor, const std::vector<Value>& args, Value* out,
                              std::string* error) {
  return method_->Bind(receiver_, executor.vm_)->Call(executor, args, out, error);
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
  Resolver resolver;
  ResolveResult resolved = resolver.Resolve(program);
  if (!resolved.errors.empty()) {
    if (error != nullptr) {
      *error = resolved.errors.front();
    }
    return false;
  }
  Executor executor(vm, &resolved.local_depths);
  return executor.Run(program, error);
}

bool ScriptInterpreter::IsCompileLikeError(const std::string& error) {
  return error.find("parse error:") != std::string::npos ||
         error.find("lexer error:") != std::string::npos ||
         error.find("resolve error") != std::string::npos;
}

}  // namespace ms
