#include "ms/scanner.hh"

namespace ms {

void Scanner::initSource(std::string_view source_) {
    source = source_;
    start = 0;
    current = 0;
    line = 1;
}

Token Scanner::scanToken() {
    skipWhitespace();
    start = current;

    if (peek() == '\0') {
        return makeToken(TokenType::TOKEN_EOF);
    }

    char c = advance();

    if (c == '(') return makeToken(TokenType::TOKEN_LEFT_PAREN);
    if (c == ')') return makeToken(TokenType::TOKEN_RIGHT_PAREN);
    if (c == '{') return makeToken(TokenType::TOKEN_LEFT_BRACE);
    if (c == '}') return makeToken(TokenType::TOKEN_RIGHT_BRACE);
    if (c == ',') return makeToken(TokenType::TOKEN_COMMA);
    if (c == '.') return makeToken(TokenType::TOKEN_DOT);
    if (c == '-') return makeToken(TokenType::TOKEN_MINUS);
    if (c == '+') return makeToken(TokenType::TOKEN_PLUS);
    if (c == ';') return makeToken(TokenType::TOKEN_SEMICOLON);
    if (c == '*') return makeToken(TokenType::TOKEN_STAR);

    if (c == '!') {
        return makeToken(match('=') ? TokenType::TOKEN_BANG_EQUAL : TokenType::TOKEN_BANG);
    }
    if (c == '=') {
        return makeToken(match('=') ? TokenType::TOKEN_EQUAL_EQUAL : TokenType::TOKEN_EQUAL);
    }
    if (c == '<') {
        return makeToken(match('=') ? TokenType::TOKEN_LESS_EQUAL : TokenType::TOKEN_LESS);
    }
    if (c == '>') {
        return makeToken(match('=') ? TokenType::TOKEN_GREATER_EQUAL : TokenType::TOKEN_GREATER);
    }

    if (c == '/') {
        if (match('/')) {
            while (peek() != '\n' && peek() != '\0') {
                advance();
            }
            return scanToken();
        } else if (match('*')) {
            while (true) {
                if (peek() == '\0') {
                    return errorToken("Unterminated comment");
                }
                if (peek() == '\n') {
                    line++;
                }
                if (peek() == '*' && peekNext() == '/') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
            return scanToken();
        } else {
            return makeToken(TokenType::TOKEN_SLASH);
        }
    }

    if (c == '"') return string();
    if (c >= '0' && c <= '9') return number();
    if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return identifier();

    return errorToken("Unexpected character");
}

void Scanner::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            line++;
            advance();
            break;
        default:
            return;
        }
    }
}

Token Scanner::identifier() {
    while (true) {
        char c = peek();
        if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            advance();
        } else {
            break;
        }
    }
    return makeToken(identifierType());
}

TokenType Scanner::identifierType() const {
    std::string_view lexeme = source.substr(start, current - start);

    if (lexeme == "and") return TokenType::TOKEN_AND;
    if (lexeme == "class") return TokenType::TOKEN_CLASS;
    if (lexeme == "else") return TokenType::TOKEN_ELSE;
    if (lexeme == "false") return TokenType::TOKEN_FALSE;
    if (lexeme == "for") return TokenType::TOKEN_FOR;
    if (lexeme == "fun") return TokenType::TOKEN_FUN;
    if (lexeme == "if") return TokenType::TOKEN_IF;
    if (lexeme == "nil") return TokenType::TOKEN_NIL;
    if (lexeme == "or") return TokenType::TOKEN_OR;
    if (lexeme == "print") return TokenType::TOKEN_PRINT;
    if (lexeme == "return") return TokenType::TOKEN_RETURN;
    if (lexeme == "super") return TokenType::TOKEN_SUPER;
    if (lexeme == "this") return TokenType::TOKEN_THIS;
    if (lexeme == "true") return TokenType::TOKEN_TRUE;
    if (lexeme == "var") return TokenType::TOKEN_VAR;
    if (lexeme == "while") return TokenType::TOKEN_WHILE;

    return TokenType::TOKEN_IDENTIFIER;
}

Token Scanner::number() {
    while (true) {
        char c = peek();
        if (c >= '0' && c <= '9') {
            advance();
        } else {
            break;
        }
    }

    if (peek() == '.' && (peekNext() >= '0' && peekNext() <= '9')) {
        advance();
        while (true) {
            char c = peek();
            if (c >= '0' && c <= '9') {
                advance();
            } else {
                break;
            }
        }
    }

    return makeToken(TokenType::TOKEN_NUMBER);
}

Token Scanner::string() {
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\n') {
            line++;
        }
        advance();
    }

    if (peek() == '\0') {
        return errorToken("Unterminated string");
    }

    advance();
    return makeToken(TokenType::TOKEN_STRING);
}

char Scanner::advance() {
    return source[current++];
}

char Scanner::peek() const {
    if (current >= source.size()) {
        return '\0';
    }
    return source[current];
}

char Scanner::peekNext() const {
    if (current + 1 >= source.size()) {
        return '\0';
    }
    return source[current + 1];
}

bool Scanner::match(char expected) {
    if (peek() == '\0') return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

Token Scanner::makeToken(TokenType type) const {
    return Token{
        .type = type,
        .start = source.substr(start, current - start),
        .length = static_cast<int>(current - start),
        .line = line
    };
}

Token Scanner::errorToken(std::string_view message) const {
    return Token{
        .type = TokenType::TOKEN_ERROR,
        .start = message,
        .length = static_cast<int>(message.size()),
        .line = line
    };
}

}
