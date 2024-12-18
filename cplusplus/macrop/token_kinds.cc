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
#include <string>
#include <unordered_map>
#include "token_kinds.h"

static const char* const kTokenNames[] = {
#define TOKEN(K, S) S,
#define PPKEYWORD(K, S) S,
#include "token_defs.h"
  nullptr
};

const char* get_token_name(TokenKind kind) {
  if (kind < TokenKind::NUM_TOKENS)
    return kTokenNames[kind];
  return nullptr;
}

const char* get_keyword_spelling(TokenKind kind) {
  switch (kind) {
#define KEYWORD(K, S) case KW_##K: return S;
#include "token_defs.h"
  default: break;
  }
  return nullptr;
}

const char* get_ppkeyword_spelling(TokenKind kind) {
  switch (kind) {
#define PPKEYWORD(K, S) case PP_##K: return S;
#include "token_defs.h"
  default: break;
  }
  return nullptr;
}

const char* get_punctuator_spelling(TokenKind kind) {
  switch (kind) {
#define PUNCTUATOR(K, S) case K: return S;
#include "token_defs.h"
  default: break;
  }
  return nullptr;
}

TokenKind get_keyword_kind(const char* k) {
  static std::unordered_map<std::string, TokenKind> kKeywords = {
#define KEYWORD(K, S) {S, KW_##K},
#include "token_defs.h"
  };

  auto find = kKeywords.find(k);
  if (find != kKeywords.end())
    return find->second;
  return TokenKind::IDENTIFILER;
}

TokenKind get_ppkeyword_kind(const char* k) {
  static std::unordered_map<std::string, TokenKind> kPPKeywords = {
#define PPKEYWORD(K, S) {S, PP_##K},
#include "token_defs.h"
  };

  auto find = kPPKeywords.find(k);
  if (find != kPPKeywords.end())
    return find->second;
  return TokenKind::UNKNOWN;
}

bool is_valid_ppkeyworkd(TokenKind kind) {
  return kind >= TokenKind::PP_IF && kind <= TokenKind::PP_PRAGMA;
}
