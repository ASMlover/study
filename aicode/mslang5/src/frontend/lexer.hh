#pragma once

#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

class Lexer {
 public:
  explicit Lexer(std::string source, bool emit_newline_tokens = false);

  std::vector<Token> scan_all_tokens();

 private:
  char advance();
  bool match(char expected);
  char peek() const;
  char peek_next() const;
  bool is_at_end() const;

  void skip_horizontal_whitespace_and_comments();
  Token make_token(TokenType type) const;
  Token error_token(const std::string& message) const;
  Token string_token();
  Token number_token();
  Token identifier_token();
  TokenType identifier_type(const std::string& text) const;

  std::string source_;
  std::size_t start_ = 0;
  std::size_t current_ = 0;
  std::size_t line_ = 1;
  bool emit_newline_tokens_ = false;
};

}  // namespace ms

