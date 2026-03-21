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
  line_start_ = source.data();
  line_ = 1;
  interp_count_ = 0;
  paren_suppress_depth_ = 0;
  suppress_next_asi_ = false;
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
  token.column = static_cast<int>(start_ - line_start_) + 1;
  return token;
}

Token Scanner::error_token(cstr_t message) const noexcept {
  Token token;
  token.type = TokenType::TOKEN_ERROR;
  token.lexeme = strv_t(message);
  token.line = line_;
  token.column = static_cast<int>(current_ - line_start_) + 1;
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
      if (is_asi_trigger(prev_type_) && paren_suppress_depth_ == 0 && !suppress_next_asi_)
        pending_asi_ = true;
      line_++;
      advance();
      line_start_ = current_;
      break;
    case '/':
      if (peek_next() == '/') {
        while (peek() != '\n' && !is_at_end()) advance();
      } else if (peek_next() == '*') {
        advance(); // skip '/'
        advance(); // skip '*'
        int depth = 1;
        while (depth > 0 && !is_at_end()) {
          if (peek() == '/' && peek_next() == '*') {
            advance(); advance();
            depth++;
          } else if (peek() == '*' && peek_next() == '/') {
            advance(); advance();
            depth--;
          } else {
            if (peek() == '\n') { line_++; advance(); line_start_ = current_; continue; }
            advance();
          }
        }
      } else {
        return;
      }
      break;
    case '\\':
      if (peek_next() == '\n') {
        advance(); // skip '\'
        advance(); // skip '\n'
        line_++;
        line_start_ = current_;
        break;
      } else if (peek_next() == '\r') {
        advance(); // skip '\'
        advance(); // skip '\r'
        if (peek() == '\n') advance(); // skip '\n'
        line_++;
        line_start_ = current_;
        break;
      }
      return;
    default:
      return;
    }
  }
}

Token Scanner::scan_string() noexcept {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\\') {
      advance(); // skip the backslash
      if (peek() == '\n') { line_++; advance(); line_start_ = current_; }
      else if (!is_at_end()) advance(); // skip the escaped character
      continue;
    }
    // Check for string interpolation: ${
    if (peek() == '$' && peek_next() == '{') {
      Token token = make_token(TokenType::TOKEN_STRING_INTERP);
      advance(); // skip '$'
      advance(); // skip '{'
      if (interp_count_ >= kMAX_INTERPOLATION_NESTING) {
        return error_token("String interpolation nesting too deep.");
      }
      interp_braces_[interp_count_] = 1;
      interp_count_++;
      return token;
    }
    if (peek() == '\n') { line_++; advance(); line_start_ = current_; continue; }
    advance();
  }

  if (is_at_end()) return error_token("Unterminated string.");

  advance(); // closing quote
  return make_token(TokenType::TOKEN_STRING);
}

Token Scanner::scan_string_continuation() noexcept {
  start_ = current_;

  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\\') {
      advance();
      if (peek() == '\n') { line_++; advance(); line_start_ = current_; }
      else if (!is_at_end()) advance();
      continue;
    }
    if (peek() == '$' && peek_next() == '{') {
      Token token = make_token(TokenType::TOKEN_STRING_INTERP);
      advance(); // skip '$'
      advance(); // skip '{'
      if (interp_count_ >= kMAX_INTERPOLATION_NESTING) {
        return error_token("String interpolation nesting too deep.");
      }
      interp_braces_[interp_count_] = 1;
      interp_count_++;
      return token;
    }
    if (peek() == '\n') { line_++; advance(); line_start_ = current_; continue; }
    advance();
  }

  if (is_at_end()) return error_token("Unterminated string.");

  // current_ is at '"', make token BEFORE advancing past it
  Token token = make_token(TokenType::TOKEN_STRING);
  advance(); // skip closing '"'
  return token;
}

