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
#ifndef __EL_PARSER_BASE_HEADER_H__
#define __EL_PARSER_BASE_HEADER_H__

#include "el_queue.h"
#include "el_token.h"

namespace el {

interface ErrorReporterBase;
interface LexerBase;
interface ReaderBase;

class ParserBase : private UnCopyable {
  ErrorReporterBase&    err_reporter_;
  LexerBase&            lexer_;
  bool                  had_error_;
  Queue<Ref<Token>, 3>  read_;
public:
  ParserBase(ErrorReporterBase& error, LexerBase& lexer)
    : err_reporter_(error)
    , lexer_(lexer)
    , had_error_(false) {
  }

  virtual ~ParserBase(void) {}

  inline bool IsInfinite(void) const {
    return lexer_.IsInfinite();
  }

  inline const Token& Current(void) const {
    return *read_[0];
  }

  bool LookAhead(TokenType type);
  bool LookAhead(TokenType current, TokenType next);
  bool LookAhead(TokenType first, TokenType second, TokenType thrid);
  bool Match(TokenType type);
  void Expect(TokenType expected, const char* exception);
  Ref<Token> Consume(void);
  Ref<Token> Consume(TokenType expected, const char* exception);
  void Error(const char* exception);

  inline bool HadError(void) const {
    return had_error_;
  }
private:
  void FillLookAhead(int count);
};

}

#endif  // __EL_PARSER_BASE_HEADER_H__
