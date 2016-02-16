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
#include <unordered_map>
#include "lexer.h"

class ReservedManager : private UnCopyable {
  std::unordered_map<std::string, Token::Type> reservedMap_;
public:
  static ReservedManager& Instance(void) {
    static ReservedManager inst;
    return inst;
  }

  Token::Type getReserved(const std::string& reserved) {
    Token::Type type = Token::Type::TYPE_EOF;
    if (reservedMap_.end() != reservedMap_.find(reserved))
      type = reservedMap_[reserved];

    return type;
  }
private:
  ReservedManager(void) {
    Init();
  }

  void Init(void) {
    reservedMap_["def"] = Token::Type::TYPE_DEF;
    reservedMap_["extern"] = Token::Type::TYPE_EXT;
  }
};

Lexer::Lexer(void)
  : stream_(static_cast<FILE*>(nullptr))
  , fname_("")
  , lineno_(1)
  , colum_(1)
  , bsize_(0)
  , lexpos_(0)
  , eof_(false) {
}

Lexer::~Lexer(void) {
}

bool Lexer::Open(const std::string& fname) {
  stream_.reset(fopen(fname.c_str(), "r"), fclose);
  if (!stream_)
    return false;

  fname_  = fname;
  lineno_ = 1;
  colum_  = 1;
  bsize_  = 0;
  lexpos_ = 0;
  eof_    = false;

  return true;
}

void Lexer::Close(void) {
}

Token::Type Lexer::GetToken(Token& token) {
  static LexerFunction kLexerWorkers[] = {
    &Lexer::LexerBegin,
    &Lexer::LexerFinish,

    &Lexer::LexerReal,
    &Lexer::LexerStr,
    &Lexer::LexerID,
    &Lexer::LexerAssign,
    &Lexer::LexerComment,
  };

  Token::Type type = Token::Type::TYPE_EOF;
  State state = State::STATE_BEGIN;
  bool save;
  int c;

  token.name.clear();
  while (State::STATE_FINISH != state) {
    c = GetChar();
    save = true;

    ++colum_;
    type = (this->*kLexerWorkers[state])(c, state, save);
    if (save)
      token.name += static_cast<char>(c);

    if (State::STATE_FINISH == state) {
      if (Token::Type::TYPE_ID == type) {
        // pass
      }

      token.type            = type;
      token.position.fname  = fname_;
      token.position.lineno = lineno_;
      token.position.colum  = colum_;
    }
  }

  return type;
}

int Lexer::GetChar(void) {
  if (lexpos_ >= bsize_) {
    if (nullptr != fgets(lexbuf_, BSIZE, stream_.get())) {
      bsize_ = static_cast<int>(strlen(lexbuf_));
      lexpos_ = 0;
    }
    else {
      eof_ = true;
      return EOF;
    }
  }

  return lexbuf_[lexpos_++];
}

void Lexer::UngetChar(void) {
  if (!eof_)
    --lexpos_;
}

Token::Type Lexer::LexerBegin(int c, State& out_state, bool& out_save) {
  Token::Type type = Token::Type::TYPE_EOF;

  if (' ' == c || '\t' == c) {
    out_save = false;
  }
  else if ('\n' == c) {
    out_save = false;
    ++lineno_;
    colum_ = 1;
  }
  else if (isdigit(c)) {
    out_state = State::STATE_REAL;
  }
  else if ('\"' == c) {
    out_state = State::STATE_STR;
  }
  else if (isalpha(c) || '_' == c) {
    out_state = State::STATE_ID;
  }
  else if ('=' == c) {
    out_state = State::STATE_ASSIGN;
  }
  else if ('#' == c) {
    out_state = State::STATE_COMMENT;
  }
  else {
    out_state = State::STATE_FINISH;
    switch (c) {
    case EOF:
      out_save = false;
      type = Token::Type::TYPE_EOF;
      break;
    default:
      out_save = false;
      break;
    }
  }

  return type;
}

Token::Type Lexer::LexerFinish(int c, State& out_state, bool& out_save) {
  out_state = State::STATE_FINISH;
  out_save = false;

  return Token::Type::TYPE_EOF;
}

Token::Type Lexer::LexerReal(int c, State& out_state, bool& out_save) {
  if (!isdigit(c)) {
    UngetChar();
    out_state = State::STATE_FINISH;
    out_save = false;

    return Token::Type::TYPE_NUM;
  }
  return Token::Type::TYPE_EOF;
}

Token::Type Lexer::LexerStr(int c, State& out_state, bool& out_save) {
  if (c == '\"') {
    out_state = State::STATE_FINISH;
    out_save = false;

    return Token::Type::TYPE_ID;
  }
  return Token::Type::TYPE_EOF;
}

Token::Type Lexer::LexerID(int c, State& out_state, bool& out_save) {
  if (!isalnum(c) && '_' != c) {
    UngetChar();
    out_state = State::STATE_FINISH;
    out_save = false;

    return Token::Type::TYPE_ID;
  }

  return Token::Type::TYPE_EOF;
}

Token::Type Lexer::LexerAssign(int c, State& out_state, bool& out_save) {
  out_state = State::STATE_FINISH;
  out_save = false;

  return Token::Type::TYPE_EOF;
}

Token::Type Lexer::LexerComment(int c, State& out_state, bool& out_save) {
  out_save = false;
  if (EOF == c) {
    out_state = State::STATE_FINISH;
    return Token::Type::TYPE_EOF;
  }
  else if ('\n' == c) {
    out_state = State::STATE_BEGIN;
    ++lineno;
  }

  return Token::Type::TYPE_EOF;
}
