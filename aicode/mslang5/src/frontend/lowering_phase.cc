#include "frontend/lowering_phase.hh"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <sstream>
#include <utility>

#include "bytecode/opcode.hh"
#include "frontend/parser.hh"

namespace ms {

namespace {

class CompilerImpl {
 public:
  explicit CompilerImpl(Parser parser) : parser_(std::move(parser)) {
    auto script = std::make_unique<FunctionContext>();
    script->prototype = std::make_shared<FunctionPrototype>();
    script->prototype->name = "<script>";
    script->prototype->chunk = std::make_shared<Chunk>();
    script->scope_depth = 0;
    script->type = FunctionType::kScript;
    current_ = script.get();
    contexts_.push_back(std::move(script));
  }

  LoweringResult compile() {
    while (!parser_.is_at_end()) {
      skip_statement_separators();
      if (parser_.is_at_end()) {
        break;
      }
      declaration();
    }
    emit_op(OpCode::kReturn, current_line());

    result_.chunk = *current_->prototype->chunk;
    result_.errors.insert(result_.errors.end(), parser_.errors().begin(), parser_.errors().end());
    result_.errors.insert(result_.errors.end(), compile_errors_.begin(), compile_errors_.end());
    return std::move(result_);
  }

 private:
  enum class FunctionType { kScript, kFunction, kMethod, kInitializer };

  struct Local {
    std::string name;
    int depth = -1;
    bool is_captured = false;
  };

  struct Upvalue {
    std::uint8_t index = 0;
    bool is_local = false;
  };

  struct FunctionContext {
    std::shared_ptr<FunctionPrototype> prototype;
    std::vector<Local> locals;
    std::vector<Upvalue> upvalues;
    int scope_depth = 0;
    FunctionType type = FunctionType::kScript;
    FunctionContext* enclosing = nullptr;
  };

  struct ClassContext {
    ClassContext* enclosing = nullptr;
    bool has_superclass = false;
    std::string superclass_name;
  };

  Chunk& chunk() { return *current_->prototype->chunk; }

  const Chunk& chunk() const { return *current_->prototype->chunk; }

  std::size_t add_constant(Constant constant) { return chunk().add_constant(std::move(constant)); }

  std::size_t current_line() const { return parser_.current().line; }

  void report_parse_error(const std::size_t line, const std::string& message) {
    std::ostringstream out;
    out << "[line " << line << "] parse error: " << message;
    compile_errors_.push_back(out.str());
  }

  void report_parse_error(const std::string& message) { report_parse_error(current_line(), message); }

  void report_resolve_error(const std::size_t line, const std::string& code,
                            const std::string& message) {
    std::ostringstream out;
    out << "[line " << line << "] resolve error (" << code << "): " << message;
    compile_errors_.push_back(out.str());
  }

  void emit_op(const OpCode op, const std::size_t line) { chunk().write_op(op, line); }

  void emit_byte(const std::uint8_t byte, const std::size_t line) { chunk().write(byte, line); }

  void emit_constant(Constant constant, const std::size_t line) {
    const std::size_t index = add_constant(std::move(constant));
    emit_op(OpCode::kConstant, line);
    emit_byte(static_cast<std::uint8_t>(index), line);
  }

  void emit_name_op(const OpCode op, const std::string& name, const std::size_t line) {
    emit_op(op, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(name)), line);
  }

  void emit_property_op(const OpCode op, const std::string& name, const std::size_t line) {
    emit_op(op, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(name)), line);
  }

