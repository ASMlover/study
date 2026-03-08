#include "frontend/lexer.hh"

#include <cctype>
#include <unordered_map>

namespace ms {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::ScanAllTokens() {
  std::vector<Token> tokens;
  while (!IsAtEnd()) {
    SkipWhitespaceAndComments();
    start_ = current_;
    if (IsAtEnd()) {
      break;
    }
    const char c = Advance();
    switch (c) {
      case '(':
        tokens.push_back(MakeToken(TokenType::kLeftParen));
        break;
      case ')':
        tokens.push_back(MakeToken(TokenType::kRightParen));
        break;
      case '{':
        tokens.push_back(MakeToken(TokenType::kLeftBrace));
        break;
      case '}':
        tokens.push_back(MakeToken(TokenType::kRightBrace));
        break;
      case ',':
        tokens.push_back(MakeToken(TokenType::kComma));
        break;
      case '<':
        tokens.push_back(MakeToken(TokenType::kLess));
        break;
      case '+':
        tokens.push_back(MakeToken(TokenType::kPlus));
        break;
      case '-':
        tokens.push_back(MakeToken(TokenType::kMinus));
        break;
      case '*':
        tokens.push_back(MakeToken(TokenType::kStar));
        break;
      case '/':
        tokens.push_back(MakeToken(TokenType::kSlash));
        break;
      case ';':
        tokens.push_back(MakeToken(TokenType::kSemicolon));
        break;
      case '.':
        tokens.push_back(MakeToken(TokenType::kDot));
        break;
      case '=':
        tokens.push_back(MakeToken(TokenType::kEqual));
        break;
      case '"':
        tokens.push_back(StringToken());
        break;
      default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
          tokens.push_back(NumberToken());
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
          tokens.push_back(IdentifierToken());
        } else {
          tokens.push_back(ErrorToken("unexpected character"));
        }
        break;
    }
  }
  tokens.push_back(Token{TokenType::kEof, "", line_});
  return tokens;
}

char Lexer::Advance() { return source_[current_++]; }

bool Lexer::Match(const char expected) {
  if (IsAtEnd() || source_[current_] != expected) {
    return false;
  }
  ++current_;
  return true;
}

char Lexer::Peek() const {
  if (IsAtEnd()) {
    return '\0';
  }
  return source_[current_];
}

char Lexer::PeekNext() const {
  if (current_ + 1 >= source_.size()) {
    return '\0';
  }
  return source_[current_ + 1];
}

bool Lexer::IsAtEnd() const { return current_ >= source_.size(); }

void Lexer::SkipWhitespaceAndComments() {
  while (!IsAtEnd()) {
    const char c = Peek();
    if (c == ' ' || c == '\r' || c == '\t') {
      Advance();
      continue;
    }
    if (c == '\n') {
      ++line_;
      Advance();
      continue;
    }
    if (c == '/' && PeekNext() == '/') {
      while (!IsAtEnd() && Peek() != '\n') {
        Advance();
      }
      continue;
    }
    break;
  }
}

Token Lexer::MakeToken(const TokenType type) const {
  return Token{type, source_.substr(start_, current_ - start_), line_};
}

Token Lexer::ErrorToken(const std::string& message) const {
  return Token{TokenType::kError, message, line_};
}

Token Lexer::StringToken() {
  while (!IsAtEnd() && Peek() != '"') {
    if (Peek() == '\n') {
      ++line_;
    }
    Advance();
  }
  if (IsAtEnd()) {
    return ErrorToken("unterminated string");
  }
  Advance();
  const std::string quoted = source_.substr(start_ + 1, current_ - start_ - 2);
  return Token{TokenType::kString, quoted, line_};
}

Token Lexer::NumberToken() {
  while (std::isdigit(static_cast<unsigned char>(Peek()))) {
    Advance();
  }
  if (Peek() == '.' && std::isdigit(static_cast<unsigned char>(PeekNext()))) {
    Advance();
    while (std::isdigit(static_cast<unsigned char>(Peek()))) {
      Advance();
    }
  }
  return MakeToken(TokenType::kNumber);
}

Token Lexer::IdentifierToken() {
  while (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_') {
    Advance();
  }
  const std::string text = source_.substr(start_, current_ - start_);
  return Token{IdentifierType(text), text, line_};
}

TokenType Lexer::IdentifierType(const std::string& text) const {
  static const std::unordered_map<std::string, TokenType> keywords = {
      {"print", TokenType::kPrint},   {"var", TokenType::kVar},
      {"import", TokenType::kImport}, {"from", TokenType::kFrom},
      {"as", TokenType::kAs},         {"true", TokenType::kTrue},
      {"false", TokenType::kFalse},   {"nil", TokenType::kNil},
      {"fun", TokenType::kFun},       {"return", TokenType::kReturn},
      {"class", TokenType::kClass},   {"this", TokenType::kThis},
      {"super", TokenType::kSuper},
  };
  const auto it = keywords.find(text);
  if (it == keywords.end()) {
    return TokenType::kIdentifier;
  }
  return it->second;
}

}  // namespace ms
