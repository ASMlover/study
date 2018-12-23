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

void Scanner::report_error(const std::string& err) {
  std::cerr << "at " << line_ << ": " << err << std::endl;
  std::abort();
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
    if (std::isdigit(c))
      resolve_numeric(c);
    else if (is_alpha(c))
      resolve_identifier();
    else
      report_error("unexpected character ...");
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

    if (peek() == '*' && peek_next() == '/')
      advance();
    else
      report_error("invalid block comments ...");
  }
  else {
    add_token(TokenKind::SLASH);
  }
}

void Scanner::resolve_char(TokenKind kind) {
  if (peek_next() == '\'') {
    advance();

    Token tok;
    tok.set_kind(kind);
    tok.set_lexeme(current_lexeme(start_ + 1, current_ - 1));
    tok.set_line(line_);

    tokens_.push_back(tok);
  }
  else {
    report_error("invalid char token ...");
  }
}

void Scanner::resolve_string(TokenKind kind, char separator) {
  while (!is_eof() && peek() != separator) {
    if (peek() == '\n')
      ++line_;
    advance();
  }

  if (is_eof())
    report_error("invalid string token ...");
  advance();

  Token tok;
  auto lexeme = current_lexeme(start_ + 1, current_ - 1);
  if (separator == '>') {
    tok.set_kind(TokenKind::ANGLESTRINGLITERAL);
  }
  else {
    if (kind == TokenKind::STRINGLITERAL && !tokens_.empty() &&
        tokens_.back().get_kind() == TokenKind::STRINGLITERAL) {
      auto& back = tokens_.back();
      back.set_lexeme(back.get_lexeme() + lexeme);
      back.set_line(line_);
      return;
    }
    tok.set_kind(kind);
  }
  tok.set_lexeme(lexeme);
  tok.set_line(line_);
  tokens_.push_back(tok);
}

void Scanner::resolve_numeric(char c, bool is_float) {
  auto floating_fn = [this]{
    if (peek() == 'f' || peek() == 'F') {
      advance();
      add_token(TokenKind::DOUBLECONST);
    }
    else if (peek() == 'l' || peek() == 'L') {
      advance();
      add_token(TokenKind::LDOUBLECONST);
    }
    else if (peek() == 'e' || peek() == 'L') {
    }
    else if (peek() == 'p' || peek() == 'P') {
    }
    else {
      report_error("invalid floating numeric ...");
    }
    add_token(TokenKind::FLOATCONST);
  };

  if (c == '0') {
    // solve hex or 8bit
    return;
  }

  while (std::isdigit(peek()))
    advance();

  if (peek() == '.') {
    if (is_float)
      report_error("invalid floating-point numeric ...");

    if (std::isdigit(peek_next())) {
      advance();
      while (std::isdigit(peek()))
        advance();
    }

    floating_fn();
    return;
  }

  if (is_float) {
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
    else if (peek() == 'e' || peek() == 'E') {
      // TODO:
    }
    else {
      if (is_alnum(peek()))
        report_error("invalid numeric ...");
      else
        add_token(TokenKind::INTCONST);
    }
  }
}

void Scanner::resolve_macro(void) {
  if (peek() == '#') {
    if (peek_next() == '#') {
      report_error("invalid macro hashhash ...");
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
        report_error("invalid macro PPKEYWORD ...");
      }
    }
  }
}

void Scanner::resolve_identifier(char begchar) {
  auto special_char = [this](char c, TokenKind char_kind, TokenKind str_kind) {
    if (c == '\'') {
      advance();
      resolve_char(char_kind);
    }
    else if (c == '"') {
      advance();
      resolve_char(str_kind);
    }
  };

  if (begchar == 'L' || begchar == 'u' || begchar == 'U') {
    switch (begchar) {
    case 'L':
      special_char(peek(), TokenKind::WCHARCONST, TokenKind::WSTRINGLITERAL);
      break;
    case 'u':
      if (peek() == '8') {
        char c = peek_next();
        if (c == '\'' || c == '"') {
          advance();
          special_char(c, TokenKind::U8CHARCONST, TokenKind::U8STRINGLITERAL);
        }
      }
      else {
        special_char(
            peek(), TokenKind::U16CHARCONST, TokenKind::U16STRINGLITERAL);
      }
      break;
    case 'U':
      special_char(
          peek(), TokenKind::U32CHARCONST, TokenKind::U32STRINGLITERAL);
      break;
    }

    return;
  }

  while (is_alnum(peek()))
    advance();

  auto lexeme = current_lexeme(start_, current_);
  auto kind = get_keyword_kind(lexeme.c_str());
  add_token(kind);
}

void Scanner::resolve_dot_start(void) {
  if (std::isdigit(peek())) {
    resolve_numeric('.', true);
    return;
  }

  if (peek() == '.') {
    if (peek_next() == '.')
      add_token(TokenKind::ELLIPSIS);
    else
      report_error("unexpected character ...");
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
  if (match('=')) {
    add_token(TokenKind::LESSEQUAL);
  }
  else if (match('<')) {
    add_token(match('=') ? TokenKind::LESSLESSEQUAL : TokenKind::LESSLESS);
  }
  else {
    if (!tokens_.empty() && tokens_.back().get_kind() == TokenKind::PP_INCLUDE)
      resolve_string(TokenKind::STRINGLITERAL, '>');
    else
      add_token(TokenKind::LESS);
  }
}
