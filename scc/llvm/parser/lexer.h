// Copyright (c) 2016 ASMlover. All rights reserved.
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

#include <memory>
#include <string>
#include "uncopyable.h"

struct Token {
  enum class Type {
    TYPE_EOF = -1,

    TYPE_DEF = -2,
    TYPE_EXT = -3,

    TYPE_ID  = -4,
    TYPE_NUM = -5,
  };

  Type        type;
  std::string name;

  struct Position {
    std::string fname;
    int         lineno;
    int         colum;
  } position;
};

class Lexer : private UnCopyable {
  enum State {
    STATE_BEGIN = 0,
    STATE_FINISH,

    STATE_REAL,     // real number
    STATE_STR,      // string
    STATE_ID,       // identifier
    STATE_ASSIGN,   // =
    STATE_COMMENT,  // #
  };
  enum { BSIZE = 128 };
  typedef Token::Type (Lexer::*LexerFunction)(int, State&, bool&);

  std::unique_ptr<FILE> stream_;
  std::string           fname_;
  int                   lineno_;
  int                   colum_;
  int                   bsize_;
  int                   lexpos_;
  bool                  eof_;
  char                  lexbuf_[BSIZE];
public:
  Lexer(void);
  ~Lexer(void);

  bool Open(const std::string& fname);
  void Close(void);
  Token::Type GetToken(Token& token);
private:
  int GetChar(void);
  coid UngetChar(void);

  Token::Type LexerBegin(int c, State& out_state, bool& out_save);
  Token::Type LexerFinish(int c, State& out_state, bool& out_save);
  Token::Type LexerReal(int c, State& out_state, bool& out_save);
  Token::Type LexerStr(int c, State& out_state, bool& out_save);
  Token::Type LexerID(int c, State& out_state, bool& out_save);
  Token::Type LexerAssign(int c, State& out_state, bool& out_save);
  Token::Type LexerComment(int c, State& out_state, bool& out_save);
};

#endif  // __LEXER_HEADER_H__
