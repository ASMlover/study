#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

class Parser {
 public:
  static constexpr std::size_t kMaxErrorCascadeCount = 8;

  explicit Parser(std::vector<Token> tokens);

  bool match(TokenType type);
  bool check(TokenType type) const;
  const Token& advance();
  const Token& previous() const;
  bool is_at_end() const;

  bool consume(TokenType type, const std::string& message);
  bool consume_statement_end(const std::string& message);
  void skip_newline_tokens();
  std::string parse_dotted_name();
  const Token& current() const;
  const Token& peek(std::size_t offset) const;

  const std::vector<std::string>& errors() const;

 private:
  void synchronize();
  void report_error(const Token& token, const std::string& message);

  std::vector<Token> tokens_;
  std::size_t current_ = 0;
  std::vector<std::string> errors_;
  bool panic_mode_ = false;
  bool errors_truncated_ = false;
};

}  // namespace ms