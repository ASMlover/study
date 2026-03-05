#include "ms/compiler.hh"

#include "ms/object.hh"
#include <cstring>

namespace ms {

Compiler::Compiler(Scanner* scanner, Chunk* chunk)
    : scanner(scanner), chunk(chunk), hadError(false), panicMode(false) {
}

bool Compiler::compile(std::string_view source, Chunk* chunk) {
    this->chunk = chunk;
    scanner->initSource(source);

    hadError = false;
    panicMode = false;
    advance();

    while (!match(TokenType::TOKEN_EOF)) {
        declaration();
    }

    return !hadError;
}

void Compiler::error(std::string_view message) {
    if (panicMode) return;
    panicMode = true;

    std::cerr << "[line " << previous.line << "] Error";
    if (previous.type == TokenType::TOKEN_EOF) {
        std::cerr << " at end";
    } else {
        std::cerr << " at '" << previous.start << "'";
    }
    std::cerr << ": " << message << "\n";
    hadError = true;
}

void Compiler::errorAtCurrent(std::string_view message) {
    errorAt(current, message);
}

void Compiler::errorAt(Token token, std::string_view message) {
    if (panicMode) return;
    panicMode = true;

    std::cerr << "[line " << token.line << "] Error";
    if (token.type == TokenType::TOKEN_EOF) {
        std::cerr << " at end";
    } else {
        std::cerr << " at '" << token.start << "'";
    }
    std::cerr << ": " << message << "\n";
    hadError = true;
}

void Compiler::advance() {
    previous = current;

    while (true) {
        current = scanner->scanToken();
        if (current.type != TokenType::TOKEN_ERROR) break;

        errorAtCurrent(std::string(current.start));
    }
}

bool Compiler::consume(TokenType type, std::string_view message) {
    if (current.type == type) {
        advance();
        return true;
    }

    errorAtCurrent(message);
    return false;
}

bool Compiler::check(TokenType type) {
    return current.type == type;
}

bool Compiler::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

void Compiler::expression() {
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void Compiler::grouping() {
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::number() {
    double value = std::stod(std::string(previous.start));
    emitConstant(NUMBER_VAL(value));
}

void Compiler::string() {
    std::string str(previous.start.substr(1, previous.length - 2));
    emitConstant(OBJ_VAL(takeString(&str)));
}

void Compiler::literal() {
    switch (previous.type) {
        case TokenType::TOKEN_NIL:
            emitByte(static_cast<uint8_t>(OpCode::OP_NIL));
            break;
        case TokenType::TOKEN_TRUE:
            emitByte(static_cast<uint8_t>(OpCode::OP_TRUE));
            break;
        case TokenType::TOKEN_FALSE:
            emitByte(static_cast<uint8_t>(OpCode::OP_FALSE));
            break;
        default:
            break;
    }
}

void Compiler::namedVariable() {
    uint8_t arg = identifierConstant(previous);

    if (match(TokenType::TOKEN_EQUAL)) {
        expression();
        emitByte(static_cast<uint8_t>(OpCode::OP_SET_LOCAL), arg);
    } else {
        emitByte(static_cast<uint8_t>(OpCode::OP_GET_LOCAL), arg);
    }
}

void Compiler::variable() {
    namedVariable();
}

void Compiler::unary() {
    TokenType operatorType = previous.type;

    parsePrecedence(Precedence::PREC_UNARY);

    switch (operatorType) {
        case TokenType::TOKEN_BANG:
            emitByte(static_cast<uint8_t>(OpCode::OP_NOT));
            break;
        case TokenType::TOKEN_MINUS:
            emitByte(static_cast<uint8_t>(OpCode::OP_NEGATE));
            break;
        default:
            break;
    }
}

void Compiler::binary() {
    TokenType operatorType = previous.type;
    ParseRule& rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

    switch (operatorType) {
        case TokenType::TOKEN_BANG_EQUAL:
            emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL));
            emitByte(static_cast<uint8_t>(OpCode::OP_NOT));
            break;
        case TokenType::TOKEN_EQUAL_EQUAL:
            emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL));
            break;
        case TokenType::TOKEN_GREATER:
            emitByte(static_cast<uint8_t>(OpCode::OP_GREATER));
            break;
        case TokenType::TOKEN_GREATER_EQUAL:
            emitByte(static_cast<uint8_t>(OpCode::OP_LESS));
            emitByte(static_cast<uint8_t>(OpCode::OP_NOT));
            break;
        case TokenType::TOKEN_LESS:
            emitByte(static_cast<uint8_t>(OpCode::OP_LESS));
            break;
        case TokenType::TOKEN_LESS_EQUAL:
            emitByte(static_cast<uint8_t>(OpCode::OP_GREATER));
            emitByte(static_cast<uint8_t>(OpCode::OP_NOT));
            break;
        case TokenType::TOKEN_PLUS:
            emitByte(static_cast<uint8_t>(OpCode::OP_ADD));
            break;
        case TokenType::TOKEN_MINUS:
            emitByte(static_cast<uint8_t>(OpCode::OP_SUBTRACT));
            break;
        case TokenType::TOKEN_STAR:
            emitByte(static_cast<uint8_t>(OpCode::OP_MULTIPLY));
            break;
        case TokenType::TOKEN_SLASH:
            emitByte(static_cast<uint8_t>(OpCode::OP_DIVIDE));
            break;
        default:
            break;
    }
}

