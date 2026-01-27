// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include "Token.hh"

namespace ms {

static constexpr cstr_t kNames[] = {
#undef TOKDEF
#define TOKDEF(k, s) s,
#include "TokenTypes.hh"
#undef TOKDEF

  nullptr,
};

static const std::unordered_map<str_t, TokenType> kKeywords = {
#undef KEYWORD
#define KEYWORD(k, s) {s, TokenType::KW_##k},
#include "TokenTypes.hh"
#undef KEYWORD
};

cstr_t get_type_name(TokenType type) noexcept {
  if (type > TokenType::TOKENTYPE_BEG && type < TokenType::TOKENTYPE_END)
    return kNames[as_type<int>(type)];
  return nullptr;
}

TokenType get_keyword_type(const str_t& keyword) noexcept {
  if (auto it = kKeywords.find(keyword); it != kKeywords.end())
    return it->second;
  return TokenType::TK_IDENTIFIER;
}

str_t Token::stringify() const {
  ss_t ss;

  ss << std::left << std::setw(20) << get_type_name(type_) << ":"
    << std::right << std::setw(24) << literal_ << "|"
    << std::right << std::setw(04) << lineno_;
  return ss.str();
}

}
