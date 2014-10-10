// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "ini_parser.h"

struct Token {
  enum Type {
    TYPE_ERR = 0, 
    TYPE_EOF, 
    TYPE_COMMENT,   // #

    TYPE_STRING, 

    TYPE_LBRACKET,  // [
    TYPE_RBRACKET,  // ]
    TYPE_ASSIGN,    // =
  };

  Type        type;
  std::string token;
};

class IniLexer : private UnCopyable {
  enum {BSIZE = 256};
  enum State {
    STATE_BEGIN = 0, 
    STATE_FINISH, 

    STATE_STRING, 
    STATE_COMMENT,
  };

  int   lineno_;
  int   bsize_;
  bool  eof_;
  FILE* file_;
  int   lexpos_;
  char  lexbuf_[BSIZE];
public:
  IniLexer(void);
  ~IniLexer(void);

  bool Open(const char* fname);
  void Close(void);

  Token::Type GetToken(Token& token);
private:
  int GetChar(void);
  void UngetChar(void);

  Token::Type LexerBegin(int c, State& out_state, bool& out_save);
  Token::Type LexerFinish(int c, State& out_state, bool& out_save);
  Token::Type LexerString(int c, State& out_state, bool& out_save);
  Token::Type LexerComment(int c, State& out_state, bool& out_save);
};

IniLexer::IniLexer(void) 
  : lineno_(1)
  , bsize_(0)
  , eof_(false)
  , file_(NULL)
  , lexpos_(0) {
}

IniLexer::~IniLexer(void){
  Close();
}

bool IniLexer::Open(const char* fname) {
  if (NULL == fname)
    return false;

  if (NULL == (file_ = fopen(fname, "r")))
    return false;
  lineno_ = 1;
  bsize_  = 0;
  eof_    = false;
  lexpos_ = 0;

  return true;
}

void IniLexer::Close(void) {
  if (NULL != file_) {
    fclose(file_);
    file_ = NULL;
  }
}

Token::Type IniLexer::GetToken(Token& token) {
  typedef Token::Type (IniLexer::*FunctionPtr)(int, State&, bool&);
  static FunctionPtr kCallbacks[] = {
    &IniLexer::LexerBegin, 
    &IniLexer::LexerFinish, 

    &IniLexer::LexerString, 
    &IniLexer::LexerComment, 
  };

  Token::Type type = Token::TYPE_ERR;
  State state = STATE_BEGIN;
  bool  save;
  int   c;

  token.token = "";
  while (STATE_FINISH != state) {
    c = GetChar();
    save = true;

    type = (this->*kCallbacks[state])(c, state, save);

    if (save)
      token.token += static_cast<char>(c);
    if (STATE_FINISH == state)
      token.type = type;
  }

  return type;
}

int IniLexer::GetChar(void) {
  if (lexpos_ >= bsize_) {
    if (NULL != fgets(lexbuf_, BSIZE, file_)) {
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

void IniLexer::UngetChar(void) {
  if (!eof_)
    --lexpos_;
}

Token::Type IniLexer::LexerBegin(
    int c, State& out_state, bool& out_save) {
  Token::Type type = Token::TYPE_ERR;

  if (' ' == c || '\t' == c) {
    out_save = false;
  }
  else if ('\n' == c) {
    out_save = false;
    ++lineno_;
  }
  else if ('#' == c) {
    out_save = false;
    out_state = STATE_COMMENT;
  }
  else if ('[' == c) {
    out_state = STATE_FINISH;
    type = Token::TYPE_LBRACKET;
  }
  else if (']' == c) {
    out_state = STATE_FINISH;
    type = Token::TYPE_RBRACKET;
  }
  else if ('=' == c) {
    out_state = STATE_FINISH;
    type = Token::TYPE_ASSIGN;
  }
  else if (EOF == c) {
    out_state = STATE_FINISH;
    type = Token::TYPE_EOF;
  }
  else {
    out_state = STATE_STRING;
  }

  return type;
}

Token::Type IniLexer::LexerFinish(
    int c, State& out_state, bool& out_save) {
  out_state = STATE_FINISH;
  out_save = false;

  return Token::TYPE_ERR;
}

Token::Type IniLexer::LexerString(
    int c, State& out_state, bool& out_save) {
  if (' ' == c || '\t' == c || '\n' == c 
      || '#' == c || '=' == c || ']' == c) {
    if ('\n' == c || '#' == c || '=' == c || ']' == c)
      UngetChar();

    out_state = STATE_FINISH;
    out_save = false;

    return Token::TYPE_STRING;
  }

  return Token::TYPE_ERR;
}

Token::Type IniLexer::LexerComment(
    int c, State& out_state, bool& out_save) {
  out_save = false;
  if (EOF == c) {
    out_state = STATE_FINISH;
    return Token::TYPE_EOF;
  }
  else if ('\n' == c) {
    ++lineno_;
    out_state = STATE_BEGIN;
  }

  return Token::TYPE_ERR;
}




IniParser::IniParser(void) 
  : lexer_(NULL)
  , section_("") {
}

IniParser& IniParser::GetSingleton(void) {
  static IniParser parser;
  return parser;
}

bool IniParser::Open(const char* fname) {
  if (NULL == (lexer_ = new IniLexer()))
    return false;

  if (!lexer_->Open(fname))
    return false;
  section_ = "";
  values_.clear();

  Parse();

  return true;
}

void IniParser::Close(void) {
  if (NULL != lexer_) {
    lexer_->Close();

    delete lexer_;
    lexer_ = NULL;
  }
}

std::string IniParser::Get(
    const std::string& section, const std::string& key) {
  std::string name(section + key);

  ValueMap::iterator found(values_.find(name));
  if (found != values_.end())
    return (*found).second.c_str();

  return "";
}

void IniParser::Parse(void) {
  Token token;
  Token::Type type;
  do {
    type = lexer_->GetToken(token);
    if (Token::TYPE_LBRACKET == type) {
      ParseSection();
    }
    else if (Token::TYPE_STRING == type) {
      ParseItem(token.token);
    }
  } while (Token::TYPE_EOF != type);
}

void IniParser::ParseSection(void) {
  Token token;
  Token::Type type = lexer_->GetToken(token);
  if (Token::TYPE_STRING != type) {
    fprintf(stderr, "parse section error ... %d\n", __LINE__);
    abort();
  }

  section_ = token.token;
  if (Token::TYPE_RBRACKET != lexer_->GetToken(token)) {
    section_ = "";

    fprintf(stderr, "parse section error ... %d\n", __LINE__);
    abort();
  }
}

void IniParser::ParseItem(const std::string& str) {
  std::string key(section_ + str);
  
  Token token;
  if (Token::TYPE_ASSIGN != lexer_->GetToken(token)) {
    fprintf(stderr, "parse item error ... %d\n", __LINE__);
    abort();
  }

  Token::Type type = lexer_->GetToken(token);
  if (Token::TYPE_STRING != type) {
    fprintf(stderr, "parse item error ... %d\n", __LINE__);
    abort();
  }

  values_[key] = token.token;
}
