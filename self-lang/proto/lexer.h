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
#ifndef __LEXER_HEADER_H__
#define __LEXER_HEADER_H__

struct Token {
  enum Type {
    TYPE_ERR = 0, 
    TYPE_EOF,       //!< EOF 
    TYPE_COMMENT,   //!< #

    TYPE_CINT,      //!< const int number
    TYPE_CREAL,     //!< const real number
    TYPE_CSTR,      //!< const string 

    TYPE_ID,        //!< identifier

    TYPE_DOT,       //!< .
    TYPE_ASSIGN,    //!< =
    TYPE_SEMI,      //!< ;
    TYPE_LBRACKET,  //!< [
    TYPE_RBRACKET,  //!< ]
    TYPE_LBRACE,    //!< {
    TYPE_RBRACE,    //!< }

    TYPE_PACKAGE,   //!< package
    TYPE_IMPORT,    //!< import
    TYPE_ENUM,      //!< enum
    TYPE_MESSAGE,   //!< message
    TYPE_EXTEND,    //!< extend
    TYPE_REQUIRED,  //!< required
    TYPE_OPTIONAL,  //!< optional
    TYPE_REPEATED,  //!< repeated
    TYPE_DEFAULT,   //!< default
    TYPE_BOOL,      //!< bool
    TYPE_BYTES,     //!< bytes
    TYPE_STRING,    //!< string
    TYPE_INT32,     //!< int32
    TYPE_UINT32,    //!< uint32
    TYPE_INT64,     //!< int64
    TYPE_UINT64,    //!< uint64
    TYPE_REAL32,    //!< real32
    TYPE_REAL64,    //!< real64
  };

  Type        type;
  std::string name;
  struct Line {
    std::string fname;
    uint32_t    lineno;
  } line;
};


class Lexer : private util::UnCopyable {
  enum State {
    STATE_BEGIN = 0, 
    STATE_FINISH, 

    STATE_CINT,     //!< const int number
    STATE_CREAL,    //!< const real number
    STATE_CSTR,     //!< const string 
    STATE_ID,       //!< identifier
    STATE_ASSIGN,   //!< =
    STATE_COMMENT,  //!< #
  };

  enum {BSIZE = 128};

  util::SmartPtr<FILE>  stream_;
  std::string           fname_;
  int                   lineno_;
  int                   bsize_;
  int                   lexpos_;
  bool                  eof_;
  char                  lexbuf_[BSIZE];

  static std::map<std::string, Token::Type> kReserveds;
public:
  Lexer(void);
  ~Lexer(void);

  bool Open(const char* fname);
  void Close(void);

  Token::Type GetToken(Token& token);
private:
  void LoadReserveds(void);
  int GetChar(void);
  void UngetChar(void);
  Token::Type Lookup(const std::string& key);

  Token::Type LexerBegin(int c, State& out_state, bool& out_save);
  Token::Type LexerFinish(int c, State& out_state, bool& out_save);
  Token::Type LexerCInt(int c, State& out_state, bool& out_save);
  Token::Type LexerCReal(int c, State& out_state, bool& out_save);
  Token::Type LexerCStr(int c, State& out_state, bool& out_save);
  Token::Type LexerID(int c, State& out_state, bool& out_save);
  Token::Type LexerAssign(int c, State& out_state, bool& out_save);
  Token::Type LexerComment(int c, State& out_state, bool& out_save);
};

#endif  //! __LEXER_HEADER_H__