void Compiler::expressionStatement() {
    expression();
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(static_cast<uint8_t>(OpCode::OP_POP));
}

void Compiler::printStatement() {
    consume(TokenType::TOKEN_PRINT, "Expect 'print'.");
    expressionStatement();
    emitByte(static_cast<uint8_t>(OpCode::OP_PRINT));
}

void Compiler::statement() {
    if (match(TokenType::TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}

void Compiler::declaration() {
    if (match(TokenType::TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (panicMode) {
        synchronize();
    }
}

void Compiler::varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TokenType::TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(static_cast<uint8_t>(OpCode::OP_NIL));
    }
    consume(TokenType::TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

uint8_t Compiler::parseVariable(std::string_view errorMessage) {
    consume(TokenType::TOKEN_IDENTIFIER, errorMessage);

    return identifierConstant(previous);
}

void Compiler::defineVariable(uint8_t global) {
    emitByte(static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL), global);
}

uint8_t Compiler::identifierConstant(Token name) {
    return static_cast<uint8_t>(chunk->addConstant(OBJ_VAL(copyString(name.start.data(), name.length))));
}

int Compiler::resolveLocal(Token name) {
    for (int i = localCount - 1; i >= 0; i--) {
        Local& local = locals[i];
        if (name.length == local.name.length &&
            std::memcmp(name.start.data(), local.name.start.data(), name.length) == 0) {
            return i;
        }
    }
    return -1;
}

void Compiler::addLocal(Token name) {
    if (localCount >= 256) {
        error("Too many local variables in function.");
        return;
    }
    Local local;
    local.name = name;
    local.depth = scopeDepth;
    local.isCaptured = false;
    locals.push_back(local);
    localCount++;
}

void Compiler::parsePrecedence(Precedence precedence) {
    advance();
    auto prefixRule = getRule(previous.type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= Precedence::PREC_ASSIGNMENT;
    (this->*prefixRule)();

    while (precedence <= getRule(current.type).precedence) {
        advance();
        auto infixRule = getRule(previous.type).infix;
        (this->*infixRule)();
    }

    if (canAssign && match(TokenType::TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

void Compiler::emitByte(uint8_t byte) {
    chunk->writeChunk(byte, previous.line);
}

void Compiler::emitByte(uint8_t byte1, uint8_t byte2) {
    chunk->writeChunk(byte1, previous.line);
    chunk->writeChunk(byte2, previous.line);
}

void Compiler::emitConstant(Value value) {
    int index = chunk->addConstant(value);
    if (index <= 255) {
        emitByte(static_cast<uint8_t>(OpCode::OP_CONSTANT), static_cast<uint8_t>(index));
    } else {
        error("Too many constants in one chunk.");
    }
}

void Compiler::emitReturn() {
    emitByte(static_cast<uint8_t>(OpCode::OP_NIL));
    emitByte(static_cast<uint8_t>(OpCode::OP_RETURN));
}

void Compiler::synchronize() {
    panicMode = false;

    while (current.type != TokenType::TOKEN_EOF) {
        if (previous.type == TokenType::TOKEN_SEMICOLON) return;

        switch (current.type) {
            case TokenType::TOKEN_CLASS:
            case TokenType::TOKEN_FUN:
            case TokenType::TOKEN_VAR:
            case TokenType::TOKEN_FOR:
            case TokenType::TOKEN_IF:
            case TokenType::TOKEN_WHILE:
            case TokenType::TOKEN_PRINT:
            case TokenType::TOKEN_RETURN:
                return;
            default:
                break;
        }

        advance();
    }
}

ParseRule Compiler::rules[] = {
    {&Compiler::grouping, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {&Compiler::unary, &Compiler::binary, Precedence::PREC_TERM},
    {nullptr, &Compiler::binary, Precedence::PREC_TERM},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, &Compiler::binary, Precedence::PREC_FACTOR},
    {nullptr, &Compiler::binary, Precedence::PREC_FACTOR},
    {&Compiler::unary, nullptr, Precedence::PREC_NONE},
    {nullptr, &Compiler::binary, Precedence::PREC_EQUALITY},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, &Compiler::binary, Precedence::PREC_EQUALITY},
    {nullptr, &Compiler::binary, Precedence::PREC_COMPARISON},
    {nullptr, &Compiler::binary, Precedence::PREC_COMPARISON},
    {nullptr, &Compiler::binary, Precedence::PREC_COMPARISON},
    {nullptr, &Compiler::binary, Precedence::PREC_COMPARISON},
    {&Compiler::variable, nullptr, Precedence::PREC_NONE},
    {&Compiler::string, nullptr, Precedence::PREC_NONE},
    {&Compiler::number, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {&Compiler::literal, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {&Compiler::literal, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {&Compiler::literal, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
    {nullptr, nullptr, Precedence::PREC_NONE},
};

ParseRule& Compiler::getRule(TokenType type) {
    return rules[static_cast<int>(type)];
}

}
