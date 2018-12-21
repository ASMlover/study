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
#include <cstdlib>
#include <iostream>
#include "scanner.h"

Scanner::Scanner(const std::string& source)
  : source_bytes_(source) {
}

std::vector<Token> Scanner::scan_tokens(void) {
  while (!is_eof()) {
    start_ = current_;
    scan_token();
  }

  Token tok;
  tok.set_kind(TokenKind::ENDF);
  tok.set_line(line_);
  tokens_.push_back(tok);

  return tokens_;
}

std::string Scanner::current_lexeme(std::size_t begpos, std::size_t endpos) {
  return source_bytes_.substr(begpos, endpos - begpos);
}

bool Scanner::is_eof(void) const {
  return current_ >= source_bytes_.size();
}

bool Scanner::is_alpha(char c) const {
  return std::isalpha(c) || c == '_';
}

bool Scanner::is_alnum(char c) const {
  return std::isalnum(c) || c == '_';
}

char Scanner::advance(void) {
  return source_bytes_[current_++];
}

bool Scanner::match(char expected) {
  if (is_eof())
    return false;
  if (source_bytes_[current_] != expected)
    return false;

  ++current_;
  return true;
}

char Scanner::peek(void) const {
  if (current_ >= source_bytes_.size())
    return 0;
  return source_bytes_[current_];
}

char Scanner::peek_next(void) const {
  if (current_ + 1 >= source_bytes_.size())
    return 0;
  return source_bytes_[current_ + 1];
}

void Scanner::add_token(TokenKind k) {
  Token tok;
  tok.set_kind(k);
  tok.set_line(line_);
  tok.set_lexeme(current_lexeme(start_, current_));

  tokens_.push_back(tok);
}

void Scanner::scan_token(void) {
  char c = advance();
  switch (c) {
  case '[': add_token(TokenKind::LSQUARE); break;
  case ']': add_token(TokenKind::RSQUARE); break;
  case '(': add_token(TokenKind::LPAREN); break;
  case ')': add_token(TokenKind::RPAREN); break;
  case '{': add_token(TokenKind::LBRACE); break;
  case '}': add_token(TokenKind::RBRACE); break;
  case '.': resolve_dot_start(); break;
  case '&':
    add_token(match('=') ?
        TokenKind::AMPEQUAL :
        (match('&') ? TokenKind::AMPAMP : TokenKind::AMP));
    break;
  case '*':
    add_token(match('=') ? TokenKind::STAREQUAL : TokenKind::STAR); break;
  case '+':
    add_token(match('=') ?
        TokenKind::PLUSEQUAL :
        (match('+') ? TokenKind::PLUSPLUS : TokenKind::PLUS));
    break;
  case '-':
    if (match('=')) add_token(TokenKind::MINUSEQUAL);
    else if (match('-')) add_token(TokenKind::MINUSMINUS);
    else if (match('>')) add_token(TokenKind::ARROW);
    else add_token(TokenKind::MINUS);
    break;
  case '~': add_token(TokenKind::TILDE); break;
  case '!':
    add_token(match('=') ? TokenKind::EXCLAIMEQUAL : TokenKind::EXCLAIM); break;
  case '/': resolve_slash(); break;
  case '%':
    add_token(match('=') ? TokenKind::PERCENTEQUAL : TokenKind::PERCENT); break;
  case '<': resolve_less_start(); break;
  case '>': resolve_greater_start(); break;
  case '^':
    add_token(match('=') ? TokenKind::CARETEQUAL : TokenKind::CARET); break;
  case '|':
    add_token(match('=') ?
        TokenKind::PIPEEQUAL :
        (match('|') ? TokenKind::PIPEPIPE : TokenKind::PIPE));
    break;
  case '?': add_token(TokenKind::QUESTION); break;
  case ':': add_token(TokenKind::COLON); break;
  case ';': add_token(TokenKind::SEMI); break;
  case '=':
    add_token(match('=') ? TokenKind::EQUALEQUAL : TokenKind::EQUAL); break;
  case ',': add_token(TokenKind::COMMA); break;
  case '#': resolve_macro(); break;
  case ' ':
  case '\t':
  case '\r':
    break;
  case '\n':
    ++line_;
    break;
  case '\'': resolve_char(); break;
  case '"': resolve_string(); break;
  default:
    if (std::isdigit(c)) {
      resolve_number();
    }
    else if (is_alpha(c)) {
      resolve_identifier();
    }
    else {
      std::cerr << "unexpected character: " << c << std::endl;
      std::abort();
    }
  }
}

void Scanner::resolve_slash(void) {
  if (match('=')) {
    add_token(TokenKind::SLASHEQUAL);
  }
  else if (match('/')) {
    while (!is_eof() && peek() != '\n')
      advance();
  }
  else if (match('*')) {
    while (!is_eof() && !(peek() == '*' && peek_next() == '/')) {
      if (peek() == '\n')
        ++line_;
      advance();
    }

    if (peek() == '*' && peek_next() == '/') {
      advance();
    }
    else {
      std::cerr << "invalid block comments" << std::endl;
      std::abort();
    }
  }
  else {
    add_token(TokenKind::SLASH);
  }
}

