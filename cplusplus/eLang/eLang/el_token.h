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
#ifndef __EL_TOKEN_HEADER_H__
#define __EL_TOKEN_HEADER_H__

namespace el {

enum class TokenType {
  TOKEN_LEFT_PAREN,     // (
  TOKEN_RIGHT_PAREN,    // )
  TOKEN_LEFT_BRACKET,   // [
  TOKEN_RIGHT_BRACKET,  // ]
  TOKEN_LEFT_BRACE,     // {
  TOKEN_RIGHT_BRACE,    // }
  TOKEN_AT,             // @
  TOKEN_DOT,            // .
  TOKEN_HASH,           // #
  TOKEN_PIPE,           // |
  TOKEN_SEMICOLON,      // ;

  TOKEN_ARROW,          // <-
  TOKEN_LONG_ARROW,     // <--
  TOKEN_BIND,           // ::
  TOKEN_SELF,           // self
  TOKEN_UNDEFINED,      // undefined
  TOKEN_RETURN,         // return

  TOKEN_NUMBER,
  TOKEN_STRING,

  TOKEN_NAME,
  TOKEN_OPERATOR,
  TOKEN_KEYWORD,

  TOKEN_LINE,           // Includes ',' too since they are equivalent in el.
  TOKEN_IGNORE_LINE,

  TOKEN_ERROR,
  TOKEN_EOF,
};

class Token {
  TokenType type_;
  double    number_;
  String    text_;
public:
  explicit Token(TokenType type)
    : type_(type)
    , number_(0)
    , text_() {
  }

  Token(TokenType type, double number)
    : type_(type)
    , number_(number)
    , text_() {
  }

  Token(TokenType type, const String& text)
    : type_(type)
    , number_(0)
    , text_(text) {
  }

  inline TokenType Type(void) const {
    return type_;
  }

  inline double Number(void) const {
    return number_;
  }

  inline String Text(void) const {
    return text_;
  }
};

std::ostream& operator<<(std::ostream& cout, const Token& token);

}

#endif  // __EL_TOKEN_HEADER_H__
