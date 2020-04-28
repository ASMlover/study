#pragma once

#include <cctype>
#include <Tadpole/Token.hh>

namespace _mevo::tadpole {

class Lexer final : private UnCopyable {
  const str_t& source_bytes_;
  sz_t begpos_{};
  sz_t curpos_{};
  int lineno_{1};

  inline bool is_alpha(char c) const noexcept { return std::isalpha(c) || c == '_'; }
  inline bool is_alnum(char c) const noexcept { return std::isalnum(c) || c == '_'; }
  inline bool is_digit(char c) const noexcept { return std::isdigit(c); }
  inline bool is_end() const noexcept { return curpos_ >= source_bytes_.size(); }
  inline char advance() noexcept { return source_bytes_[curpos_++]; }

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const noexcept {
    return source_bytes_.substr(begpos, endpos - begpos);
  }

  inline char peek(int distance = 0) const noexcept {
    return curpos_ + distance >= source_bytes_.size()
      ? 0 : source_bytes_[curpos_ + distance];
  }

  inline bool match(char expected) noexcept {
    if (source_bytes_[curpos_] == expected) {
      advance();
      return true;
    }
    return false;
  }

  void skip_whitespace();
  Token make_token(TokenKind kind, const str_t& literal);
  Token make_token(TokenKind kind);
  Token make_error(const str_t& err_msg);
  Token make_identifier();
  Token make_numeric();
  Token make_string();
public:
  Lexer(const str_t& source_bytes) noexcept
    : source_bytes_(source_bytes) {
  }

  Token next_token();
};

}