Token Scanner::scan_number() noexcept {
  while (is_digit(peek())) advance();

  if (peek() == '.' && is_digit(peek_next())) {
    advance(); // consume '.'
    while (is_digit(peek())) advance();
    return make_token(TokenType::TOKEN_NUMBER);
  }

  return make_token(TokenType::TOKEN_INTEGER);
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
      case 'b': return check_keyword(2, 6, "stract", TokenType::TOKEN_ABSTRACT);
      case 'n': return check_keyword(2, 1, "d", TokenType::TOKEN_AND);
      case 's': return check_keyword(2, 0, "", TokenType::TOKEN_AS);
      }
    }
    break;
  case 'b': return check_keyword(1, 4, "reak", TokenType::TOKEN_BREAK);
  case 'c':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'a':
        if (current_ - start_ > 2) {
          switch (start_[2]) {
          case 's': return check_keyword(3, 1, "e", TokenType::TOKEN_CASE);
          case 't': return check_keyword(3, 2, "ch", TokenType::TOKEN_CATCH);
          }
        }
        break;
      case 'l': return check_keyword(2, 3, "ass", TokenType::TOKEN_CLASS);
      case 'o': return check_keyword(2, 6, "ntinue", TokenType::TOKEN_CONTINUE);
      }
    }
    break;
  case 'd':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'e':
        if (current_ - start_ > 2) {
          switch (start_[2]) {
          case 'f':
            if (current_ - start_ > 3) {
              switch (start_[3]) {
              case 'a': return check_keyword(4, 3, "ult", TokenType::TOKEN_DEFAULT);
              case 'e': return check_keyword(4, 1, "r", TokenType::TOKEN_DEFER);
              }
            }
            break;
          }
        }
        break;
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
      case 'n': return check_keyword(2, 0, "", TokenType::TOKEN_IN);
      case 'm': return check_keyword(2, 4, "port", TokenType::TOKEN_IMPORT);
      }
    }
    break;
  case 'n': return check_keyword(1, 2, "il", TokenType::TOKEN_NIL);
  case 'o': return check_keyword(1, 1, "r", TokenType::TOKEN_OR);
  case 'p': return check_keyword(1, 4, "rint", TokenType::TOKEN_PRINT);
  case 'r': return check_keyword(1, 5, "eturn", TokenType::TOKEN_RETURN);
  case 's':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 't': return check_keyword(2, 4, "atic", TokenType::TOKEN_STATIC);
      case 'u': return check_keyword(2, 3, "per", TokenType::TOKEN_SUPER);
      case 'w': return check_keyword(2, 4, "itch", TokenType::TOKEN_SWITCH);
      }
    }
    break;
  case 't':
    if (current_ - start_ > 1) {
      switch (start_[1]) {
      case 'h':
        if (current_ - start_ > 2) {
          switch (start_[2]) {
          case 'i': return check_keyword(3, 1, "s", TokenType::TOKEN_THIS);
          case 'r': return check_keyword(3, 2, "ow", TokenType::TOKEN_THROW);
          }
        }
        break;
      case 'r':
        if (current_ - start_ > 2) {
          switch (start_[2]) {
          case 'u': return check_keyword(3, 1, "e", TokenType::TOKEN_TRUE);
          case 'y': return check_keyword(3, 0, "", TokenType::TOKEN_TRY);
          }
        }
        break;
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

bool Scanner::is_asi_trigger(TokenType type) noexcept {
  switch (type) {
  // Literals
  case TokenType::TOKEN_NUMBER:
  case TokenType::TOKEN_INTEGER:
  case TokenType::TOKEN_STRING:
  case TokenType::TOKEN_TRUE:
  case TokenType::TOKEN_FALSE:
  case TokenType::TOKEN_NIL:
  // Identifiers
  case TokenType::TOKEN_IDENTIFIER:
  // Terminator keywords
  case TokenType::TOKEN_BREAK:
  case TokenType::TOKEN_CONTINUE:
  case TokenType::TOKEN_RETURN:
  // Closers
  case TokenType::TOKEN_RIGHT_PAREN:
  case TokenType::TOKEN_RIGHT_BRACKET:
  case TokenType::TOKEN_RIGHT_BRACE:
  // Other keywords that end statements
  case TokenType::TOKEN_THIS:
  case TokenType::TOKEN_SUPER:
    return true;
  default:
    return false;
  }
}

ScannerState Scanner::save_state() const noexcept {
  return {start_, current_, line_start_, line_, prev_type_, pending_asi_, paren_suppress_depth_, suppress_next_asi_};
}

void Scanner::restore_state(const ScannerState& state) noexcept {
  start_ = state.start;
  current_ = state.current;
  line_start_ = state.line_start;
  line_ = state.line;
  prev_type_ = state.prev_type;
  pending_asi_ = state.pending_asi;
  paren_suppress_depth_ = state.paren_suppress_depth;
  suppress_next_asi_ = state.suppress_next_asi;
}

Token Scanner::scan_token() noexcept {
  skip_whitespace();
  start_ = current_;

  if (pending_asi_) {
    pending_asi_ = false;
    prev_type_ = TokenType::TOKEN_SEMICOLON;
    return make_token(TokenType::TOKEN_SEMICOLON);
  }

  suppress_next_asi_ = false;

  if (is_at_end()) return make_token(TokenType::TOKEN_EOF);

  char c = advance();

  auto emit = [this](Token token) noexcept -> Token {
    prev_type_ = token.type;
    return token;
  };

  if (is_alpha(c)) return emit(scan_identifier());
  if (is_digit(c)) return emit(scan_number());

  switch (c) {
  case '(':
    if (paren_suppress_depth_ > 0) {
      paren_suppress_depth_++;
    } else if (prev_type_ == TokenType::TOKEN_IF || prev_type_ == TokenType::TOKEN_WHILE ||
               prev_type_ == TokenType::TOKEN_FOR || prev_type_ == TokenType::TOKEN_SWITCH ||
               prev_type_ == TokenType::TOKEN_CATCH) {
      paren_suppress_depth_ = 1;
    }
    return emit(make_token(TokenType::TOKEN_LEFT_PAREN));
  case ')':
    if (paren_suppress_depth_ > 0) {
      paren_suppress_depth_--;
      if (paren_suppress_depth_ == 0)
        suppress_next_asi_ = true;
    }
    return emit(make_token(TokenType::TOKEN_RIGHT_PAREN));
  case '{':
    if (interp_count_ > 0) interp_braces_[interp_count_ - 1]++;
    return emit(make_token(TokenType::TOKEN_LEFT_BRACE));
  case '}':
    if (interp_count_ > 0) {
      interp_braces_[interp_count_ - 1]--;
      if (interp_braces_[interp_count_ - 1] == 0) {
        interp_count_--;
        return emit(scan_string_continuation());
      }
    }
    return emit(make_token(TokenType::TOKEN_RIGHT_BRACE));
  case '[': return emit(make_token(TokenType::TOKEN_LEFT_BRACKET));
  case ']': return emit(make_token(TokenType::TOKEN_RIGHT_BRACKET));
  case ';': return emit(make_token(TokenType::TOKEN_SEMICOLON));
  case ',': return emit(make_token(TokenType::TOKEN_COMMA));
  case '.': return emit(make_token(TokenType::TOKEN_DOT));
  case '-': return emit(make_token(match('=') ? TokenType::TOKEN_MINUS_EQUAL : TokenType::TOKEN_MINUS));
  case '+': return emit(make_token(match('=') ? TokenType::TOKEN_PLUS_EQUAL : TokenType::TOKEN_PLUS));
  case '/': return emit(make_token(match('=') ? TokenType::TOKEN_SLASH_EQUAL : TokenType::TOKEN_SLASH));
  case '*': return emit(make_token(match('=') ? TokenType::TOKEN_STAR_EQUAL : TokenType::TOKEN_STAR));
  case '%': return emit(make_token(match('=') ? TokenType::TOKEN_PERCENT_EQUAL : TokenType::TOKEN_PERCENT));
  case '&': return emit(make_token(TokenType::TOKEN_AMPERSAND));
  case '|': return emit(make_token(TokenType::TOKEN_PIPE));
  case '^': return emit(make_token(TokenType::TOKEN_CARET));
  case '~': return emit(make_token(TokenType::TOKEN_TILDE));
  case '?': return emit(make_token(TokenType::TOKEN_QUESTION));
  case ':': return emit(make_token(TokenType::TOKEN_COLON));
  case '!': return emit(make_token(match('=') ? TokenType::TOKEN_BANG_EQUAL : TokenType::TOKEN_BANG));
  case '=': return emit(make_token(match('=') ? TokenType::TOKEN_EQUAL_EQUAL : TokenType::TOKEN_EQUAL));
  case '<':
    if (match('<')) return emit(make_token(TokenType::TOKEN_LEFT_SHIFT));
    return emit(make_token(match('=') ? TokenType::TOKEN_LESS_EQUAL : TokenType::TOKEN_LESS));
  case '>':
    if (match('>')) return emit(make_token(TokenType::TOKEN_RIGHT_SHIFT));
    return emit(make_token(match('=') ? TokenType::TOKEN_GREATER_EQUAL : TokenType::TOKEN_GREATER));
  case '"': return emit(scan_string());
  }

  return emit(error_token("Unexpected character."));
}

} // namespace ms
