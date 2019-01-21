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
#include "scanner.h"

namespace lox {

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

Token Scanner::make_token(TokenKind kind) {
  auto literal = gen_literal(begpos_, curpos_);
  return Token(kind, literal, lineno_);
}

Token Scanner::error_token(const std::string& message) {
  return Token(TokenKind::TK_ERROR, message, lineno_);
}

Token Scanner::scan_token(void) {
  begpos_ = curpos_;

  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  switch (c) {
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case ';': return make_token(TokenKind::TK_SEMI);
  case ',': return make_token(TokenKind::TK_COMMA);
  case '.': return make_token(TokenKind::TK_DOT);
  case '+': return make_token(TokenKind::TK_PLUS);
  case '-': return make_token(TokenKind::TK_MINUS);
  case '*': return make_token(TokenKind::TK_STAR);
  case '/': return make_token(TokenKind::TK_SLASH);
  case '%': return make_token(TokenKind::TK_PERCENT);
  case '!':
    return make_token(match('=') ?
        TokenKind::TK_BANGEQUAL : TokenKind::TK_BANG);
  case '=':
    return make_token(match('=') ?
        TokenKind::TK_EQUALEQUAL : TokenKind::TK_EQUAL);
  case '>':
    return make_token(match('=') ?
        TokenKind::TK_GREATEREQUAL : TokenKind::TK_GREATER);
  case '<':
    return make_token(match('=') ?
        TokenKind::TK_LESSEQUAL : TokenKind::TK_LESS);
  }

  return error_token("unexpected character ...");
}

}
