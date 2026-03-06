#pragma once

#include "ms/chunk.hh"
#include "ms/value.hh"
#include "ms/object.hh"
#include "ms/table.hh"
#include "ms/common.hh"

#include <vector>
#include <string>
#include <optional>

namespace ms {

struct Upvalue {
    Value* location;
    Value closed;
    Upvalue* next;

    explicit Upvalue(Value* loc) : location(loc), next(nullptr) {}
};

struct CallFrame {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;

    CallFrame() : closure(nullptr), ip(nullptr), slots(nullptr) {}
    CallFrame(ObjClosure* cl, uint8_t* ip, Value* slots) : closure(cl), ip(ip), slots(slots) {}
    ObjFunction* function() const { return closure ? closure->function : nullptr; }
};

enum class InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
public:
    VM();
    ~VM() = default;

    bool interpret(std::string_view source);
    InterpretResult run();

    void resetStack();
    void push(Value value);
    Value pop();
    Value peek(int distance);

    bool call(ObjClosure* closure, int argCount);
    bool callValue(Value callee, int argCount);
    ObjUpvalue* captureUpvalue(Value* local);
    void closeUpvalues(Value* last);
    void defineMethod(ObjString* name);
    bool bindMethod(ObjClass* klass, ObjString* name);
    bool invoke(ObjString* name, int argCount);
    bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount);
    void concatenate();

    Chunk* currentChunk();
    Value* stackTop() { return stack.data() + stack.size(); }

private:
    Chunk* chunk;
    uint8_t* ip;
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::vector<ObjUpvalue*> openUpvalues;
    Table globals;
    int grayCount;
    int grayCapacity;
    std::vector<Obj*> grayStack;

    bool isFalsey(Value value);
    bool isStringsOrNumbers(Value a, Value b);
    void binaryOp(OpCode op);
};

}