  void emit_invoke_op(const OpCode op, const std::string& name, const std::uint8_t arg_count,
                      const std::size_t line) {
    emit_op(op, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(name)), line);
    emit_byte(arg_count, line);
  }

  std::size_t emit_jump(const OpCode op) {
    const std::size_t line = current_line();
    emit_op(op, line);
    emit_byte(0xff, line);
    emit_byte(0xff, line);
    return chunk().code().size() - 2;
  }

  void patch_jump(const std::size_t offset_index) {
    const std::size_t jump = chunk().code().size() - offset_index - 2;
    auto& code = chunk().mutable_code();
    code[offset_index] = static_cast<std::uint8_t>((jump >> 8) & 0xff);
    code[offset_index + 1] = static_cast<std::uint8_t>(jump & 0xff);
  }

  void emit_loop(const std::size_t loop_start) {
    const std::size_t line = current_line();
    emit_op(OpCode::kLoop, line);
    const std::size_t offset = chunk().code().size() + 2 - loop_start;
    emit_byte(static_cast<std::uint8_t>((offset >> 8) & 0xff), line);
    emit_byte(static_cast<std::uint8_t>(offset & 0xff), line);
  }

  void begin_scope() { ++current_->scope_depth; }

  void end_scope() {
    --current_->scope_depth;
    while (!current_->locals.empty() && current_->locals.back().depth > current_->scope_depth) {
      if (current_->locals.back().is_captured) {
        emit_op(OpCode::kCloseUpvalue, current_line());
      } else {
        emit_op(OpCode::kPop, current_line());
      }
      current_->locals.pop_back();
    }
  }

  void declare_local(const std::string& name) {
    if (current_->scope_depth == 0) {
      return;
    }
    for (int i = static_cast<int>(current_->locals.size()) - 1; i >= 0; --i) {
      const Local& local = current_->locals[static_cast<std::size_t>(i)];
      if (local.depth != -1 && local.depth < current_->scope_depth) {
        break;
      }
      if (local.name == name) {
        report_parse_error("already a variable with this name in this scope");
      }
    }
    current_->locals.push_back(Local{name, -1, false});
  }

  void mark_initialized() {
    if (current_->scope_depth == 0 || current_->locals.empty()) {
      return;
    }
    current_->locals.back().depth = current_->scope_depth;
  }

  std::optional<std::uint8_t> resolve_local(FunctionContext* context,
                                            const std::string& name) const {
    for (int i = static_cast<int>(context->locals.size()) - 1; i >= 0; --i) {
      if (context->locals[static_cast<std::size_t>(i)].name == name) {
        return static_cast<std::uint8_t>(i);
      }
    }
    return std::nullopt;
  }

  std::uint8_t add_upvalue(FunctionContext* context, const std::uint8_t index,
                           const bool is_local) {
    for (std::size_t i = 0; i < context->upvalues.size(); ++i) {
      const Upvalue& upvalue = context->upvalues[i];
      if (upvalue.index == index && upvalue.is_local == is_local) {
        return static_cast<std::uint8_t>(i);
      }
    }
    context->upvalues.push_back(Upvalue{index, is_local});
    context->prototype->upvalue_count = static_cast<int>(context->upvalues.size());
    return static_cast<std::uint8_t>(context->upvalues.size() - 1);
  }

  std::optional<std::uint8_t> resolve_upvalue(FunctionContext* context,
                                              const std::string& name) {
    if (context->enclosing == nullptr) {
      return std::nullopt;
    }

    if (const auto local = resolve_local(context->enclosing, name); local.has_value()) {
      context->enclosing->locals[*local].is_captured = true;
      return add_upvalue(context, *local, true);
    }

    if (const auto upvalue = resolve_upvalue(context->enclosing, name); upvalue.has_value()) {
      return add_upvalue(context, *upvalue, false);
    }

    return std::nullopt;
  }

  void emit_variable_get(const std::string& name, const std::size_t line) {
    if (const auto local = resolve_local(current_, name); local.has_value()) {
      emit_op(OpCode::kGetLocal, line);
      emit_byte(*local, line);
      return;
    }
    if (const auto upvalue = resolve_upvalue(current_, name); upvalue.has_value()) {
      emit_op(OpCode::kGetUpvalue, line);
      emit_byte(*upvalue, line);
      return;
    }
    emit_name_op(OpCode::kGetGlobal, name, line);
  }
  void skip_statement_separators() { parser_.skip_newline_tokens(); }

  bool consume_statement_end(const std::string& message) {
    return parser_.consume_statement_end(message);
  }

  bool match_for_clause_separator() {
    if (parser_.match(TokenType::kSemicolon) || parser_.match(TokenType::kNewline)) {
      parser_.skip_newline_tokens();
      return true;
    }
    return false;
  }

  bool consume_for_clause_separator(const std::string& message) {
    if (match_for_clause_separator()) {
      return true;
    }
    report_parse_error(message);
    return false;
  }

  void declaration() {
    skip_statement_separators();
    if (parser_.is_at_end()) {
      return;
    }
    if (parser_.match(TokenType::kClass)) {
      class_declaration();
      return;
    }
    if (parser_.match(TokenType::kFun)) {
      fun_declaration();
      return;
    }
    if (parser_.match(TokenType::kVar)) {
      var_declaration();
      return;
    }
    statement();
  }

  void class_declaration() {
    if (!parser_.consume(TokenType::kIdentifier, "expected class name")) {
      return;
    }
    const std::string class_name = parser_.previous().lexeme;
    const std::size_t class_line = parser_.previous().line;

    declare_local(class_name);

    emit_op(OpCode::kClass, class_line);
    emit_byte(static_cast<std::uint8_t>(add_constant(class_name)), class_line);

    if (current_->scope_depth == 0) {
      emit_name_op(OpCode::kDefineGlobal, class_name, class_line);
    } else {
      mark_initialized();
    }

    ClassContext klass;
    klass.enclosing = current_class_;
    current_class_ = &klass;

    if (parser_.match(TokenType::kLess)) {
      if (!parser_.consume(TokenType::kIdentifier, "expected superclass name")) {
        current_class_ = current_class_->enclosing;
        return;
      }
      const std::string super_name = parser_.previous().lexeme;
      const std::size_t super_line = parser_.previous().line;
      if (super_name == class_name) {
        report_resolve_error(super_line, "MS3004", "a class cannot inherit from itself");
      }

      emit_variable_get(class_name, class_line);
      emit_variable_get(super_name, super_line);
      emit_op(OpCode::kInherit, super_line);

      current_class_->has_superclass = true;
      current_class_->superclass_name = super_name;
    }

    emit_variable_get(class_name, class_line);

    parser_.consume(TokenType::kLeftBrace, "expected '{' before class body");
    while (!parser_.check(TokenType::kRightBrace) && !parser_.is_at_end()) {
      skip_statement_separators();
      if (parser_.check(TokenType::kRightBrace) || parser_.is_at_end()) {
        break;
      }
      method();
    }
    parser_.consume(TokenType::kRightBrace, "expected '}' after class body");
    skip_statement_separators();

    emit_op(OpCode::kPop, current_line());
    current_class_ = current_class_->enclosing;
  }

  void method() {
    if (!parser_.consume(TokenType::kIdentifier, "expected method name")) {
      return;
    }
    const std::string method_name = parser_.previous().lexeme;
    const std::size_t method_line = parser_.previous().line;

    FunctionType type = FunctionType::kMethod;
    if (method_name == "init") {
      type = FunctionType::kInitializer;
    }
    function_body(method_name, type);

    emit_op(OpCode::kMethod, method_line);
    emit_byte(static_cast<std::uint8_t>(add_constant(method_name)), method_line);
  }

  void fun_declaration() {
    if (!parser_.consume(TokenType::kIdentifier, "expected function name")) {
      return;
    }
    const std::string name = parser_.previous().lexeme;
    if (current_->scope_depth > 0) {
      declare_local(name);
      mark_initialized();
    }
    function_body(name, FunctionType::kFunction);
    if (current_->scope_depth == 0) {
      emit_name_op(OpCode::kDefineGlobal, name, current_line());
    }
  }

  void function_body(const std::string& name, const FunctionType type) {
    auto nested = std::make_unique<FunctionContext>();
    nested->prototype = std::make_shared<FunctionPrototype>();
    nested->prototype->name = name;
    nested->prototype->chunk = std::make_shared<Chunk>();
    nested->scope_depth = 1;
    nested->type = type;
    nested->enclosing = current_;
    const std::string first_local =
        (type == FunctionType::kMethod || type == FunctionType::kInitializer) ? "this" : "";
    nested->locals.push_back(Local{first_local, 0, false});

    FunctionContext* enclosing = current_;
    current_ = nested.get();
    contexts_.push_back(std::move(nested));

    parser_.consume(TokenType::kLeftParen, "expected '(' after function name");
    if (!parser_.check(TokenType::kRightParen)) {
      do {
        ++current_->prototype->arity;
        if (!parser_.consume(TokenType::kIdentifier, "expected parameter name")) {
          break;
        }
        declare_local(parser_.previous().lexeme);
        mark_initialized();
      } while (parser_.match(TokenType::kComma));
    }
    parser_.consume(TokenType::kRightParen, "expected ')' after parameters");
    parser_.consume(TokenType::kLeftBrace, "expected '{' before function body");
    while (!parser_.check(TokenType::kRightBrace) && !parser_.is_at_end()) {
      skip_statement_separators();
      if (parser_.check(TokenType::kRightBrace) || parser_.is_at_end()) {
        break;
      }
      declaration();
    }
    parser_.consume(TokenType::kRightBrace, "expected '}' after function body");
    skip_statement_separators();

    if (type == FunctionType::kInitializer) {
      emit_op(OpCode::kGetLocal, current_line());
      emit_byte(0, current_line());
    } else {
      emit_constant(std::monostate{}, current_line());
    }
    emit_op(OpCode::kReturn, current_line());

    const std::shared_ptr<FunctionPrototype> compiled = current_->prototype;
    const std::vector<Upvalue> compiled_upvalues = current_->upvalues;
    current_ = enclosing;

    emit_op(OpCode::kClosure, current_line());
    emit_byte(static_cast<std::uint8_t>(add_constant(compiled)), current_line());
    for (const auto& upvalue : compiled_upvalues) {
      emit_byte(upvalue.is_local ? 1 : 0, current_line());
      emit_byte(upvalue.index, current_line());
    }
  }

  void statement() {
    skip_statement_separators();
    if (parser_.is_at_end() || parser_.check(TokenType::kRightBrace)) {
      return;
    }
    if (parser_.match(TokenType::kPrint)) {
      print_statement();
      return;
    }
    if (parser_.match(TokenType::kReturn)) {
      return_statement();
      return;
    }
    if (parser_.match(TokenType::kIf)) {
      if_statement();
      return;
    }
    if (parser_.match(TokenType::kWhile)) {
      while_statement();
      return;
    }
    if (parser_.match(TokenType::kFor)) {
      for_statement();
      return;
    }
    if (parser_.match(TokenType::kLeftBrace)) {
      begin_scope();
      block();
      end_scope();
      return;
    }
    if (parser_.match(TokenType::kImport)) {
      import_statement();
      return;
    }
    if (parser_.match(TokenType::kFrom)) {
      from_import_statement();
      return;
    }
    if (parser_.check(TokenType::kIdentifier) && parser_.peek(1).type == TokenType::kEqual) {
      assignment_statement();
      return;
    }
    expression_statement();
  }

  void block() {
    while (!parser_.check(TokenType::kRightBrace) && !parser_.is_at_end()) {
      skip_statement_separators();
      if (parser_.check(TokenType::kRightBrace) || parser_.is_at_end()) {
        break;
      }
      declaration();
    }
    parser_.consume(TokenType::kRightBrace, "expected '}' after block");
    skip_statement_separators();
  }

  void print_statement() {
    expression();
    consume_statement_end("expected statement end after print");
    emit_op(OpCode::kPrint, current_line());
  }

  void return_statement() {
    const std::size_t return_line = parser_.previous().line;
    if (current_->type == FunctionType::kScript) {
      report_resolve_error(return_line, "MS3001", "cannot return from top-level code");
      while (!parser_.check(TokenType::kSemicolon) && !parser_.check(TokenType::kNewline) &&
             !parser_.check(TokenType::kRightBrace) && !parser_.is_at_end()) {
        parser_.advance();
      }
      consume_statement_end("expected statement end after return");
      return;
    }

    if (parser_.match(TokenType::kSemicolon) || parser_.match(TokenType::kNewline) ||
        parser_.check(TokenType::kRightBrace) || parser_.is_at_end()) {
      parser_.skip_newline_tokens();
      if (current_->type == FunctionType::kInitializer) {
        emit_op(OpCode::kGetLocal, return_line);
        emit_byte(0, return_line);
      } else {
        emit_constant(std::monostate{}, return_line);
      }
      emit_op(OpCode::kReturn, return_line);
      return;
    }

    expression();
    consume_statement_end("expected statement end after return value");
    if (current_->type == FunctionType::kInitializer) {
      emit_op(OpCode::kPop, return_line);
      emit_op(OpCode::kGetLocal, return_line);
      emit_byte(0, return_line);
    }
    emit_op(OpCode::kReturn, return_line);
  }

  void expression_statement() {
    expression();
    consume_statement_end("expected statement end after expression");
    emit_op(OpCode::kPop, current_line());
  }

  void if_statement() {
    parser_.consume(TokenType::kLeftParen, "expected '(' after 'if'");
    expression();
    parser_.consume(TokenType::kRightParen, "expected ')' after if condition");

    const std::size_t then_jump = emit_jump(OpCode::kJumpIfFalse);
    emit_op(OpCode::kPop, current_line());
    statement();
    const std::size_t else_jump = emit_jump(OpCode::kJump);

    patch_jump(then_jump);
    emit_op(OpCode::kPop, current_line());
    if (parser_.match(TokenType::kElse)) {
      statement();
    }
    patch_jump(else_jump);
  }

  void while_statement() {
    const std::size_t loop_start = chunk().code().size();
    parser_.consume(TokenType::kLeftParen, "expected '(' after 'while'");
    expression();
    parser_.consume(TokenType::kRightParen, "expected ')' after condition");

    const std::size_t exit_jump = emit_jump(OpCode::kJumpIfFalse);
    emit_op(OpCode::kPop, current_line());
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_op(OpCode::kPop, current_line());
  }

  void for_statement() {
    begin_scope();
    parser_.consume(TokenType::kLeftParen, "expected '(' after 'for'");
    parser_.skip_newline_tokens();

    if (match_for_clause_separator()) {
      // No initializer.
    } else if (parser_.match(TokenType::kVar)) {
      var_declaration();
    } else {
      expression_statement();
    }

    std::size_t loop_start = chunk().code().size();
    std::optional<std::size_t> exit_jump;
    if (!match_for_clause_separator()) {
      expression();
      consume_for_clause_separator("expected separator after loop condition");
      exit_jump = emit_jump(OpCode::kJumpIfFalse);
      emit_op(OpCode::kPop, current_line());
    }

    parser_.skip_newline_tokens();
    if (!parser_.match(TokenType::kRightParen)) {
      const std::size_t body_jump = emit_jump(OpCode::kJump);
      const std::size_t increment_start = chunk().code().size();
      bool increment_produced_value = true;
      if (parser_.check(TokenType::kIdentifier) && parser_.peek(1).type == TokenType::kEqual) {
        parser_.consume(TokenType::kIdentifier, "expected variable name");
        const std::string name = parser_.previous().lexeme;
        parser_.consume(TokenType::kEqual, "expected '=' in increment expression");
        expression();
        emit_assignment(name);
        increment_produced_value = false;
      } else {
        expression();
      }
      if (increment_produced_value) {
        emit_op(OpCode::kPop, current_line());
      }
      parser_.skip_newline_tokens();
      parser_.consume(TokenType::kRightParen, "expected ')' after for clauses");

      emit_loop(loop_start);
      loop_start = increment_start;
      patch_jump(body_jump);
    }

    statement();
    emit_loop(loop_start);

    if (exit_jump.has_value()) {
      patch_jump(*exit_jump);
      emit_op(OpCode::kPop, current_line());
    }
    end_scope();
  }

  void var_declaration() {
    if (!parser_.consume(TokenType::kIdentifier, "expected variable name")) {
      return;
    }
    const std::string name = parser_.previous().lexeme;
    declare_local(name);

    if (parser_.match(TokenType::kEqual)) {
      expression();
    } else {
      emit_constant(std::monostate{}, current_line());
    }
    consume_statement_end("expected statement end after variable declaration");

    if (current_->scope_depth == 0) {
      emit_name_op(OpCode::kDefineGlobal, name, current_line());
      return;
    }
    mark_initialized();
  }

  void assignment_statement() {
    parser_.consume(TokenType::kIdentifier, "expected variable name");
    const std::string name = parser_.previous().lexeme;
    parser_.consume(TokenType::kEqual, "expected '=' in assignment");
    expression();
    emit_assignment(name);
    consume_statement_end("expected statement end after assignment");
  }

  void import_statement() {
    const std::string module = parser_.parse_dotted_name();
    consume_statement_end("expected statement end after import statement");
    emit_name_op(OpCode::kImportModule, module, current_line());
  }

  void from_import_statement() {
    const std::string module = parser_.parse_dotted_name();
    parser_.consume(TokenType::kImport, "expected 'import' keyword");
    parser_.consume(TokenType::kIdentifier, "expected imported symbol name");
    const std::string symbol = parser_.previous().lexeme;
    std::string alias = symbol;
    if (parser_.match(TokenType::kAs)) {
      parser_.consume(TokenType::kIdentifier, "expected alias name after 'as'");
      alias = parser_.previous().lexeme;
    }
    consume_statement_end("expected statement end after from-import statement");

    const std::size_t line = current_line();
    emit_op(OpCode::kImportSymbol, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(module)), line);
    emit_byte(static_cast<std::uint8_t>(add_constant(symbol)), line);
    emit_byte(static_cast<std::uint8_t>(add_constant(alias)), line);
  }

  void expression() { logic_or(); }

  void emit_assignment(const std::string& name) {
    if (const auto local = resolve_local(current_, name); local.has_value()) {
      emit_op(OpCode::kSetLocal, current_line());
      emit_byte(*local, current_line());
      return;
    }
    if (const auto upvalue = resolve_upvalue(current_, name); upvalue.has_value()) {
      emit_op(OpCode::kSetUpvalue, current_line());
      emit_byte(*upvalue, current_line());
      return;
    }
    emit_name_op(OpCode::kSetGlobal, name, current_line());
  }

  void logic_or() {
    logic_and();
    while (parser_.match(TokenType::kOr)) {
      const std::size_t else_jump = emit_jump(OpCode::kJumpIfFalse);
      const std::size_t end_jump = emit_jump(OpCode::kJump);
      patch_jump(else_jump);
      emit_op(OpCode::kPop, current_line());
      logic_and();
      patch_jump(end_jump);
    }
  }

  void logic_and() {
    equality();
    while (parser_.match(TokenType::kAnd)) {
      const std::size_t end_jump = emit_jump(OpCode::kJumpIfFalse);
      emit_op(OpCode::kPop, current_line());
      equality();
      patch_jump(end_jump);
    }
  }

  void equality() {
    comparison();
    while (parser_.match(TokenType::kEqualEqual) || parser_.match(TokenType::kBangEqual)) {
      const TokenType op = parser_.previous().type;
      comparison();
      emit_op(OpCode::kEqual, current_line());
      if (op == TokenType::kBangEqual) {
        emit_op(OpCode::kNot, current_line());
      }
    }
  }

  void comparison() {
    term();
    while (parser_.match(TokenType::kGreater) || parser_.match(TokenType::kGreaterEqual) ||
           parser_.match(TokenType::kLess) || parser_.match(TokenType::kLessEqual)) {
      const TokenType op = parser_.previous().type;
      term();
      switch (op) {
        case TokenType::kGreater:
          emit_op(OpCode::kGreater, current_line());
          break;
        case TokenType::kGreaterEqual:
          emit_op(OpCode::kLess, current_line());
          emit_op(OpCode::kNot, current_line());
          break;
        case TokenType::kLess:
          emit_op(OpCode::kLess, current_line());
          break;
        case TokenType::kLessEqual:
          emit_op(OpCode::kGreater, current_line());
          emit_op(OpCode::kNot, current_line());
          break;
        default:
          break;
      }
    }
  }

  void term() {
    factor();
    while (parser_.match(TokenType::kPlus) || parser_.match(TokenType::kMinus)) {
      const TokenType op = parser_.previous().type;
      factor();
      emit_op(op == TokenType::kPlus ? OpCode::kAdd : OpCode::kSubtract, current_line());
    }
  }

  void factor() {
    unary();
    while (parser_.match(TokenType::kStar) || parser_.match(TokenType::kSlash)) {
      const TokenType op = parser_.previous().type;
      unary();
      emit_op(op == TokenType::kStar ? OpCode::kMultiply : OpCode::kDivide, current_line());
    }
  }

  void unary() {
    if (parser_.match(TokenType::kBang)) {
      unary();
      emit_op(OpCode::kNot, current_line());
      return;
    }
    if (parser_.match(TokenType::kMinus)) {
      unary();
      emit_op(OpCode::kNegate, current_line());
      return;
    }
    call();
  }

  void call() {
    primary();
    while (true) {
      if (parser_.match(TokenType::kLeftParen)) {
        std::uint8_t arg_count = 0;
        if (!parser_.check(TokenType::kRightParen)) {
          do {
            expression();
            ++arg_count;
          } while (parser_.match(TokenType::kComma));
        }
        parser_.consume(TokenType::kRightParen, "expected ')' after arguments");
        emit_op(OpCode::kCall, current_line());
        emit_byte(arg_count, current_line());
      } else if (parser_.match(TokenType::kDot)) {
        parser_.consume(TokenType::kIdentifier, "expected property name after '.'");
        const std::string name = parser_.previous().lexeme;
        const std::size_t line = parser_.previous().line;
        if (parser_.match(TokenType::kEqual)) {
          expression();
          emit_property_op(OpCode::kSetProperty, name, line);
        } else if (parser_.match(TokenType::kLeftParen)) {
          std::uint8_t arg_count = 0;
          if (!parser_.check(TokenType::kRightParen)) {
            do {
              expression();
              ++arg_count;
            } while (parser_.match(TokenType::kComma));
          }
          parser_.consume(TokenType::kRightParen, "expected ')' after arguments");
          emit_invoke_op(OpCode::kInvoke, name, arg_count, line);
        } else {
          emit_property_op(OpCode::kGetProperty, name, line);
        }
      } else {
        break;
      }
    }
  }

  void primary() {
    if (parser_.match(TokenType::kNumber)) {
      emit_constant(std::strtod(parser_.previous().lexeme.c_str(), nullptr), current_line());
      return;
    }
    if (parser_.match(TokenType::kString)) {
      emit_constant(parser_.previous().lexeme, current_line());
      return;
    }
    if (parser_.match(TokenType::kTrue)) {
      emit_constant(true, current_line());
      return;
    }
    if (parser_.match(TokenType::kFalse)) {
      emit_constant(false, current_line());
      return;
    }
    if (parser_.match(TokenType::kNil)) {
      emit_constant(std::monostate{}, current_line());
      return;
    }
    if (parser_.match(TokenType::kThis)) {
      const std::size_t line = parser_.previous().line;
      if (current_class_ == nullptr) {
        report_resolve_error(line, "MS3002", "cannot use 'this' outside of a class");
      }
      emit_variable_get("this", line);
      return;
    }
    if (parser_.match(TokenType::kSuper)) {
      const std::size_t line = parser_.previous().line;
      const bool valid_super = current_class_ != nullptr && current_class_->has_superclass;
      if (!valid_super) {
        report_resolve_error(line, "MS3003", "cannot use 'super' outside of a subclass");
      }
      parser_.consume(TokenType::kDot, "expected '.' after 'super'");
      parser_.consume(TokenType::kIdentifier, "expected superclass method name");
      const std::string method = parser_.previous().lexeme;

      if (!valid_super) {
        emit_constant(std::monostate{}, line);
        return;
      }

      emit_variable_get("this", line);
      emit_variable_get(current_class_->superclass_name, line);
      emit_property_op(OpCode::kGetSuper, method, line);
      return;
    }
    if (parser_.match(TokenType::kIdentifier)) {
      const std::string name = parser_.previous().lexeme;
      emit_variable_get(name, current_line());
      return;
    }
    if (parser_.match(TokenType::kFun)) {
      function_body("anonymous", FunctionType::kFunction);
      return;
    }
    if (parser_.match(TokenType::kLeftParen)) {
      expression();
      parser_.consume(TokenType::kRightParen, "expected ')' after expression");
      return;
    }
    parser_.consume(TokenType::kIdentifier, "expected expression");
    if (!parser_.is_at_end()) {
      parser_.advance();
    }
  }

  Parser parser_;
  LoweringResult result_;
  std::vector<std::string> compile_errors_;
  std::vector<std::unique_ptr<FunctionContext>> contexts_;
  FunctionContext* current_ = nullptr;
  ClassContext* current_class_ = nullptr;
};

}  // namespace

LoweringResult run_lowering_phase(LoweringInput input) {
  (void)input.metadata;

  Parser parser(std::move(input.tokens));
  CompilerImpl compiler(std::move(parser));
  return compiler.compile();
}

}  // namespace ms














