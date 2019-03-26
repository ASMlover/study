// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <cctype>
#include "scanner.hh"

namespace lvm {

Scanner::Scanner(const std::string& source_bytes)
  : source_bytes_(source_bytes) {
}

Token Scanner::scan_token(void) {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();

  if (std::isdigit(c))
    return make_numeric();

  switch (c) {
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case ',': return make_token(TokenKind::TK_COMMA);
  case '.': return make_token(TokenKind::TK_DOT);
  case ';': return make_token(TokenKind::TK_SEMI);
  case '+': return make_token(TokenKind::TK_PLUS);
  case '-': return make_token(TokenKind::TK_MINUS);
  case '*': return make_token(TokenKind::TK_STAR);
  case '/': return make_token(TokenKind::TK_SLASH);
  case '!':
    return make_token(match('=') ?
        TokenKind::TK_BANGEQUAL : TokenKind::TK_BANG);
  case '=':
    return make_token(match('=') ?
        TokenKind::TK_EQUALEQUAL : TokenKind::TK_EQUAL);
  case '<':
    return make_token(match('<') ?
        TokenKind::TK_LESSEQUAL : TokenKind::TK_LESS);
  case '>':
    return make_token(match('>') ?
        TokenKind::TK_GREATEREQUAL : TokenKind::TK_GREATER);
  case '"': return make_string();
  }

  return error_token("unexpected character");
}

bool Scanner::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Scanner::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

std::string Scanner::gen_literal(std::size_t begpos, std::size_t endpos) const {
  return source_bytes_.substr(begpos, endpos - begpos);
}

bool Scanner::is_end(void) const {
  return curpos_ >= source_bytes_.size();
}

char Scanner::advance(void) {
  return source_bytes_[curpos_++];
}

bool Scanner::match(char expected) {
  if (is_end() || source_bytes_[curpos_] != expected)
    return false;

  ++curpos_;
  return true;
}

char Scanner::peek(void) const {
  if (curpos_ >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_];
}

char Scanner::peek_next(void) const {
  if (curpos_ + 1 >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_ + 1];
}

Token Scanner::error_token(const std::string& message) {
  return Token(TokenKind::TK_ERROR, message, lineno_);
}

Token Scanner::make_token(TokenKind kind) {
  return Token(kind, gen_literal(begpos_, curpos_), lineno_);
}

Token Scanner::make_token(TokenKind kind, const std::string& literal) {
  return Token(kind, literal, lineno_);
}

void Scanner::skip_whitespace(void) {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance(); break;
    case '\n':
      ++lineno_; advance(); break;
    case '/':
      if (peek_next() == '/') {
        while (!is_end() && peek() != '\n')
          advance();
      }
      else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Token Scanner::make_string(void) {
  std::string literal;
  while (!is_end() && peek() != '"') {
    char c = peek();
    switch (c) {
    case '\n': ++lineno_; break;
    case '\\':
      switch (peek_next()) {
      case '"': c = '"'; advance(); break;
      case '\\': c = '\\'; advance(); break;
      case '%': c = '%'; advance(); break;
      case '0': c = '\0'; advance(); break;
      case 'a': c = '\a'; advance(); break;
      case 'b': c = '\b'; advance(); break;
      case 'f': c = '\f'; advance(); break;
      case 'n': c = '\n'; advance(); break;
      case 'r': c = '\r'; advance(); break;
      case 't': c = '\t'; advance(); break;
      case 'v': c = '\v'; advance(); break;
      }
      break;
    }
    literal.push_back(c);
    advance();
  }

  if (is_end())
    return error_token("unterminated string");

  advance();
  return make_token(TokenKind::TK_STRINGLITERAL, literal);
}

Token Scanner::make_numeric(void) {
  while (std::isdigit(peek()))
    advance();
  if (peek() == '.' && std::isdigit(peek_next())) {
    advance();
    while (std::isdigit(peek()))
      advance();
  }

  return make_token(TokenKind::TK_NUMERICCONST);
}

}
