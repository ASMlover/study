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
  State state_;
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

  Token::Type LexerBegin(int c);
  Token::Type LexerFinish(int c);
  Token::Type LexerString(int c);
  Token::Type LexerComment(int c);
};




IniParser::IniParser(void) 
  : lexer_(NULL)
  , section_("") {
}

IniParser& IniParser::GetSingleton(void) {
  static IniParser parser;
  return parser;
}

bool IniParser::Open(const char* fname) {
  return true;
}

void IniParser::Close(void) {
}

std::string IniParser::Get(
    const std::string& section, const std::string& key) {
  return "";
}

void IniParser::Parse(void) {
}

void IniParser::ParseSection(void) {
}

void IniParser::ParseItem(void) {
}

const std::string IniParser::ParseKey(void) {
  return "";
}

const std::string IniParser::ParseValue(void) {
  return "";
}
