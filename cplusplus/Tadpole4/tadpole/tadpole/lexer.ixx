module;
#include <cctype>
#include "common.hh"
#include "token.hh"
import common;
import token;

export module lexer;

export namespace tadpole {

class Lexer final : private UnCopyable {
  const str_t& source_bytes_;
  sz_t begpos_{};
  sz_t curpos_{};
  int lineno_{1};

  inline bool is_alpha(char c) const noexcept { return std::isalpha(c) || c == '_'; }
  inline bool is_alnum(char c) const noexcept { return std::isalnum(c) || c == '_'; }
  inline bool is_digit(char c) const noexcept { return std::isdigit(c); }
  inline bool is_tail() const noexcept { return curpos_ >= source_bytes_.size(); }
  inline char advance() noexcept { return source_bytes_[curpos_++]; }

  inline str_t gen_literal(sz_t begpos, sz_t endpos) const noexcept {
    return source_bytes_.substr(begpos, endpos - begpos);
  }

  inline char peek(sz_t distance = 0) const noexcept {
    return curpos_ + distance >= source_bytes_.size() ? 0 : source_bytes_[curpos_ + distance];
  }

  inline bool match(char expected) noexcept {
    if (source_bytes_[curpos_] == expected) {
      advance();
      return true;
    }
    return false;
  }

  void skip_whitespace() {
    for (;;) {
      char c = peek();
      switch (c) {
      case ' ': case '\r': case '\t': advance(); break;
      case '\n': ++lineno_; advance(); break;
      case '/':
        if (peek(1) == '/') {
          while (peek() != '\n')
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

  Token make_token(TokenKind kind) {
    return Token(kind, gen_literal(begpos_, curpos_), lineno_);
  }

  Token make_token(TokenKind kind, const str_t& literal) {
    return Token(kind, literal, lineno_);
  }

  Token make_error(const str_t& message) {
    return Token(TokenKind::TK_ERR, message, lineno_);
  }

  Token make_identifier() {
    while (is_alnum(peek()))
      advance();

    str_t literal = gen_literal(begpos_, curpos_);
    return make_token(get_keyword_kind(literal), literal);
  }

  Token make_numeric() {
    while (is_digit(peek()))
      advance();
    if (peek() == '.' && is_digit(peek(1))) {
      advance();
      while (is_digit(peek()))
        advance();
    }
    return make_token(TokenKind::TK_NUMERIC);
  }

  Token make_string() {
#define _MAKE_CHAR(x, y) case x: c = y; advance(); break

    str_t literal;
    while (!is_tail() && peek() != '"') {
      char c = peek();
      switch (c) {
      case '\n': ++lineno_; break;
      case '\\':
        switch (peek()) {
        _MAKE_CHAR('"', '"');
        _MAKE_CHAR('\\', '\\');
        _MAKE_CHAR('%', '%');
        _MAKE_CHAR('0', '\0');
        _MAKE_CHAR('a', '\a');
        _MAKE_CHAR('b', '\b');
        _MAKE_CHAR('f', '\f');
        _MAKE_CHAR('n', '\n');
        _MAKE_CHAR('r', '\r');
        _MAKE_CHAR('t', '\t');
        _MAKE_CHAR('v', '\v');
        }
        break;
      }
      literal.push_back(c);
      advance();
    }
#undef _MAKE_CHAR

    if (is_tail())
      return make_error("unterminated string");

    advance();
    return make_token(TokenKind::TK_STRING, literal);
  }
public:
  Lexer(const str_t& source_bytes) noexcept : source_bytes_(source_bytes) {}

  Token next_token() {
    skip_whitespace();

    begpos_ = curpos_;
    if (is_tail())
      return make_token(TokenKind::TK_EOF);

    char c = advance();
    if (is_alpha(c))
      return make_identifier();
    if (is_digit(c))
      return make_numeric();

#define _MAKE_TOKEN(c, k) case c: return make_token(TokenKind::TK_##k)
    switch (c) {
    _MAKE_TOKEN('(', LPAREN);
    _MAKE_TOKEN(')', RPAREN);
    _MAKE_TOKEN('{', LBRACE);
    _MAKE_TOKEN('}', RBRACE);
    _MAKE_TOKEN(',', COMMA);
    _MAKE_TOKEN('-', MINUS);
    _MAKE_TOKEN('+', PLUS);
    _MAKE_TOKEN(';', SEMI);
    _MAKE_TOKEN('/', SLASH);
    _MAKE_TOKEN('*', STAR);
    _MAKE_TOKEN('=', EQ);
    case '"': return make_string();
    }
#undef _MAKE_TOKEN

    return make_error(from_fmt("unexpected character `%c`", c));
  }
};

}