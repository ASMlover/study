#pragma once

#include <Core/MemoryEvolve.hh>

namespace _mevo::tadpole {

enum class TokenKind {
#undef TOKDEF
#define TOKDEF(K, S) K,
#include <Tadpole/KindsDef.hh>
#undef TOKEN

  NUM_KINDS,
};

const char* get_token_name(TokenKind kind);
TokenKind get_keyword_kind(const str_t& key);

class Token final : public Copyable {
  TokenKind kind_{TokenKind::TK_ERR};
  str_t literal_{};
  int lineno_{};
public:
  Token() noexcept {}

  Token(TokenKind kind, const str_t& literal, int lineno) noexcept
    : kind_(kind), literal_(literal), lineno_(lineno) {
  }

  Token(const Token& r) noexcept
    : kind_(r.kind_), literal_(r.literal_), lineno_(r.lineno_) {
  }

  Token(Token&& r) noexcept
    : kind_(std::move(r.kind_))
    , literal_(std::move(r.literal_))
    , lineno_(std::move(r.lineno_)) {
  }

  Token& operator=(const Token& r) noexcept {
    if (this != &r) {
      kind_ = r.kind_;
      literal_ = r.literal_;
      lineno_ = r.lineno_;
    }
    return *this;
  }

  Token& operator=(Token&& r) noexcept {
    if (this != &r) {
      kind_ = std::move(r.kind_);
      literal_ = std::move(r.literal_);
      lineno_ = std::move(r.lineno_);
    }
    return *this;
  }

  inline bool operator==(const Token& r) const noexcept {
    return literal_ == r.literal_;
  }

  inline bool operator!=(const Token& r) const noexcept {
    return literal_ != r.literal_;
  }

  inline TokenKind kind() const noexcept { return kind_; }
  inline const str_t& literal() const noexcept { return literal_; }
  inline int line() const noexcept { return lineno_; }
  inline double as_numeric() const noexcept { return std::atof(literal_.c_str()); }
  inline const str_t& as_string() const noexcept { return literal_; }

  static Token make(const str_t& literal) {
    return Token(TokenKind::TK_STRING, literal, 0);
  }
};

}