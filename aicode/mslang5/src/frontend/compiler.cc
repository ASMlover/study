#include "frontend/compiler.hh"

#include <cstdint>
#include <cstdlib>
#include <optional>

#include "bytecode/opcode.hh"
#include "frontend/lexer.hh"
#include "frontend/parser.hh"

namespace ms {

namespace {

class CompilerImpl {
 public:
  explicit CompilerImpl(Parser parser) : parser_(std::move(parser)) {}

  CompileResult compile() {
    while (!parser_.is_at_end()) {
      declaration();
    }
    emit_op(OpCode::kReturn, current_line());
    result_.errors.insert(result_.errors.end(), parser_.errors().begin(), parser_.errors().end());
    return std::move(result_);
  }

 private:
  struct Local {
    std::string name;
    int depth = -1;
  };

  std::size_t add_constant(Constant constant) { return result_.chunk.add_constant(std::move(constant)); }

  std::size_t current_line() const { return parser_.current().line; }

  void emit_op(const OpCode op, const std::size_t line) { result_.chunk.write_op(op, line); }

  void emit_byte(const std::uint8_t byte, const std::size_t line) { result_.chunk.write(byte, line); }

  void emit_constant(Constant constant, const std::size_t line) {
    const std::size_t index = add_constant(std::move(constant));
    emit_op(OpCode::kConstant, line);
    emit_byte(static_cast<std::uint8_t>(index), line);
  }

  void emit_name_op(const OpCode op, const std::string& name, const std::size_t line) {
    emit_op(op, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(name)), line);
  }

  std::size_t emit_jump(const OpCode op) {
    const std::size_t line = current_line();
    emit_op(op, line);
    emit_byte(0xff, line);
    emit_byte(0xff, line);
    return result_.chunk.code().size() - 2;
  }

  void patch_jump(const std::size_t offset_index) {
    const std::size_t jump = result_.chunk.code().size() - offset_index - 2;
    auto& code = result_.chunk.mutable_code();
    code[offset_index] = static_cast<std::uint8_t>((jump >> 8) & 0xff);
    code[offset_index + 1] = static_cast<std::uint8_t>(jump & 0xff);
  }

  void emit_loop(const std::size_t loop_start) {
    const std::size_t line = current_line();
    emit_op(OpCode::kLoop, line);
    const std::size_t offset = result_.chunk.code().size() + 2 - loop_start;
    emit_byte(static_cast<std::uint8_t>((offset >> 8) & 0xff), line);
    emit_byte(static_cast<std::uint8_t>(offset & 0xff), line);
  }

  void begin_scope() { ++scope_depth_; }

  void end_scope() {
    --scope_depth_;
    while (!locals_.empty() && locals_.back().depth > scope_depth_) {
      emit_op(OpCode::kPop, current_line());
      locals_.pop_back();
    }
  }

  void declare_local(const std::string& name) {
    if (scope_depth_ == 0) {
      return;
    }
    locals_.push_back(Local{name, -1});
  }

  void mark_initialized() {
    if (scope_depth_ == 0 || locals_.empty()) {
      return;
    }
    locals_.back().depth = scope_depth_;
  }

  std::optional<std::uint8_t> resolve_local(const std::string& name) const {
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; --i) {
      if (locals_[i].name == name) {
        return static_cast<std::uint8_t>(i);
      }
    }
    return std::nullopt;
  }

  void declaration() {
    if (parser_.match(TokenType::kVar)) {
      var_declaration();
      return;
    }
    statement();
  }

  void statement() {
    if (parser_.match(TokenType::kPrint)) {
      print_statement();
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
      declaration();
    }
    parser_.consume(TokenType::kRightBrace, "expected '}' after block");
  }

  void print_statement() {
    expression();
    parser_.consume(TokenType::kSemicolon, "expected ';' after print");
    emit_op(OpCode::kPrint, current_line());
  }

  void expression_statement() {
    expression();
    parser_.consume(TokenType::kSemicolon, "expected ';' after expression");
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
    const std::size_t loop_start = result_.chunk.code().size();
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

    if (parser_.match(TokenType::kSemicolon)) {
      // No initializer.
    } else if (parser_.match(TokenType::kVar)) {
      var_declaration();
    } else {
      expression_statement();
    }

    std::size_t loop_start = result_.chunk.code().size();
    std::optional<std::size_t> exit_jump;
    if (!parser_.match(TokenType::kSemicolon)) {
      expression();
      parser_.consume(TokenType::kSemicolon, "expected ';' after loop condition");
      exit_jump = emit_jump(OpCode::kJumpIfFalse);
      emit_op(OpCode::kPop, current_line());
    }

    if (!parser_.match(TokenType::kRightParen)) {
      const std::size_t body_jump = emit_jump(OpCode::kJump);
      const std::size_t increment_start = result_.chunk.code().size();
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
    parser_.consume(TokenType::kSemicolon, "expected ';' after variable declaration");

    if (scope_depth_ == 0) {
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
    parser_.consume(TokenType::kSemicolon, "expected ';' after assignment");
  }

  void import_statement() {
    const std::string module = parser_.parse_dotted_name();
    parser_.consume(TokenType::kSemicolon, "expected ';' after import statement");
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
    parser_.consume(TokenType::kSemicolon, "expected ';' after from-import statement");

    const std::size_t line = current_line();
    emit_op(OpCode::kImportSymbol, line);
    emit_byte(static_cast<std::uint8_t>(add_constant(module)), line);
    emit_byte(static_cast<std::uint8_t>(add_constant(symbol)), line);
    emit_byte(static_cast<std::uint8_t>(add_constant(alias)), line);
  }

  void expression() { logic_or(); }

  void emit_assignment(const std::string& name) {
    const auto local = resolve_local(name);
    if (local.has_value()) {
      emit_op(OpCode::kSetLocal, current_line());
      emit_byte(*local, current_line());
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
    primary();
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
    if (parser_.match(TokenType::kIdentifier)) {
      const std::string name = parser_.previous().lexeme;
      const auto local = resolve_local(name);
      if (local.has_value()) {
        emit_op(OpCode::kGetLocal, current_line());
        emit_byte(*local, current_line());
      } else {
        emit_name_op(OpCode::kGetGlobal, name, current_line());
      }
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
  CompileResult result_;
  int scope_depth_ = 0;
  std::vector<Local> locals_;
};

}  // namespace

CompileResult compile_to_chunk(const std::string& source) {
  Lexer lexer(source);
  Parser parser(lexer.scan_all_tokens());
  CompilerImpl compiler(std::move(parser));
  return compiler.compile();
}

}  // namespace ms
