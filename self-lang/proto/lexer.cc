//! Copyright (c) 2014 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include <utility.h>
#include "lexer.h"



std::map<std::string, Token::Type> Lexer::kReserveds;

Lexer::Lexer(void)
  : stream_(static_cast<FILE*>(NULL))
  , fname_("")
  , lineno_(0)
  , bsize_(0)
  , lexpos_(0)
  , eof_(false)
{
  LoadReserveds();
}

Lexer::~Lexer(void)
{
}

bool 
Lexer::Open(const char* fname)
{
  if (NULL == fname)
    return false;

  stream_.Reset(fopen(fname, "r"), fclose);
  if (NULL == stream_.Get())
    return false;

  fname_  = fname;
  lineno_ = 1;
  bsize_  = 0;
  lexpos_ = 0;
  eof_    = false;

  return true;
}

void 
Lexer::Close(void)
{
}

Token::Type 
Lexer::GetToken(Token& token)
{
  typedef Token::Type (Lexer::*LexerFunction)(int, State&, bool&);
  static LexerFunction kLexerWorker[] = {
    &Lexer::LexerBegin, 
    &Lexer::LexerFinish, 

    &Lexer::LexerCInt, 
    &Lexer::LexerCReal, 
    &Lexer::LexerCStr, 
    &Lexer::LexerID, 
    &Lexer::LexerAssign, 
    &Lexer::LexerComment, 
  };

  Token::Type type = Token::TYPE_ERR;
  State state = STATE_BEGIN;
  bool save;
  int c;

  token.name = "";
  while (STATE_FINISH != state) {
    c = GetChar();
    save = true;

    type = (this->*kLexerWorker[state])(c, state, save);
    
    if (save)
      token.name += static_cast<char>(c);

    if (STATE_FINISH == state) {
      if (Token::TYPE_ID == type)
        type = Lookup(token.name);

      token.type = type;
      token.line.fname  = fname_;
      token.line.lineno = lineno_;
    }
  }

  return type;
}




void 
Lexer::LoadReserveds(void)
{
  kReserveds["package"]   = Token::TYPE_PACKAGE;
  kReserveds["import"]    = Token::TYPE_IMPORT;
  kReserveds["enum"]      = Token::TYPE_ENUM;
  kReserveds["message"]   = Token::TYPE_MESSAGE;
  kReserveds["extend"]    = Token::TYPE_EXTEND;
  kReserveds["required"]  = Token::TYPE_REQUIRED;
  kReserveds["optional"]  = Token::TYPE_OPTIONAL;
  kReserveds["repeated"]  = Token::TYPE_REPEATED;
  kReserveds["default"]   = Token::TYPE_DEFAULT;
  kReserveds["bool"]      = Token::TYPE_BOOL;
  kReserveds["bytes"]     = Token::TYPE_BYTES;
  kReserveds["string"]    = Token::TYPE_STRING;
  kReserveds["int32"]     = Token::TYPE_INT32;
  kReserveds["uint32"]    = Token::TYPE_UINT32;
  kReserveds["int64"]     = Token::TYPE_INT64;
  kReserveds["uint64"]    = Token::TYPE_UINT64;
  kReserveds["real32"]    = Token::TYPE_REAL32;
  kReserveds["real64"]    = Token::TYPE_REAL64;
}

int 
Lexer::GetChar(void)
{
  if (lexpos_ >= bsize_) {
    if (NULL != fgets(lexbuf_, BSIZE, stream_.Get())) {
      bsize_  = static_cast<int>(strlen(lexbuf_));
      lexpos_ = 0;
    }
  }
  else {
    eof_ = true;
    return EOF;
  }

  return lexbuf_[lexpos_++];
}

void 
Lexer::UngetChar(void)
{
  if (!eof_)
    --lexpos_;
}

Token::Type 
Lexer::Lookup(const std::string& key)
{
  std::map<std::string, Token::Type>::iterator it;
  it = kReserveds.find(key);
  if (it != kReserveds.end())
    return it->second;

  return Token::TYPE_ID;
}


Token::Type 
Lexer::LexerBegin(int c, State& out_state, bool& out_save)
{
  Token::Type type = Token::TYPE_ERR;

  if (' ' == c || '\t' == c) {
    out_save = false;
  }
  else if ('\n' == c) {
    out_save = false;
    ++lineno_;
  }
  else if (isdigit(c)) {
    out_state = STATE_CINT;
  }
  else if ('\"' == c) {
    out_state = STATE_CSTR;
  }
  else if (isalpha(c) || '_' == c) {
    out_state = STATE_ID;
  }
  else if ('=' == c) {
    out_state = STATE_ASSIGN;
  }
  else if ('#' == c) {
    out_state = STATE_COMMENT;
  }
  else {
    out_state = STATE_FINISH;
    switch (c) {
    case EOF:
      out_save = false;
      type = Token::TYPE_EOF;
      break;
    case '.':
      type = Token::TYPE_DOT;
      break;
    case ';':
      type = Token::TYPE_SEMI;
      break;
    case '[':
      type = Token::TYPE_LBRACKET;
      break;
    case ']':
      type = Token::TYPE_RBRACKET;
      break;
    case '{':
      type = Token::TYPE_LBRACE;
      break;
    case '}':
      type = Token::TYPE_RBRACE;
      break;
    default:
      out_save = false;
      break;
    }
  }

  return type;
}

Token::Type 
Lexer::LexerFinish(int c, State& out_state, bool& out_save)
{
  out_state = STATE_FINISH;
  out_save = false;

  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerCInt(int c, State& out_state, bool& out_save)
{
  if ('.' == c) {
    out_state = STATE_CREAL;
  }
  else {
    if (!isdigit(c)) {
      UngetChar();
      out_state = STATE_FINISH;
      out_save = false;

      return Token::TYPE_CINT;
    }
  }

  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerCReal(int c, State& out_state, bool& out_save)
{
  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerCStr(int c, State& out_state, bool& out_save)
{
  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerID(int c, State& out_state, bool& out_save)
{
  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerAssign(int c, State& out_state, bool& out_save)
{
  return Token::TYPE_ERR;
}

Token::Type 
Lexer::LexerComment(int c, State& out_state, bool& out_save)
{
  return Token::TYPE_ERR;
}
