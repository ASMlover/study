// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <iostream>
#include <sstream>
#include "scanner.h"

double Scanner::str2number(const std::string& s) {
  double r{};
  std::stringstream ss(s);
  ss >> r;
  return r;
}

bool Scanner::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Scanner::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

std::string Scanner::get_lexeme(
    const std::string& s, std::size_t beg, std::size_t end) {
  return s.substr(beg, end - beg);
}

bool Scanner::is_at_end(void) const {
  return current_ >= source_.size();
}

char Scanner::advance(void) {
  return source_[current_++];
}

bool Scanner::match(char expected) {
  if (is_at_end())
    return false;
  if (source_[current_] != expected)
    return false;

  ++current_;
  return true;
}

char Scanner::peek(void) const {
  return source_[current_];
}

char Scanner::peek_next(void) const {
  if (current_ + 1 >= source_.size())
    return 0;
  return source_[current_ + 1];
}

void Scanner::add_string(void) {
  std::string literal;
  while (peek() != '"' && !is_at_end()) {
    char c = peek();
    switch (c) {
    case '\n': ++line_; break;
    case '\\':
      switch (peek_next()) {
      case '"': c = '"';  advance(); break;
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
  if (is_at_end()) {
    std::cerr << "unterminated string: " << line_ << std::endl;
    return;
  }

  // the closing "
  advance();

  // trim the surround quotes
  tokens_.push_back(Token(TOKEN_STRING, literal, line_));
}

void Scanner::add_number(void) {
  while (std::isdigit(peek()))
    advance();

  // look for a fractional part
  if (peek() == '.' && std::isdigit(peek_next())) {
    // consume the `.`
    advance();

    while (std::isdigit(peek()))
      advance();
  }

  add_token(TOKEN_NUMBER);
}

void Scanner::add_identifier(void) {
  while (is_alnum(peek()))
    advance();

  // see if the identifier is a reserved word
  auto type = keywords_.find(get_lexeme(source_, start_, current_));
  if (type == keywords_.end())
    add_token(TOKEN_IDENTIFIER);
  else
    add_token(type->second);
}

void Scanner::skip_comments(void) {
  // a comment goes until the end of the line
  while (!is_at_end() && peek() != '\n')
    advance();
}

void Scanner::add_token(TokenType type) {
  auto lexeme = get_lexeme(source_, start_, current_);
  tokens_.push_back(Token(type, lexeme, line_));
}

void Scanner::scan_token(void) {
  char c = advance();
  switch (c) {
  case '(': add_token(TOKEN_LEFT_PAREN); break;
  case ')': add_token(TOKEN_RIGHT_PAREN); break;
  case '{': add_token(TOKEN_LEFT_BRACE); break;
  case '}': add_token(TOKEN_RIGHT_BRACE); break;
  case ',': add_token(TOKEN_COMMA); break;
  case ';': add_token(TOKEN_SEMICOLON); break;
  case '.': add_token(TOKEN_DOT); break;
  case '+': add_token(TOKEN_PLUS); break;
  case '-': add_token(TOKEN_MINUS); break;
  case '*': add_token(TOKEN_STAR); break;
  case '!': add_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG); break;
  case '=': add_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL); break;
  case '<': add_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS); break;
  case '>': add_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER); break;
  case '/':
    if (match('/'))
      skip_comments();
    else
      add_token(TOKEN_SLASH);
    break;
  case ' ':
  case '\r':
  case '\t':
    // ignore whitespace
    break;
  case '\n': ++line_; break;
  case '"': add_string(); break;
  case '#': skip_comments(); break;
  default:
    if (std::isdigit(c))
      add_number();
    else if (is_alpha(c))
      add_identifier();
    else
      std::cerr << "unexpected character, at: " << line_ << std::endl;
    break;
  }
}

Scanner::Scanner(const std::string& source)
  : source_(source) {
  keywords_["and"] = TOKEN_AND;
  keywords_["class"] = TOKEN_CLASS;
  keywords_["else"] = TOKEN_ELSE;
  keywords_["false"] = TOKEN_FALSE;
  keywords_["fun"] = TOKEN_FUN;
  keywords_["for"] = TOKEN_FOR;
  keywords_["if"] = TOKEN_IF;
  keywords_["nil"] = TOKEN_NIL;
  keywords_["not"] = TOKEN_NOT;
  keywords_["or"] = TOKEN_OR;
  keywords_["print"] = TOKEN_PRINT;
  keywords_["return"] = TOKEN_RETURN;
  keywords_["super"] = TOKEN_SUPER;
  keywords_["this"] = TOKEN_THIS;
  keywords_["true"] = TOKEN_TRUE;
  keywords_["var"] = TOKEN_VAR;
  keywords_["while"] = TOKEN_WHILE;
}

std::vector<Token> Scanner::scan_tokens(void) {
  while (!is_at_end()) {
    // TODO:
    start_ = current_;
    scan_token();
  }

  tokens_.push_back(Token(TOKEN_EOF, "", line_));
  return tokens_;
}
