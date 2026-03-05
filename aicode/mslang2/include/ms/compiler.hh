#pragma once

#include "ms/scanner.hh"
#include "ms/chunk.hh"
#include "ms/value.hh"

#include <memory>
#include <vector>
#include <string_view>
#include <functional>

namespace ms {

enum class Precedence : int {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
};

struct ParseRule {
    void (Compiler::*prefix)();
    void (Compiler::*infix)();
    Precedence precedence;
};

struct Local {
    Token name;
    int depth;
    bool isCaptured;
};

class Compiler {
public:
    Compiler(Scanner* scanner, Chunk* chunk);

    bool compile(std::string_view source, Chunk* chunk);

private:
    Scanner* scanner;
    Chunk* chunk;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;

    std::vector<Local> locals;
    int localCount = 0;
    int scopeDepth = 0;

    void error(std::string_view message);
    void errorAtCurrent(std::string_view message);
    void errorAt(Token token, std::string_view message);

    void advance();
    bool consume(TokenType type, std::string_view message);
    bool check(TokenType type);
    bool match(TokenType type);

    void expression();
    void grouping();
    void unary();
    void binary();
    void number();
    void literal();
    void string();
    void variable();
    void namedVariable();

    void expressionStatement();
    void printStatement();
    void statement();
    void declaration();
    void varDeclaration();

    void parsePrecedence(Precedence precedence);

    void synchronize();

    uint8_t identifierConstant(Token name);
    int resolveLocal(Token name);
    void defineVariable(uint8_t global);
    uint8_t parseVariable(std::string_view errorMessage);
    void addLocal(Token name);

    void emitByte(uint8_t byte);
    void emitByte(uint8_t byte1, uint8_t byte2);
    void emitConstant(Value value);
    void emitReturn();

    static ParseRule rules[];
    static ParseRule& getRule(TokenType type);
};

}
