#pragma once

#include "ms/common.hh"

namespace ms {

enum class TokenType {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,
    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string_view start;
    int length;
    int line;
};

class Scanner {
public:
    void initSource(std::string_view source);
    Token scanToken();

private:
    std::string_view source;
    size_t start;
    size_t current;
    int line;

    void skipWhitespace();
    Token identifier();
    Token number();
    Token string();
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    Token makeToken(TokenType type) const;
    Token errorToken(std::string_view message) const;
    TokenType identifierType() const;
};

}
