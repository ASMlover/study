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
#include "el_token.h"
#include "el_reader_base.h"
#include "el_lexer.h"

namespace el {

bool Lexer::IsInfinite(void) const {
  return reader_.IsInfinite();
}

Ref<Token> Lexer::ReadToken(void) {
  while (true) {
    if (IsDone())
      return Ref<Token>(new Token(TokenType::TOKEN_EOF));

    if (needs_line_) {
      Advance();
      continue;
    }

    start_ = pos_;
    char c = Peek();
    switch (c) {
    case ' ':
    case '\t':
      while (IsWhitespace(Peek()))
        Advance();
      break;
    case '\0':
      needs_line_ = true;
      return Ref<Token>(new Token(TokenType::TOKEN_LINE));
    case '(':
      return SingleToken(TokenType::TOKEN_LEFT_PAREN);
    case ')':
      return SingleToken(TokenType::TOKEN_RIGHT_PAREN);
    case '[':
      return SingleToken(TokenType::TOKEN_LEFT_BRACKET);
    case ']':
      return SingleToken(TokenType::TOKEN_RIGHT_BRACKET);
    case '{':
      return SingleToken(TokenType::TOKEN_LEFT_BRACE);
    case '}':
      return SingleToken(TokenType::TOKEN_RIGHT_BRACE);
    case '@':
      return SingleToken(TokenType::TOKEN_AT);
    case '.':
      return SingleToken(TokenType::TOKEN_DOT);
    case '#':
      return SingleToken(TokenType::TOKEN_HASH);
    case '|':
      return SingleToken(TokenType::TOKEN_PIPE);
    case ';':
      return SingleToken(TokenType::TOKEN_SEMICOLON);
    case ',':
      return SingleToken(TokenType::TOKEN_LINE);
    case '\\':
      return SingleToken(TokenType::TOKEN_IGNORE_LINE);
    case ':':
      Advance();
      if (':' == Peek())
        return SingleToken(TokenType::TOKEN_BIND);
      return Ref<Token>(new Token(TokenType::TOKEN_KEYWORD, ":"));
    case '-':
      Advance();
      if (IsDigit(Peek()))
        return ReadNumber();
      return ReadOperator();
    case '/':
      Advance();
      if ('/' == Peek()) {
        needs_line_ = true;
        return Ref<Token>(new Token(TokenType::TOKEN_LINE));
      }
      else if ('*' == Peek()) {
        SkipBlockComment();
      }
      else {
        return ReadOperator();
      }
      break;
    case '"':
      return ReadString();
    default:
      if (IsDigit(c)) {
        return ReadNumber();
      }
      else if (IsAlpha(c)) {
        return ReadName();
      }
      else if (IsOperator(c)) {
        return ReadOperator();
      }
      else {
        Advance();
        return Ref<Token>(new Token(TokenType::TOKEN_ERROR,
              String::Format("Unrecognized character \"%c\".", c)));
      }
    }
  }
}

bool Lexer::IsDone(void) const {
  return needs_line_ && reader_.EndOfLines();
}

char Lexer::Peek(int ahead) const {
  if (pos_ + ahead >= line_.Length())
    return '\0';
  return line_[pos_ + ahead];
}

char Lexer::Advance(void) {
  char c = Peek();
  ++pos_;
  return c;
}

void Lexer::AdvanceLine(void) {
  line_ = reader_.NextLine();
  pos_ = 0;
  start_ = 0;
  needs_line_ = false;
}

void Lexer::SkipBlockComment(void) {
  Advance();
  Advance();

  int nesting = 1;
  while (nesting > 0) {
    if (IsDone())
      return;

    if ('/' == Peek() && '*' == Peek(1)) {
      Advance();
      Advance();
      ++nesting;
    }
    else if ('*' == Peek() && '/' == Peek(1)) {
      Advance();
      Advance();
      --nesting;
    }
    else if ('\0' == Peek()) {
      AdvanceLine();
    }
    else {
      Advance();
    }
  }
}

bool Lexer::IsWhitespace(char c) const {
  return (' ' == c || '\t' == c);
}

bool Lexer::IsAlpha(char c) const {
  return isalpha(c) ? true : false;
}

bool Lexer::IsDigit(char c) const {
  return isdigit(c) ? true : false;
}

bool Lexer::IsOperator(char c) const {
  return ('\0' != c && nullptr != strchr("-+=/<>?~!$%^&*", c));
}

Ref<Token> Lexer::SingleToken(TokenType type) {
  Advance();
  return Ref<Token>(new Token(type));
}

Ref<Token> Lexer::ReadString(void) {
  Advance();

  String text;
  while (true) {
    if (IsDone()) {
      return Ref<Token>(new Token(
            TokenType::TOKEN_ERROR, "Unterminated string."));
    }

    char c = Advance();
    if ('"' == c)
      return Ref<Token>(new Token(TokenType::TOKEN_STRING, text));

    if ('\\' == c) {
      if (IsDone()) {
        return Ref<Token>(new Token(
              TokenType::TOKEN_ERROR, "Unterminated string escape."));
      }

      char e = Advance();
      switch (e) {
      case 'n':
        text += '\n'; break;
      case '"':
        text += '\"'; break;
      case '\\':
        text += '\\'; break;
      case 't':
        text += '\t'; break;
      default:
        return Ref<Token>(new Token(TokenType::TOKEN_ERROR, 
              String::Format("Unrecognized escape sequence \"%c\"", e)));
      }
    }
    else {
      text += c;
    }
  }
}

Ref<Token> Lexer::ReadNumber(void) {
  Advance();
  while (IsDigit(Peek()))
    Advance();

  if ('.' == Peek()) {
    Advance();
    while (IsDigit(Peek()))
      Advance();
  }

  String text = line_.SubString(start_, pos_ - start_);
  return Ref<Token>(new Token(TokenType::TOKEN_NUMBER, text.ToNumber()));
}

Ref<Token> Lexer::ReadName(void) {
  while (IsOperator(Peek()) || IsAlpha(Peek()) || IsDigit(Peek())) {
    if ('/' == Peek() && '/' == Peek(1))
      break;
    if ('/' == Peek() && '*' == Peek(1))
      break;
    Advance();
  }

  TokenType type = TokenType::TOKEN_NAME;
  if (':' == Peek()) {
    Advance();
    type = TokenType::TOKEN_KEYWORD;
  }

  String name = line_.SubString(start_, pos_ - start_);
  if (name == "return")
    return Ref<Token>(new Token(TokenType::TOKEN_RETURN));
  else if (name == "self")
    return Ref<Token>(new Token(TokenType::TOKEN_SELF));
  else if (name == "undefined")
    return Ref<Token>(new Token(TokenType::TOKEN_UNDEFINED));

  return Ref<Token>(new Token(type, name));
}

Ref<Token> Lexer::ReadOperator(void) {
  while (IsOperator(Peek())) {
    if ('/' == Peek() && '*' == Peek(1))
      break;
    if ('/' == Peek() && '/' == Peek(1))
      break;
    Advance();
  }

  if (IsAlpha(Peek()))
    return ReadName();

  String oper = line_.SubString(start_, pos_ - start_);
  if (oper == "<-")
    return Ref<Token>(new Token(TokenType::TOKEN_ARROW));
  else if (oper == "<--")
    return Ref<Token>(new Token(TokenType::TOKEN_LONG_ARROW));

  return Ref<Token>(new Token(TokenType::TOKEN_OPERATOR, oper));
}

}
