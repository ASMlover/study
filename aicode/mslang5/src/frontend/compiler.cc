#include "frontend/compiler.hh"

#include <cstdlib>

#include "bytecode/opcode.hh"
#include "frontend/lexer.hh"
#include "frontend/parser.hh"

namespace ms {

namespace {

class CompilerImpl {
 public:
  explicit CompilerImpl(Parser parser) : parser_(std::move(parser)) {}

  CompileResult Compile() {
    while (!parser_.is_at_end()) {
      Statement();
    }
    EmitOp(OpCode::kReturn, CurrentLine());
    result_.errors.insert(result_.errors.end(), parser_.errors().begin(),
                          parser_.errors().end());
    return std::move(result_);
  }

 private:
  std::size_t add_constant(Constant constant) {
    return result_.chunk.add_constant(std::move(constant));
  }

  std::size_t CurrentLine() const { return parser_.current().line; }

  void EmitOp(const OpCode op, const std::size_t line) {
    result_.chunk.write_op(op, line);
  }

  void EmitByte(const std::uint8_t byte, const std::size_t line) {
    result_.chunk.write(byte, line);
  }

  void EmitConstant(Constant constant, const std::size_t line) {
    const std::size_t index = add_constant(std::move(constant));
    EmitOp(OpCode::kConstant, line);
    EmitByte(static_cast<std::uint8_t>(index), line);
  }

  void Statement() {
    if (parser_.match(TokenType::kPrint)) {
      Expression();
      parser_.consume(TokenType::kSemicolon, "expected ';' after print");
      EmitOp(OpCode::kPrint, CurrentLine());
      return;
    }

    if (parser_.match(TokenType::kVar)) {
      VarDeclaration();
      return;
    }

    if (parser_.match(TokenType::kImport)) {
      ImportStatement();
      return;
    }

    if (parser_.match(TokenType::kFrom)) {
      FromImportStatement();
      return;
    }

    if (parser_.check(TokenType::kIdentifier) &&
        parser_.peek(1).type == TokenType::kEqual) {
      Assignment();
      return;
    }

    Expression();
    parser_.consume(TokenType::kSemicolon, "expected ';' after expression");
    EmitOp(OpCode::kPop, CurrentLine());
  }

  void VarDeclaration() {
    if (!parser_.consume(TokenType::kIdentifier, "expected variable name")) {
      return;
    }
    const std::string name = parser_.previous().lexeme;
    if (!parser_.consume(TokenType::kEqual, "expected '=' in variable declaration")) {
      return;
    }
    Expression();
    parser_.consume(TokenType::kSemicolon, "expected ';' after variable declaration");
    EmitNameOp(OpCode::kDefineGlobal, name, CurrentLine());
  }

  void Assignment() {
    parser_.consume(TokenType::kIdentifier, "expected variable name");
    const std::string name = parser_.previous().lexeme;
    parser_.consume(TokenType::kEqual, "expected '=' in assignment");
    Expression();
    parser_.consume(TokenType::kSemicolon, "expected ';' after assignment");
    EmitNameOp(OpCode::kSetGlobal, name, CurrentLine());
  }

  void ImportStatement() {
    const std::string module = parser_.parse_dotted_name();
    parser_.consume(TokenType::kSemicolon, "expected ';' after import statement");
    EmitNameOp(OpCode::kImportModule, module, CurrentLine());
  }

  void FromImportStatement() {
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

    const std::size_t line = CurrentLine();
    EmitOp(OpCode::kImportSymbol, line);
    EmitByte(static_cast<std::uint8_t>(add_constant(module)), line);
    EmitByte(static_cast<std::uint8_t>(add_constant(symbol)), line);
    EmitByte(static_cast<std::uint8_t>(add_constant(alias)), line);
  }

  void EmitNameOp(const OpCode op, const std::string& name, const std::size_t line) {
    EmitOp(op, line);
    EmitByte(static_cast<std::uint8_t>(add_constant(name)), line);
  }

  void Expression() { Term(); }

  void Term() {
    Factor();
    while (parser_.match(TokenType::kPlus) || parser_.match(TokenType::kMinus)) {
      const TokenType op = parser_.previous().type;
      Factor();
      EmitOp(op == TokenType::kPlus ? OpCode::kAdd : OpCode::kSubtract, CurrentLine());
    }
  }

  void Factor() {
    Unary();
    while (parser_.match(TokenType::kStar) || parser_.match(TokenType::kSlash)) {
      const TokenType op = parser_.previous().type;
      Unary();
      EmitOp(op == TokenType::kStar ? OpCode::kMultiply : OpCode::kDivide, CurrentLine());
    }
  }

  void Unary() {
    if (parser_.match(TokenType::kMinus)) {
      Unary();
      EmitOp(OpCode::kNegate, CurrentLine());
      return;
    }
    Primary();
  }

  void Primary() {
    if (parser_.match(TokenType::kNumber)) {
      EmitConstant(std::strtod(parser_.previous().lexeme.c_str(), nullptr), CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kString)) {
      EmitConstant(parser_.previous().lexeme, CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kTrue)) {
      EmitConstant(true, CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kFalse)) {
      EmitConstant(false, CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kNil)) {
      EmitConstant(std::monostate{}, CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kIdentifier)) {
      EmitNameOp(OpCode::kGetGlobal, parser_.previous().lexeme, CurrentLine());
      return;
    }
    if (parser_.match(TokenType::kLeftParen)) {
      Expression();
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
};

}  // namespace

CompileResult compile_to_chunk(const std::string& source) {
  Lexer lexer(source);
  Parser parser(lexer.scan_all_tokens());
  CompilerImpl compiler(std::move(parser));
  return compiler.Compile();
}

}  // namespace ms
