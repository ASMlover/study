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
#ifndef __LEXER_HEADER_H__
#define __LEXER_HEADER_H__

#include <string>
#include "token.h"

class Lexer {
  enum class LexState {
    DEFAULT,
    IN_NAME,
    IN_OPERATOR,
    IN_KEYWORD,
    IN_STRING,
    IN_COMMENT,
  };

  std::string mText;
  LexState    mState;
  int         mTokenStart;
  int         mIndex;
  bool        mEatLines;

  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;
public:
  explicit Lexer(const std::string& text);
  ~Lexer(void) = default;
  Token readToken(void);
private:
  Token readRawToken(void);
  Token singleCharToken(TokenType type);
  void startToken(LexState state);
  void changeToken(LexState state);
  Token createStringToken(TokenType type);
  bool isOperator(char c);
};

#endif  // __LEXER_HEADER_H__
