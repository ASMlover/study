#include "frontend/parser.hh"

#include <sstream>
#include <utility>

namespace ms {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

bool Parser::match(const TokenType type) {
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}

bool Parser::check(const TokenType type) const {
  if (is_at_end()) {
    return type == TokenType::kEof;
  }
  return tokens_[current_].type == type;
}

const Token& Parser::advance() {
  if (!is_at_end()) {
    ++current_;
  }
  return previous();
}

const Token& Parser::previous() const { return tokens_[current_ - 1]; }

bool Parser::is_at_end() const { return tokens_[current_].type == TokenType::kEof; }

bool Parser::consume(const TokenType type, const std::string& message) {
  if (check(type)) {
    advance();
    return true;
  }
  report_error(tokens_[current_], message);
  synchronize();
  return false;
}

bool Parser::consume_statement_end(const std::string& message) {
  if (match(TokenType::kSemicolon)) {
    skip_newline_tokens();
    return true;
  }
  if (match(TokenType::kNewline)) {
    skip_newline_tokens();
    return true;
  }
  if (check(TokenType::kRightBrace) || check(TokenType::kEof)) {
    return true;
  }
  report_error(tokens_[current_], message);
  synchronize();
  return false;
}

void Parser::skip_newline_tokens() {
  while (match(TokenType::kNewline)) {
  }
}

std::string Parser::parse_dotted_name() {
  if (!consume(TokenType::kIdentifier, "expected identifier")) {
    return {};
  }
  std::string name = previous().lexeme;
  while (match(TokenType::kDot)) {
    if (!consume(TokenType::kIdentifier, "expected identifier after '.'")) {
      return {};
    }
    name += ".";
    name += previous().lexeme;
  }
  return name;
}

const Token& Parser::current() const { return tokens_[current_]; }

const Token& Parser::peek(const std::size_t offset) const {
  const std::size_t idx = current_ + offset;
  if (idx >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_[idx];
}

const std::vector<std::string>& Parser::errors() const { return errors_; }

void Parser::synchronize() {
  while (!is_at_end()) {
    if (current_ > 0) {
      const TokenType prev = previous().type;
      if (prev == TokenType::kSemicolon || prev == TokenType::kNewline) {
        panic_mode_ = false;
        skip_newline_tokens();
        return;
      }
    }

    switch (current().type) {
      case TokenType::kClass:
      case TokenType::kFun:
      case TokenType::kVar:
      case TokenType::kFor:
      case TokenType::kIf:
      case TokenType::kWhile:
      case TokenType::kPrint:
      case TokenType::kReturn:
      case TokenType::kImport:
      case TokenType::kFrom:
        panic_mode_ = false;
        return;
      case TokenType::kRightBrace:
        // Consume block boundary to guarantee forward progress in recovery mode.
        advance();
        panic_mode_ = false;
        return;
      case TokenType::kEof:
        panic_mode_ = false;
        return;
      default:
        break;
    }

    advance();
  }

  panic_mode_ = false;
}

void Parser::report_error(const Token& token, const std::string& message) {
  if (panic_mode_) {
    return;
  }
  panic_mode_ = true;

  if (errors_.size() >= kMaxErrorCascadeCount) {
    errors_truncated_ = true;
    return;
  }

  std::ostringstream out;
  out << "[line " << token.line << "] parse error: " << message;
  errors_.push_back(out.str());
}

}  // namespace ms