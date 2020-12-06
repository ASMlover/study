#pragma once

#include "common.hh"

namespace tadpole {

enum class TokenKind {
#undef TOKDEF
#define TOKDEF(k, s) k,
#include "kinds_def.hh"
#undef TOKDEF

  NUM_KINDS,
};

inline bool operator==(TokenKind k1, TokenKind k2) noexcept {
  return as_type<int>(k1) == as_type<int>(k2);
}

inline bool operator!=(TokenKind k1, TokenKind k2) noexcept {
  return !(k1 == k2);
}

inline bool operator<(TokenKind k1, TokenKind k2) noexcept {
  return as_type<int>(k1) < as_type<int>(k2);
}

inline bool operator<=(TokenKind k1, TokenKind k2) noexcept {
  return (k1 < k2) || (k1 == k2);
}

inline bool operator>(TokenKind k1, TokenKind k2) noexcept {
  return !(k1 <= k2);
}

inline bool operator>=(TokenKind k1, TokenKind k2) noexcept {
  return !(k1 < k2);
}

}