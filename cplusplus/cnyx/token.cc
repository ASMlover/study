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
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include "token.hh"

namespace nyx {

static constexpr const char* kNames[] = {
#undef TOKDEF
#define TOKDEF(k, s) s,
#include "kinds_def.hh"
#undef TOKDEF
  nullptr
};

static const std::unordered_map<std::string, TokenKind> kKeywords = {
#undef KEYWORD
#define KEYWORD(k, s) {s, TokenKind::KW_##k},
#include "kinds_def.hh"
#undef KEYWORD
};

const char* get_token_name(TokenKind kind) {
  if (kind >= TokenKind::TK_ERROR && kind < TokenKind::NUM_KINDS)
    return kNames[EnumUtil<TokenKind>::as_int(kind)];
  return nullptr;
}

TokenKind get_keyword_kind(const char* key) {
  auto kind_iter = kKeywords.find(key);
  if (kind_iter != kKeywords.end())
    return kind_iter->second;
  return TokenKind::TK_IDENTIFIER;
}

std::string Token::stringify(void) const {
  std::stringstream ss;

  ss << std::left << std::setw(20) << get_token_name(kind_) << ":"
    << std::right << std::setw(24) << literal_ << "|"
    << std::right << std::setw(4) << lineno_;
  return ss.str();
}

std::ostream& operator<<(std::ostream& out, const Token& tok) {
  return out << tok.stringify();
}

}
