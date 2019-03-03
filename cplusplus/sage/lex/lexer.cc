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
#include "../common/errors.hh"
#include "lexer.hh"

namespace sage {

Lexer::Lexer(ErrorReport& err_report,
    const std::string& source_bytes, const std::string& fname)
  : err_report_(err_report)
  , source_bytes_(source_bytes)
  , fname_(fname) {
}

std::vector<Token>& Lexer::parse_tokens(void) {
  while (!is_end()) {
    begpos_ = curpos_;
    next_token();
  }

  tokens_.push_back(Token(TokenKind::TK_EOF, "", fname_, lineno_));
  return tokens_;
}

bool Lexer::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Lexer::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

std::string Lexer::gen_literal(std::size_t begpos, std::size_t endpos) const {
  return source_bytes_.substr(begpos, endpos - begpos);
}

bool Lexer::is_end(void) const {
  return curpos_ >= source_bytes_.size();
}

char Lexer::advance(void) {
  return source_bytes_[curpos_++];
}

bool Lexer::match(char expected) {
  if (is_end() || source_bytes_[curpos_] != expected)
    return false;

  ++curpos_;
  return true;
}

char Lexer::peek(void) const {
  if (curpos_ >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_];
}

char Lexer::peek_next(void) const {
  if (curpos_ + 1 >= source_bytes_.size())
    return 0;
  return source_bytes_[curpos_ + 1];
}

void Lexer::next_token(void) {
  char c = advance();
  switch (c) {
  case '[': make_token(TokenKind::TK_LSQUARE); break;
  case ']': make_token(TokenKind::TK_RSQUARE); break;
  case '(': make_token(TokenKind::TK_LPAREN); break;
  case ')': make_token(TokenKind::TK_RPAREN); break;
  case '{': make_token(TokenKind::TK_LBRACE); break;
  case '}': make_token(TokenKind::TK_RBRACE); break;
  case '.':
    if (match('.')) {
      make_token(match('.')
          ? TokenKind::TK_PERIODPERIODPERIOD
          : TokenKind::TK_PERIODPERIOD);
    }
    else {
      make_token(TokenKind::TK_PERIOD);
    }
    break;
  case ',': make_token(TokenKind::TK_COMMA); break;
  case ':': make_token(TokenKind::TK_COLON); break;
  case ';': make_token(TokenKind::TK_SEMI); break;
  case '+':
    make_token(match('=') ? TokenKind::TK_PLUSEQUAL : TokenKind::TK_PLUS);
    break;
  case '-':
    make_token(match('=') ? TokenKind::TK_MINUSEQUAL : TokenKind::TK_MINUS);
    break;
  case '*':
    make_token(match('=') ? TokenKind::TK_STAREQUAL : TokenKind::TK_STAR);
    break;
  case '/':
    make_token(match('=') ? TokenKind::TK_SLASHEQUAL : TokenKind::TK_SLASH);
    break;
  case '%':
    make_token(match('=') ? TokenKind::TK_PERCENTEQUAL : TokenKind::TK_PERCENT);
    break;
  case '<':
    make_token(match('=') ? TokenKind::TK_LESSEQUAL : TokenKind::TK_LESS);
    break;
  case '>':
    make_token(match('=') ? TokenKind::TK_GREATEREQUAL : TokenKind::TK_GREATER);
    break;
  case '!':
    make_token(match('=') ? TokenKind::TK_EXCLAIMEQUAL : TokenKind::TK_EXCLAIM);
    break;
  case '=':
    make_token(match('=') ? TokenKind::TK_EQUALEQUAL : TokenKind::TK_EQUAL);
    break;
  case '#': skip_comment(); break;
  case ' ':
  case '\r':
  case '\t':
    // ignore whitespaces
    break;
  case '\n': make_token(TokenKind::TK_NL, "NL"); ++lineno_; break;
  case '"': make_string(); break;
  default:
    if (std::isdigit(c)) {
      make_numeric();
    }
    else if (is_alpha(c)) {
      make_identifier();
    }
    else {
      err_report_.error(fname_, lineno_, "unexpected charactor");
      return;
    }
    break;
  }
}

void Lexer::make_token(TokenKind kind) {
  auto literal = gen_literal(begpos_, curpos_);
  tokens_.push_back(Token(kind, literal, fname_, lineno_));
}

void Lexer::make_token(TokenKind kind, const std::string& literal) {
  tokens_.push_back(Token(kind, literal, fname_, lineno_));
}

void Lexer::skip_comment(void) {
  while (!is_end() && peek() != '\n')
    advance();
}

void Lexer::make_string(void) {
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

  // unterminated string
  if (is_end()) {
    err_report_.error(fname_, lineno_, "unterminated string");
    return;
  }

  // the closing "
  advance();

  // trim the surrounding string
  make_token(TokenKind::TK_STRINGLITERAL, literal);
}

void Lexer::make_numeric(void) {
  while (std::isdigit(peek()))
    advance();

  TokenKind kind{TokenKind::TK_INTEGERCONST};
  if (peek() == '.' && std::isdigit(peek_next())) {
    advance();
    while (std::isdigit(peek()))
      advance();

    kind = TokenKind::TK_DECIMALCONST;
  }

  if (is_alpha(peek())) {
    err_report_.error(fname_, lineno_, "invalid numeric or identifier");
    return;
  }

  make_token(kind);
}

void Lexer::make_identifier(void) {
  while (is_alnum(peek()))
    advance();

  auto literal = gen_literal(begpos_, curpos_);
  make_token(get_keyword_kind(literal.c_str()), literal);
}

}
