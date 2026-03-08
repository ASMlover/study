#include "frontend/parser.hh"

#include <sstream>

namespace ms {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

bool Parser::Match(const TokenType type) {
  if (!Check(type)) {
    return false;
  }
  Advance();
  return true;
}

bool Parser::Check(const TokenType type) const {
  if (IsAtEnd()) {
    return type == TokenType::kEof;
  }
  return tokens_[current_].type == type;
}

const Token& Parser::Advance() {
  if (!IsAtEnd()) {
    ++current_;
  }
  return Previous();
}

const Token& Parser::Previous() const { return tokens_[current_ - 1]; }

bool Parser::IsAtEnd() const { return tokens_[current_].type == TokenType::kEof; }

bool Parser::Consume(const TokenType type, const std::string& message) {
  if (Check(type)) {
    Advance();
    return true;
  }
  ReportError(tokens_[current_], message);
  return false;
}

std::string Parser::ParseDottedName() {
  if (!Consume(TokenType::kIdentifier, "expected identifier")) {
    return {};
  }
  std::string name = Previous().lexeme;
  while (Match(TokenType::kDot)) {
    if (!Consume(TokenType::kIdentifier, "expected identifier after '.'")) {
      return {};
    }
    name += ".";
    name += Previous().lexeme;
  }
  return name;
}

const Token& Parser::Current() const { return tokens_[current_]; }

const Token& Parser::Peek(const std::size_t offset) const {
  const std::size_t idx = current_ + offset;
  if (idx >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_[idx];
}

const std::vector<std::string>& Parser::Errors() const { return errors_; }

void Parser::ReportError(const Token& token, const std::string& message) {
  std::ostringstream out;
  out << "[line " << token.line << "] parse error: " << message;
  errors_.push_back(out.str());
}

}  // namespace ms
