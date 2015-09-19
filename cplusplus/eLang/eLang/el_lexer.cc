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
#include "el_base.h"
#include "el_token.h"
#include "el_reader_base.h"
#include "el_lexer.h"

namespace el {

bool Lexer::IsInfinite(void) const {
  return true;
}

Ref<Token> Lexer::ReadToken(void) {
  return Ref<Token>(new Token(TokenType::TOKEN_ERROR));
}

bool Lexer::IsDone(void) const {
  return true;
}

char Lexer::Peek(int ahead) const {
  return 0;
}

char Lexer::Advance(void) {
  return 0;
}

void Lexer::AdvanceLine(void) {
}

void Lexer::SkipBlockComment(void) {
}

bool Lexer::IsWhitespace(char c) const {
  return true;
}

bool Lexer::IsAlpha(char c) const {
  return true;
}

bool Lexer::IsDigit(char c) const {
  return true;
}

bool Lexer::IsOperator(char c) const {
  return true;
}

Ref<Token> Lexer::SingleToken(TokenType type) {
  return Ref<Token>(new Token(type));
}

Ref<Token> Lexer::ReadString(void) {
  return SingleToken(TokenType::TOKEN_ERROR);
}

Ref<Token> Lexer::ReadNumber(void) {
  return SingleToken(TokenType::TOKEN_ERROR);
}

Ref<Token> Lexer::ReadName(void) {
  return SingleToken(TokenType::TOKEN_ERROR);
}

Ref<Token> Lexer::ReadOperator(void) {
  return SingleToken(TokenType::TOKEN_ERROR);
}

}