void Scanner::resolve_char(bool wchar) {
  if (peek_next() == '\'') {
    advance();

    Token tok;
    tok.set_kind(wchar ? TokenKind::WCHARCONST : TokenKind::CHARCONST);
    tok.set_lexeme(current_lexeme(start_ + 1, current_ - 1));
    tok.set_line(line_);

    tokens_.push_back(tok);
  }
  else {
    std::cerr << "invalid char token ..." << std::endl;
    std::abort();
  }
}

void Scanner::resolve_string(bool wstr) {
  while (!is_eof() && peek() != '"') {
    if (peek() == '\n')
      ++line_;
    advance();
  }

  if (is_eof()) {
    std::cerr << "invalid string at line: " << line_ << std::endl;
    std::abort();
  }
  advance();

  Token tok;
  tok.set_kind(wstr ? TokenKind::WSTRINGLITERAL : TokenKind::STRINGLITERAL);
  tok.set_lexeme(current_lexeme(start_ + 1, current_ - 1));
  tok.set_line(line_);

  tokens_.push_back(tok);
}

void Scanner::resolve_number(bool real) {
  auto floating_fn = [this]{
    if (peek() == 'f' || peek() == 'F') {
      advance();
      add_token(TokenKind::DOUBLECONST);
    }
    else if (peek() == 'l' || peek() == 'L') {
      advance();
      add_token(TokenKind::LDOUBLECONST);
    }
    add_token(TokenKind::FLOATCONST);
  };

  while (std::isdigit(peek()))
    advance();

  if (peek() == '.') {
    if (real) {
      std::cerr << "invalid floating-point number ..." << std::endl;
      std::abort();
    }

    if (std::isdigit(peek_next())) {
      advance();
      while (std::isdigit(peek()))
        advance();
    }

    floating_fn();
    return;
  }

  if (real) {
    floating_fn();
  }
  else {
    if (peek() == 'u' || peek() == 'U') {
      advance();
      if (peek() == 'l' || peek() == 'L') {
        advance();
        if (peek() == 'l' || peek() == 'L') {
          advance();
          add_token(TokenKind::ULLONGCONST);
        }
        else {
          add_token(TokenKind::ULONGCONST);
        }
      }
      else {
        add_token(TokenKind::UINTCONST);
      }
    }
    else if (peek() == 'l' || peek() == 'L') {
      advance();
      if (peek() == 'l' || peek() == 'L') {
        advance();
        add_token(TokenKind::LLONGCONST);
      }
      else {
        add_token(TokenKind::LONGCONST);
      }
    }
    else {
      add_token(TokenKind::INTCONST);
    }
  }
}

void Scanner::resolve_macro(void) {
  if (peek() == '#') {
    if (peek_next() == '#') {
      std::cerr << "invalid macro joint ..." << std::endl;
      std::abort();
    }
    else {
      advance();
      add_token(TokenKind::HASHHASH);
    }
  }
  else {
    while (peek() == ' ' || peek() == '\t')
      advance();

    if (is_alpha(peek())) {
      start_ = current_;

      advance();
      while (is_alnum(peek()))
        advance();

      auto lexeme = (std::string("#") + current_lexeme(start_, current_));
      auto kind = get_ppkeyword_kind(lexeme.c_str());
      if (kind != TokenKind::UNKNOWN) {
        Token tok;
        tok.set_kind(kind);
        tok.set_lexeme(lexeme);
        tok.set_line(line_);

        tokens_.push_back(tok);
      }
      else {
        std::cerr << "invlaid macro identifier ..." << lexeme << std::endl;
        std::abort();
      }
    }
  }
}

void Scanner::resolve_identifier(char begchar) {
  while (is_alnum(peek()))
    advance();

  if (begchar == 'L') {
    if (peek() == '\'') {
      advance();
      resolve_char(true);
      return;
    }
    else if (peek() == '"') {
      advance();
      resolve_string(true);
      return;
    }
  }

  auto lexeme = current_lexeme(start_, current_);
  auto kind = get_keyword_kind(lexeme.c_str());
  add_token(kind);
}

void Scanner::resolve_dot_start(void) {
  if (std::isdigit(peek())) {
    resolve_number(true);
    return;
  }

  if (peek() == '.') {
    if (peek_next() == '.') {
      add_token(TokenKind::ELLIPSIS);
    }
    else {
      std::cerr << "unexpected character: " << peek_next() << std::endl;
    }
  }
  else {
    add_token(TokenKind::PERIOD);
  }
}

void Scanner::resolve_greater_start(void) {
  if (match('='))
    add_token(TokenKind::GREATEREQUAL);
  else if (match('>'))
    add_token(match('=') ?
        TokenKind::GREATERGREATEREQUAL : TokenKind::GREATERGREATER);
  else
    add_token(TokenKind::GREATER);
}

void Scanner::resolve_less_start(void) {
  if (match('='))
    add_token(TokenKind::LESSEQUAL);
  else if (match('<'))
    add_token(match('=') ? TokenKind::LESSLESSEQUAL : TokenKind::LESSLESS);
  else
    add_token(TokenKind::LESS);
}
