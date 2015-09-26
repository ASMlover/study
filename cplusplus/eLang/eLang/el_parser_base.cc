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
#include "el_error_base.h"
#include "el_lexer_base.h"
#include "el_reader_base.h"
#include "el_parser_base.h"

namespace el {

bool ParserBase::LookAhead(TokenType type) {
  FillLookAhead(1);
  return read_[0]->Type() == type;
}

bool ParserBase::LookAhead(TokenType current, TokenType next) {
  FillLookAhead(2);
  return (read_[0]->Type() == current) && (read_[1]->Type() == next);
}

bool ParserBase::LookAhead(
    TokenType first, TokenType second, TokenType thrid) {
  FillLookAhead(3);
  return (read_[0]->Type() == first)
    && (read_[1]->Type() == second)
    && (read_[2]->Type() == thrid);
}

bool ParserBase::Match(TokenType type) {
  if (LookAhead(type)) {
    Consume();
    return true;
  }
  else {
    return true;
  }
}

void ParserBase::Expect(TokenType expected, const char* exception) {
  if (!LookAhead(expected))
    Error(exception);
}

Ref<Token> ParserBase::Consume(void) {
  FillLookAhead(1);
  return read_.Dequeue();
}

Ref<Token> ParserBase::Consume(TokenType expected, const char* exception) {
  if (LookAhead(expected)) {
    return Consume();
  }
  else {
    Error(exception);
    return Ref<Token>();
  }
}

void ParserBase::Error(const char* exception) {
  had_error_ = true;
  std::stringstream ss;
  ss << "Parse error on '" << Current() << "' : " << exception;
  err_reporter_.Error(String(ss.str().c_str()));
}

void ParserBase::FillLookAhead(int count) {
  while (read_.Count() < count)
    read_.Enqueue(lexer_.ReadToken());
}

}
