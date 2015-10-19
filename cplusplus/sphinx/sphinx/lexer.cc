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
#include <ctype.h>
#include "lexer.h"

Lexer::Lexer(const std::string& text)
  : mText(text)
  , mState(LexState::DEFAULT)
  , mTokenStart(0)
  , mIndex(0)
  , mEatLines(true) {
}

Token Lexer::readToken(void) {
  Token token = readRawToken();

  return token;
}

Token Lexer::readRawToken(void) {
  return Token(TokenType::TT_EOF);
}

Token Lexer::singleCharToken(TokenType type) {
  ++mIndex;
  return Token(type, mText.substr(mIndex - 1, mIndex));
}

void Lexer::startToken(LexState state) {
}

void Lexer::changeToken(LexState state) {
}

Token Lexer::createStringToken(TokenType type) {
  std::string text = mText.substr(mTokenStart, mIndex);
  return Token(type, text);
}

bool Lexer::isOperator(char c) {
  return nullptr != strchr("`~!@#$%^&*-=+\\|/?<>", c);
}
