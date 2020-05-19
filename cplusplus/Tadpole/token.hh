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

const char* get_token_name(TokenKind kind);
TokenKind get_keyword_kind(const str_t& key);

}