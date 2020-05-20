#include <unordered_map>
#include "token.hh"

namespace tadpole {

static constexpr const char* kNames[] = {
#undef TOKDEF
#define TOKDEF(k, s) s,
#include "kinds_def.hh"
#undef TOKDEF

  nullptr
};

static const std::unordered_map<str_t, TokenKind> kKWs = {
#undef KEYWORD
#define KEYWORD(k, s) {s, TokenKind::KW_##k},
#include "kinds_def.hh"
#undef KEYWORD
};

const char* get_token_name(TokenKind kind) {
  if (kind >= TokenKind::TK_LPAREN && kind < TokenKind::NUM_KINDS)
    return kNames[as_type<int>(kind)];
  return nullptr;
}

TokenKind get_keyword_kind(const str_t& key) {
  if (auto it = kKWs.find(key); it != kKWs.end())
    return it->second;
  return TokenKind::TK_IDENTIFIER;
}

}