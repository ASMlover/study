#include "frontend/lexer.hh"

#include <cctype>
#include <unordered_map>

namespace ms {

Lexer::Lexer(std::string source, const bool emit_newline_tokens)
    : source_(std::move(source)), emit_newline_tokens_(emit_newline_tokens) {}

std::vector<Token> Lexer::scan_all_tokens() {
  std::vector<Token> tokens;
  while (!is_at_end()) {
    skip_horizontal_whitespace_and_comments();
    start_ = current_;
    if (is_at_end()) {
      break;
    }

    const char c = advance();
    if (c == '\n') {
      ++line_;
      if (emit_newline_tokens_) {
        tokens.push_back(Token{TokenType::kNewline, "\\n", line_ - 1});
      }
      continue;
    }

    switch (c) {
      case '(':
        tokens.push_back(make_token(TokenType::kLeftParen));
        break;
      case ')':
        tokens.push_back(make_token(TokenType::kRightParen));
        break;
      case '{':
        tokens.push_back(make_token(TokenType::kLeftBrace));
        break;
      case '}':
        tokens.push_back(make_token(TokenType::kRightBrace));
        break;
      case ',':
        tokens.push_back(make_token(TokenType::kComma));
        break;
      case '!':
        tokens.push_back(make_token(match('=') ? TokenType::kBangEqual : TokenType::kBang));
        break;
      case '<':
        tokens.push_back(make_token(match('=') ? TokenType::kLessEqual : TokenType::kLess));
        break;
      case '>':
        tokens.push_back(make_token(match('=') ? TokenType::kGreaterEqual : TokenType::kGreater));
        break;
      case '+':
        tokens.push_back(make_token(TokenType::kPlus));
        break;
      case '-':
        tokens.push_back(make_token(TokenType::kMinus));
        break;
      case '*':
        tokens.push_back(make_token(TokenType::kStar));
        break;
      case '/':
        tokens.push_back(make_token(TokenType::kSlash));
        break;
      case ';':
        tokens.push_back(make_token(TokenType::kSemicolon));
        break;
      case '.':
        tokens.push_back(make_token(TokenType::kDot));
        break;
      case '=':
        tokens.push_back(make_token(match('=') ? TokenType::kEqualEqual : TokenType::kEqual));
        break;
      case '"':
        tokens.push_back(string_token());
        break;
      default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
          tokens.push_back(number_token());
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
          tokens.push_back(identifier_token());
        } else {
          tokens.push_back(error_token("unexpected character"));
        }
        break;
    }
  }
  tokens.push_back(Token{TokenType::kEof, "", line_});
  return tokens;
}

char Lexer::advance() { return source_[current_++]; }

bool Lexer::match(const char expected) {
  if (is_at_end() || source_[current_] != expected) {
    return false;
  }
  ++current_;
  return true;
}

char Lexer::peek() const {
  if (is_at_end()) {
    return '\0';
  }
  return source_[current_];
}

char Lexer::peek_next() const {
  if (current_ + 1 >= source_.size()) {
    return '\0';
  }
  return source_[current_ + 1];
}

bool Lexer::is_at_end() const { return current_ >= source_.size(); }

void Lexer::skip_horizontal_whitespace_and_comments() {
  while (!is_at_end()) {
    const char c = peek();
    if (c == ' ' || c == '\r' || c == '\t') {
      advance();
      continue;
    }
    if (c == '/' && peek_next() == '/') {
      while (!is_at_end() && peek() != '\n') {
        advance();
      }
      continue;
    }
    break;
  }
}

Token Lexer::make_token(const TokenType type) const {
  return Token{type, source_.substr(start_, current_ - start_), line_};
}

Token Lexer::error_token(const std::string& message) const {
  return Token{TokenType::kError, message, line_};
}

Token Lexer::string_token() {
  while (!is_at_end() && peek() != '"') {
    if (peek() == '\n') {
      ++line_;
    }
    advance();
  }
  if (is_at_end()) {
    return error_token("unterminated string");
  }
  advance();
  const std::string quoted = source_.substr(start_ + 1, current_ - start_ - 2);
  return Token{TokenType::kString, quoted, line_};
}

Token Lexer::number_token() {
  while (std::isdigit(static_cast<unsigned char>(peek()))) {
    advance();
  }
  if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
    advance();
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
      advance();
    }
  }
  return make_token(TokenType::kNumber);
}

Token Lexer::identifier_token() {
  while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
    advance();
  }
  const std::string text = source_.substr(start_, current_ - start_);
  return Token{identifier_type(text), text, line_};
}

TokenType Lexer::identifier_type(const std::string& text) const {
  static const std::unordered_map<std::string, TokenType> keywords = {
      {"print", TokenType::kPrint},   {"var", TokenType::kVar},
      {"import", TokenType::kImport}, {"from", TokenType::kFrom},
      {"as", TokenType::kAs},         {"true", TokenType::kTrue},
      {"false", TokenType::kFalse},   {"nil", TokenType::kNil},
      {"if", TokenType::kIf},         {"else", TokenType::kElse},
      {"while", TokenType::kWhile},   {"for", TokenType::kFor},
      {"and", TokenType::kAnd},       {"or", TokenType::kOr},
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