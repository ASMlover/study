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
    while (!parser_.IsAtEnd()) {
      Statement();
    }
    EmitOp(OpCode::kReturn, CurrentLine());
    result_.errors.insert(result_.errors.end(), parser_.Errors().begin(),
                          parser_.Errors().end());
    return std::move(result_);
  }

 private:
  std::size_t AddConstant(Constant constant) {
    return result_.chunk.AddConstant(std::move(constant));
  }

  std::size_t CurrentLine() const { return parser_.Current().line; }

  void EmitOp(const OpCode op, const std::size_t line) {
    result_.chunk.WriteOp(op, line);
  }

  void EmitByte(const std::uint8_t byte, const std::size_t line) {
    result_.chunk.Write(byte, line);
  }

  void EmitConstant(Constant constant, const std::size_t line) {
    const std::size_t index = AddConstant(std::move(constant));
    EmitOp(OpCode::kConstant, line);
    EmitByte(static_cast<std::uint8_t>(index), line);
  }

  void Statement() {
    if (parser_.Match(TokenType::kPrint)) {
      Expression();
      parser_.Consume(TokenType::kSemicolon, "expected ';' after print");
      EmitOp(OpCode::kPrint, CurrentLine());
      return;
    }

    if (parser_.Match(TokenType::kVar)) {
      VarDeclaration();
      return;
    }

    if (parser_.Match(TokenType::kImport)) {
      ImportStatement();
      return;
    }

    if (parser_.Match(TokenType::kFrom)) {
      FromImportStatement();
      return;
    }

    if (parser_.Check(TokenType::kIdentifier) &&
        parser_.Peek(1).type == TokenType::kEqual) {
      Assignment();
      return;
    }

    Expression();
    parser_.Consume(TokenType::kSemicolon, "expected ';' after expression");
    EmitOp(OpCode::kPop, CurrentLine());
  }

  void VarDeclaration() {
    if (!parser_.Consume(TokenType::kIdentifier, "expected variable name")) {
      return;
    }
    const std::string name = parser_.Previous().lexeme;
    if (!parser_.Consume(TokenType::kEqual, "expected '=' in variable declaration")) {
      return;
    }
    Expression();
    parser_.Consume(TokenType::kSemicolon, "expected ';' after variable declaration");
    EmitNameOp(OpCode::kDefineGlobal, name, CurrentLine());
  }

  void Assignment() {
    parser_.Consume(TokenType::kIdentifier, "expected variable name");
    const std::string name = parser_.Previous().lexeme;
    parser_.Consume(TokenType::kEqual, "expected '=' in assignment");
    Expression();
    parser_.Consume(TokenType::kSemicolon, "expected ';' after assignment");
    EmitNameOp(OpCode::kSetGlobal, name, CurrentLine());
  }

  void ImportStatement() {
    const std::string module = parser_.ParseDottedName();
    parser_.Consume(TokenType::kSemicolon, "expected ';' after import statement");
    EmitNameOp(OpCode::kImportModule, module, CurrentLine());
  }

  void FromImportStatement() {
    const std::string module = parser_.ParseDottedName();
    parser_.Consume(TokenType::kImport, "expected 'import' keyword");
    parser_.Consume(TokenType::kIdentifier, "expected imported symbol name");
    const std::string symbol = parser_.Previous().lexeme;
    std::string alias = symbol;
    if (parser_.Match(TokenType::kAs)) {
      parser_.Consume(TokenType::kIdentifier, "expected alias name after 'as'");
      alias = parser_.Previous().lexeme;
    }
    parser_.Consume(TokenType::kSemicolon, "expected ';' after from-import statement");

    const std::size_t line = CurrentLine();
    EmitOp(OpCode::kImportSymbol, line);
    EmitByte(static_cast<std::uint8_t>(AddConstant(module)), line);
    EmitByte(static_cast<std::uint8_t>(AddConstant(symbol)), line);
    EmitByte(static_cast<std::uint8_t>(AddConstant(alias)), line);
  }

  void EmitNameOp(const OpCode op, const std::string& name, const std::size_t line) {
    EmitOp(op, line);
    EmitByte(static_cast<std::uint8_t>(AddConstant(name)), line);
  }

  void Expression() { Term(); }

  void Term() {
    Factor();
    while (parser_.Match(TokenType::kPlus) || parser_.Match(TokenType::kMinus)) {
      const TokenType op = parser_.Previous().type;
      Factor();
      EmitOp(op == TokenType::kPlus ? OpCode::kAdd : OpCode::kSubtract, CurrentLine());
    }
  }

  void Factor() {
    Unary();
    while (parser_.Match(TokenType::kStar) || parser_.Match(TokenType::kSlash)) {
      const TokenType op = parser_.Previous().type;
      Unary();
      EmitOp(op == TokenType::kStar ? OpCode::kMultiply : OpCode::kDivide, CurrentLine());
    }
  }

  void Unary() {
    if (parser_.Match(TokenType::kMinus)) {
      Unary();
      EmitOp(OpCode::kNegate, CurrentLine());
      return;
    }
    Primary();
  }

  void Primary() {
    if (parser_.Match(TokenType::kNumber)) {
      EmitConstant(std::strtod(parser_.Previous().lexeme.c_str(), nullptr), CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kString)) {
      EmitConstant(parser_.Previous().lexeme, CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kTrue)) {
      EmitConstant(true, CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kFalse)) {
      EmitConstant(false, CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kNil)) {
      EmitConstant(std::monostate{}, CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kIdentifier)) {
      EmitNameOp(OpCode::kGetGlobal, parser_.Previous().lexeme, CurrentLine());
      return;
    }
    if (parser_.Match(TokenType::kLeftParen)) {
      Expression();
      parser_.Consume(TokenType::kRightParen, "expected ')' after expression");
      return;
    }
    parser_.Consume(TokenType::kIdentifier, "expected expression");
    if (!parser_.IsAtEnd()) {
      parser_.Advance();
    }
  }

  Parser parser_;
  CompileResult result_;
};

}  // namespace

CompileResult CompileToChunk(const std::string& source) {
  Lexer lexer(source);
  Parser parser(lexer.ScanAllTokens());
  CompilerImpl compiler(std::move(parser));
  return compiler.Compile();
}

}  // namespace ms
