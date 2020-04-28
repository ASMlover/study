#include <Tadpole/Lexer.hh>

namespace _mevo::tadpole {

Token Lexer::next_token() {
  skip_whitespace();

  begpos_ = curpos_;
  if (is_end())
    return make_token(TokenKind::TK_EOF);

  char c = advance();
  if (is_alpha(c))
    return make_identifier();
  if (is_digit(c))
    return make_numeric();

  switch (c) {
  case '(': return make_token(TokenKind::TK_LPAREN);
  case ')': return make_token(TokenKind::TK_RPAREN);
  case '{': return make_token(TokenKind::TK_LBRACE);
  case '}': return make_token(TokenKind::TK_RBRACE);
  case ',': return make_token(TokenKind::TK_COMMA);
  case '-': return make_token(TokenKind::TK_MINUS);
  case '+': return make_token(TokenKind::TK_PLUS);
  case ';': return make_token(TokenKind::TK_SEMI);
  case '/': return make_token(TokenKind::TK_SLASH);
  case '*': return make_token(TokenKind::TK_STAR);
  case '=': return make_token(TokenKind::TK_EQ);
  case '"': return make_string();
  }
  return make_error("unexpected charactor");
}

void Lexer::skip_whitespace() {
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

Token Lexer::make_token(TokenKind kind, const str_t& literal) {
  return Token(kind, literal, lineno_);
}

Token Lexer::make_token(TokenKind kind) {
  return Token(kind, gen_literal(begpos_, curpos_), lineno_);
}

Token Lexer::make_error(const str_t& err_msg) {
  return Token(TokenKind::TK_ERR, err_msg, lineno_);
}

Token Lexer::make_identifier() {
  while (is_alnum(peek()))
    advance();

  str_t literal = gen_literal(begpos_, curpos_);
  return make_token(get_keyword_kind(literal), literal);
}

Token Lexer::make_numeric() {
  while (is_digit(peek()))
    advance();
  if (peek() == '.' && is_digit(peek(1))) {
    advance();
    while (is_digit(peek()))
      advance();
  }

  return make_token(TokenKind::TK_NUMERIC);
}

Token Lexer::make_string() {
  return Token{};
}

}