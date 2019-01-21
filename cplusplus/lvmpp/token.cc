// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include "token.h"

namespace lox {

static constexpr const char* const kTokenNames[] = {
#define TOKDEF(k, s) s,
#include "kind_defs.h"
#undef TOKDEF
  nullptr
};

static const std::unordered_map<std::string, TokenKind> kTokenKeywords = {
#define KEYWORD(k, s) {s, TokenKind::KW_##k},
#include "kind_defs.h"
#undef KEYWORD
};

const char* get_token_name(TokenKind kind) {
  if (kind < TokenKind::NUM_TOKENS)
    return kTokenNames[kind];
  return nullptr;
}

bool contains_in_keywords(TokenKind kind) {
  return kind >= TokenKind::KW_AND && kind <= TokenKind::KW_WHILE;
}

TokenKind get_keyword_kind(const char* key) {
  auto kind_iter = kTokenKeywords.find(key);
  if (kind_iter != kTokenKeywords.end())
    return kind_iter->second;
  return TokenKind::TK_IDENTIFILER;
}

std::string Token::stringify(void) const {
  std::stringstream ss;

  ss << std::left << std::setw(20) << get_token_name(kind_) << ":"
    << std::right << std::setw(24) << literal_ << "|"
    << std::right << std::setw(4) << lineno_;
  return ss.str();
}

double Token::as_numeric(void) const {
  return std::atof(literal_.c_str());
}

std::string Token::as_string(void) const {
  return literal_;
}

std::ostream& operator<<(std::ostream& out, const Token& tok) {
  return out << tok.stringify();
}

}
