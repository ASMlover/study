#pragma once

#include "ms/value.hh"

#include <vector>

namespace ms {

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD,
    OP_LIST,
    OP_INDEX,
    OP_INDEX_ASSIGN,
};

class Chunk {
public:
    Chunk() = default;
    ~Chunk() = default;

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    void writeChunk(uint8_t byte, int line);
    int addConstant(Value value);
    Value getConstant(int index) const;
    uint8_t getCode(int index) const;
    int getLine(int index) const;
    size_t count() const;
    void clear();

    std::vector<uint8_t>& code() { return code_; }
    const std::vector<uint8_t>& code() const { return code_; }

    std::vector<Value>& constants() { return constants_; }
    const std::vector<Value>& constants() const { return constants_; }

    std::vector<int>& lines() { return lines_; }
    const std::vector<int>& lines() const { return lines_; }

private:
    std::vector<uint8_t> code_;
    std::vector<Value> constants_;
    std::vector<int> lines_;
};

}
