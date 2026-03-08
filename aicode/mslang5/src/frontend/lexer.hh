#pragma once

#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

class Lexer {
 public:
  explicit Lexer(std::string source);

  std::vector<Token> ScanAllTokens();

 private:
  char Advance();
  bool Match(char expected);
  char Peek() const;
  char PeekNext() const;
  bool IsAtEnd() const;

  void SkipWhitespaceAndComments();
  Token MakeToken(TokenType type) const;
  Token ErrorToken(const std::string& message) const;
  Token StringToken();
  Token NumberToken();
  Token IdentifierToken();
  TokenType IdentifierType(const std::string& text) const;

  std::string source_;
  std::size_t start_ = 0;
  std::size_t current_ = 0;
  std::size_t line_ = 1;
};

}  // namespace ms

