module;
#include <unordered_map>
#include "common.hh"
#include "token.hh"
import common;

export module token;

export namespace tadpole {

constexpr const char* kNames[] = {
#undef TOKDEF
#define TOKDEF(k, s) s,
#include "kinds_def.hh"
#undef TOKDEF

  nullptr
};

const char* get_kind_name(TokenKind kind) {
  if (kind >= TokenKind::TK_LPAREN && kind < TokenKind::NUM_KINDS)
    return kNames[as_type<int>(kind)];
  return nullptr;
}

const std::unordered_map<str_t, TokenKind> kKWs = {
#undef KEYWORD
#define KEYWORD(k, s) {s, TokenKind::KW_##k},
#include "kinds_def.hh"
#undef KEYWORD
};

TokenKind get_keyword_kind(const str_t& key) {
  if (auto it = kKWs.find(key); it != kKWs.end())
    return it->second;
  return TokenKind::TK_IDENTIFIER;
}

std::ostream& operator<<(std::ostream& out, TokenKind kind) noexcept {
  return out << get_kind_name(kind);
}

class Token final : public Copyable {
  TokenKind kind_{ TokenKind::TK_ERR };
  str_t literal_;
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

  inline Token& operator=(const Token& r) noexcept {
    if (this != &r) {
      kind_ = r.kind_;
      literal_ = r.literal_;
      lineno_ = r.lineno_;
    }
    return *this;
  }

  inline Token& operator=(Token&& r) noexcept {
    if (this != &r) {
      kind_ = std::move(r.kind_);
      literal_ = std::move(r.literal_);
      lineno_ = std::move(r.lineno_);
    }
    return *this;
  }

  inline bool operator==(const Token& r) const noexcept {
    return this == &r ? true : literal_ == r.literal_;
  }

  inline bool operator!=(const Token& r) const noexcept {
    return !(*this == r);
  }

  inline TokenKind kind() const noexcept { return kind_; }
  inline const str_t& literal() const noexcept { return literal_; }
  inline int lineno() const noexcept { return lineno_; }
  inline double as_numeric() const noexcept { return std::atof(literal_.c_str()); }
  inline const str_t& as_string() const noexcept { return literal_; }
  inline const char* as_cstring() const noexcept { return literal_.c_str(); }

  static Token make(const str_t& literal) noexcept {
    return Token(TokenKind::TK_STRING, literal, 0);
  }
};

}