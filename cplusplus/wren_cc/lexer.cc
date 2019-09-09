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
#include "lexer.hh"

namespace wrencc {

Token Lexer::next_token(void) {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  if (std::isdigit(c))
    return make_numeric();
  if (is_alpha(c))
    return make_identifier(c);

  switch (c) {
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '[': return make_token(TokenKind::TK_LBRACKET);
  case ']': return make_token(TokenKind::TK_RBRACKET);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case ';': return make_token(TokenKind::TK_NL);
  case ':': return make_token(TokenKind::TK_COLON);
  case '.':
    if (match('.')) {
      return make_token(match('.') ?
          TokenKind::TK_DOTDOTDOT : TokenKind::TK_DOTDOT);
    }
    else {
      return make_token(TokenKind::TK_DOT);
    }
  case ',': return make_token(TokenKind::TK_COMMA);
  case '*': return make_token(TokenKind::TK_STAR);
  case '/': return make_token(TokenKind::TK_SLASH);
  case '%': return make_token(TokenKind::TK_PERCENT);
  case '+': return make_token(TokenKind::TK_PLUS);
  case '~': return make_token(TokenKind::TK_TILDE);
  case '?': return make_token(TokenKind::TK_QUESTION);
  case '-': return make_token(TokenKind::TK_MINUS);
  case '^': return make_token(TokenKind::TK_CARET);
  case '|':
    return make_token(match('|') ? TokenKind::TK_PIPEPIPE :  TokenKind::TK_PIPE);
  case '&':
    return make_token(match('&') ? TokenKind::TK_AMPAMP : TokenKind::TK_AMP);
  case '!':
    return make_token(match('=') ? TokenKind::TK_BANGEQ : TokenKind::TK_BANG);
  case '=':
    return make_token(match('=') ? TokenKind::TK_EQEQ : TokenKind::TK_EQ);
  case '<':
    if (peek() == '<') {
      advance();
      return make_token(TokenKind::TK_LSHIFT);
    }
    else {
      return make_token(match('=') ? TokenKind::TK_LTEQ : TokenKind::TK_LT);
    }
  case '>':
    if (peek() == '>') {
      advance();
      return make_token(TokenKind::TK_RSHIFT);
    }
    else {
      return make_token(match('=') ? TokenKind::TK_GTEQ : TokenKind::TK_GT);
    }
  case '\n': return make_token(TokenKind::TK_NL, lineno_++);
  case '"': return make_string();
  }
  return make_error("unexpected charactor");
}

void Lexer::skip_whitespace(void) {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ': case '\r': case '\t': advance(); break;
    case '/':
      if (peek_next() == '/') {
        while (!is_end() && peek() != '\n')
          advance();
      }
      else {
        return;
      }
      break;
    default: return;
    }
  }
}

Token Lexer::make_token(TokenKind kind, const str_t& literal) {
  return Token(kind, literal, lineno_);
}

Token Lexer::make_token(TokenKind kind) {
  return Token(kind, gen_literal(begpos_, curpos_), lineno_);
}

Token Lexer::make_token(TokenKind kind, int lineno) {
  return Token(kind, gen_literal(begpos_, curpos_), lineno);
}

Token Lexer::make_error(const str_t& error_message) {
  return Token(TokenKind::TK_ERROR, error_message, lineno_);
}

Token Lexer::make_identifier(char beg_char) {
  char cur_char = peek();
  while (is_alnum(peek()))
    advance();

  auto literal = gen_literal(begpos_, curpos_);
  TokenKind kind = get_keyword_kind(literal.c_str());
  if (kind == TokenKind::TK_IDENTIFIER && beg_char == '_')
    kind = cur_char == '_' ? TokenKind::TK_STATIC_FIELD : TokenKind::TK_FIELD;
  return make_token(kind, literal);
}

Token Lexer::make_numeric(void) {
  while (std::isdigit(peek()))
    advance();
  if (peek() == '.' && std::isdigit(peek_next())) {
    advance();
    while (std::isdigit(peek()))
      advance();
  }

  return make_token(TokenKind::TK_NUMERIC);
}

Token Lexer::make_string(void) {
  str_t literal;
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
    return make_error("unterminated string");

  advance(); // closing "
  return make_token(TokenKind::TK_STRING, literal);
}

}
