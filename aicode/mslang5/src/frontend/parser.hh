#pragma once

#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

class Parser {
 public:
  explicit Parser(std::vector<Token> tokens);

  bool Match(TokenType type);
  bool Check(TokenType type) const;
  const Token& Advance();
  const Token& Previous() const;
  bool IsAtEnd() const;

  bool Consume(TokenType type, const std::string& message);
  std::string ParseDottedName();
  const Token& Current() const;
  const Token& Peek(std::size_t offset) const;

  const std::vector<std::string>& Errors() const;

 private:
  void ReportError(const Token& token, const std::string& message);

  std::vector<Token> tokens_;
  std::size_t current_ = 0;
  std::vector<std::string> errors_;
};

}  // namespace ms
