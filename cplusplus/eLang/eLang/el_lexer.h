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
#ifndef __EL_LEXER_HEADER_H__
#define __EL_LEXER_HEADER_H__

#include "el_lexer_base.h"

namespace el {

interface ReaderBase;
class Lexer : public LexerBase, private UnCopyable {
  ReaderBase& reader_;
  bool   needs_line_;
  int    pos_;
  int    start_;
  String line_;
public:
  explicit Lexer(ReaderBase& reader)
    : reader_(reader)
    , needs_line_(true)
    , pos_(0)
    , start_(0)
    , line_() {
  }

  virtual bool IsInfinite(void) const override;
  virtual Ref<Token> ReadToken(void) override;
private:
  bool IsDone(void) const;
  char Peek(int ahead = 0) const;
  char Advance(void);
  void AdvanceLine(void);
  void SkipBlockComment(void);

  bool IsWhitespace(char c) const;
  bool IsAlpha(char c) const;
  bool IsDigit(char c) const;
  bool IsOperator(char c) const;

  Ref<Token> SingleToken(TokenType type);
  Ref<Token> ReadString(void);
  Ref<Token> ReadNumber(void);
  Ref<Token> ReadName(void);
  Ref<Token> ReadOperator(void);
};

}

#endif  // __EL_LEXER_HEADER_H__
