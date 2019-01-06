// Copyright (c) 2018 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <unordered_map>
#include "token_kinds.h"

namespace lox {

static constexpr char* const kTokenNames[] = {
#define TOKDEF(k, s) s,
#include "kinds_def.h"
#undef TOKDEF
  nullptr
};

static const std::unordered_map<std::string, TokenKind> kTokenKeywords = {
#define KEYWORD(k, s) {s, TokenKind::KW_##k},
#include "kinds_def.h"
#undef KEYWORD
};

const char* get_token_name(TokenKind kind) {
  if (kind < TokenKind::NUM_TOKENS)
    return kTokenNames[kind];
  return nullptr;
}

const char* get_keyword_spelling(TokenKind kind) {
  switch (kind) {
#define KEYWORD(k, s) case TokenKind::KW_##k: return s;
#include "kinds_def.h"
#undef KEYWORD
  default: break;
  }
  return nullptr;
}

const char* get_punctuator_spelling(TokenKind kind) {
  switch (kind) {
#define PUNCTUATOR(k, s) case TokenKind::TK_##k: return s;
#include "kinds_def.h"
#undef PUNCTUATOR
  default: break;
  }
  return nullptr;
}

TokenKind get_keyword_kind(const char* k) {
  auto kind = kTokenKeywords.find(k);
  if (kind != kTokenKeywords.end())
    return kind->second;
  return TokenKind::TK_UNKNOWN;
}

bool contains_in_keywords(TokenKind kind) {
  return TokenKind::KW_AND <= kind && kind <= TokenKind::KW_PRINT;
}

}
