// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TOKEN_HEADER_H__
#define __TOKEN_HEADER_H__

#include <string>

enum class TokenType {
  TT_LEFT_PAREN,    // (
  TT_RIGHT_PAREN,   // )
  TT_LEFT_BRACKET,  // [
  TT_RIGHT_BRACKET, // ]
  TT_LEFT_BRACE,    // {
  TT_RIGHT_BRACE,   // }
  TT_COMMA,         // ,
  TT_DOT,           // .
  TT_EQUALS,        // =
  TT_NAME,
  TT_OPERATOR,
  TT_KEYWORD,
  TT_ELSE,          // else
  TT_FN,            // fn
  TT_IF,            // if
  TT_VAR,           // var
  TT_STRING,
  TT_LINE,
  TT_EOF,
};

class Token {
  TokenType   mType;
  std::string mStringValue;

  Token(const Token&) = delete;
  Token& operator=(const Token&) = delete;
public:
  explicit Token(TokenType type)
    : mType(type)
    , mStringValue("") {
  }

  Token(TokenType type, const std::string& value)
    : mType(type)
    , mStringValue(value) {
  }

  TokenType getType(void) const {
    return mType;
  }

  std::string getString(void) const {
    return mStringValue;
  }

  std::string toString(void) {
    switch (mType) {
    case TokenType::TT_LEFT_PAREN: return "(";
    case TokenType::TT_RIGHT_PAREN: return ")";
    case TokenType::TT_LEFT_BRACKET: return "[";
    case TokenType::TT_RIGHT_BRACKET: return "]";
    case TokenType::TT_LEFT_BRACE: return "{";
    case TokenType::TT_RIGHT_BRACE: return "}";
    case TokenType::TT_COMMA: return ",";
    case TokenType::TT_DOT: return ".";
    case TokenType::TT_EQUALS: return "=";
    case TokenType::TT_NAME: return mStringValue + "(name)";
    case TokenType::TT_OPERATOR: return mStringValue + "(op)";
    case TokenType::TT_KEYWORD: return mStringValue + "(keyword)";
    case TokenType::TT_ELSE: return "else";
    case TokenType::TT_FN: return "fn";
    case TokenType::TT_IF: return "if";
    case TokenType::TT_VAR: return "var";
    case TokenType::TT_STRING: return "\"" + mStringValue + "\"";
    case TokenType::TT_LINE: return "(line)";
    case TokenType::TT_EOF: return "(eof)";
    default: return "(unknown token?!)";
    }
  }
};

#endif  // __TOKEN_HEADER_H__
