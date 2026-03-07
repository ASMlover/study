// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include <cstring>
#include "Scanner.hh"

namespace ms {

namespace {

inline bool is_digit(char c) noexcept {
  return c >= '0' && c <= '9';
}

inline bool is_alpha(char c) noexcept {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         c == '_';
}

} // anonymous namespace

Scanner::Scanner(strv_t source) noexcept {
  init(source);
}

void Scanner::init(strv_t source) noexcept {
  start_ = source.data();
  current_ = source.data();
  line_ = 1;
}

bool Scanner::is_at_end() const noexcept {
  return *current_ == '\0';
}

char Scanner::advance() noexcept {
  return *current_++;
}

char Scanner::peek() const noexcept {
  return *current_;
}

char Scanner::peek_next() const noexcept {
  if (is_at_end()) return '\0';
  return current_[1];
}

bool Scanner::match(char expected) noexcept {
  if (is_at_end()) return false;
  if (*current_ != expected) return false;
  current_++;
  return true;
}

Token Scanner::make_token(TokenType type) const noexcept {
  Token token;
  token.type = type;
  token.lexeme = strv_t(start_, static_cast<sz_t>(current_ - start_));
  token.line = line_;
  return token;
}

Token Scanner::error_token(cstr_t message) const noexcept {
  Token token;
  token.type = TokenType::TOKEN_ERROR;
  token.lexeme = strv_t(message);
  token.line = line_;
  return token;
}

void Scanner::skip_whitespace() noexcept {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line_++;
      advance();
      break;
    case '/':
      if (peek_next() == '/') {
        while (peek() != '\n' && !is_at_end()) advance();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Token Scanner::scan_string() noexcept {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n') line_++;
    advance();
  }

  if (is_at_end()) return error_token("Unterminated string.");

  advance(); // closing quote
  return make_token(TokenType::TOKEN_STRING);
}

Token Scanner::scan_number() noexcept {
  while (is_digit(peek())) advance();

  if (peek() == '.' && is_digit(peek_next())) {
    advance(); // consume '.'
    while (is_digit(peek())) advance();
  }

  return make_token(TokenType::TOKEN_NUMBER);
}

TokenType Scanner::check_keyword(int start, int length, cstr_t rest, TokenType type) const noexcept {
  if (current_ - start_ == start + length &&
      std::memcmp(start_ + start, rest, static_cast<sz_t>(length)) == 0) {
    return type;
  }
  return TokenType::TOKEN_IDENTIFIER;
}

TokenType Scanner::identifier_type() const noexcept {
  switch (start_[0]) {
  case 'a':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'n': return check_keyword(2, 1, "d", TokenType::TOKEN_AND);
      case 's': return check_keyword(2, 0, "", TokenType::TOKEN_AS);
      }
    }
    break;
  case 'b': return check_keyword(1, 4, "reak", TokenType::TOKEN_BREAK);
  case 'c':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'l': return check_keyword(2, 3, "ass", TokenType::TOKEN_CLASS);
      case 'o': return check_keyword(2, 6, "ntinue", TokenType::TOKEN_CONTINUE);
      }
    }
    break;
  case 'e': return check_keyword(1, 3, "lse", TokenType::TOKEN_ELSE);
  case 'f':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'a': return check_keyword(2, 3, "lse", TokenType::TOKEN_FALSE);
      case 'o': return check_keyword(2, 1, "r", TokenType::TOKEN_FOR);
      case 'r': return check_keyword(2, 2, "om", TokenType::TOKEN_FROM);
      case 'u': return check_keyword(2, 1, "n", TokenType::TOKEN_FUN);
      }
    }
    break;
  case 'i':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'f': return check_keyword(2, 0, "", TokenType::TOKEN_IF);
      case 'm': return check_keyword(2, 4, "port", TokenType::TOKEN_IMPORT);
      }
    }
    break;
  case 'n': return check_keyword(1, 2, "il", TokenType::TOKEN_NIL);
  case 'o': return check_keyword(1, 1, "r", TokenType::TOKEN_OR);
  case 'p': return check_keyword(1, 4, "rint", TokenType::TOKEN_PRINT);
  case 'r': return check_keyword(1, 5, "eturn", TokenType::TOKEN_RETURN);
  case 's': return check_keyword(1, 4, "uper", TokenType::TOKEN_SUPER);
  case 't':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'h': return check_keyword(2, 2, "is", TokenType::TOKEN_THIS);
      case 'r': return check_keyword(2, 2, "ue", TokenType::TOKEN_TRUE);
      }
    }
    break;
  case 'v': return check_keyword(1, 2, "ar", TokenType::TOKEN_VAR);
  case 'w': return check_keyword(1, 4, "hile", TokenType::TOKEN_WHILE);
  }

  return TokenType::TOKEN_IDENTIFIER;
}

Token Scanner::scan_identifier() noexcept {
  while (is_alpha(peek()) || is_digit(peek())) advance();
  return make_token(identifier_type());
}

Token Scanner::scan_token() noexcept {
  skip_whitespace();
  start_ = current_;

  if (is_at_end()) return make_token(TokenType::TOKEN_EOF);

  char c = advance();

  if (is_alpha(c)) return scan_identifier();
  if (is_digit(c)) return scan_number();

  switch (c) {
  case '(': return make_token(TokenType::TOKEN_LEFT_PAREN);
  case ')': return make_token(TokenType::TOKEN_RIGHT_PAREN);
  case '{': return make_token(TokenType::TOKEN_LEFT_BRACE);
  case '}': return make_token(TokenType::TOKEN_RIGHT_BRACE);
  case ';': return make_token(TokenType::TOKEN_SEMICOLON);
  case ',': return make_token(TokenType::TOKEN_COMMA);
  case '.': return make_token(TokenType::TOKEN_DOT);
  case '-': return make_token(TokenType::TOKEN_MINUS);
  case '+': return make_token(TokenType::TOKEN_PLUS);
  case '/': return make_token(TokenType::TOKEN_SLASH);
  case '*': return make_token(TokenType::TOKEN_STAR);
  case '%': return make_token(TokenType::TOKEN_PERCENT);
  case ':': return make_token(TokenType::TOKEN_COLON);
  case '!': return make_token(match('=') ? TokenType::TOKEN_BANG_EQUAL : TokenType::TOKEN_BANG);
  case '=': return make_token(match('=') ? TokenType::TOKEN_EQUAL_EQUAL : TokenType::TOKEN_EQUAL);
  case '<': return make_token(match('=') ? TokenType::TOKEN_LESS_EQUAL : TokenType::TOKEN_LESS);
  case '>': return make_token(match('=') ? TokenType::TOKEN_GREATER_EQUAL : TokenType::TOKEN_GREATER);
  case '"': return scan_string();
  }

  return error_token("Unexpected character.");
}

} // namespace ms
