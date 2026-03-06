#include "ms/vm.hh"

#include "ms/chunk.hh"
#include "ms/value.hh"
#include "ms/object.hh"
#include "ms/table.hh"
#include "ms/compiler.hh"

#include <iostream>
#include <cstring>

namespace ms {

VM::VM() : chunk(nullptr), ip(nullptr), grayCount(0), grayCapacity(0) {
}

bool VM::interpret(std::string_view source) {
    resetStack();
    frames.clear();

    Compiler compiler(nullptr, nullptr);
    Chunk chunk;
    compiler = Compiler(nullptr, &chunk);

    if (!compiler.compile(source, &chunk)) {
        return false;
    }

    this->chunk = &chunk;
    ip = chunk.code().data();

    ObjFunction* function = newFunction();
    function->chunk = &chunk;
    function->arity = 0;

    ObjClosure* closure = newClosure(function);
    push(OBJ_VAL(closure));
    call(closure, 0);

    InterpretResult result = run();

    return result == InterpretResult::INTERPRET_OK;
}

InterpretResult VM::run() {
    CallFrame frame;

    auto READ_BYTE = [&]() -> uint8_t {
        return *ip++;
    };

    auto READ_CONSTANT = [&]() -> Value {
        return chunk->constants()[*ip++];
    };

    auto READ_SHORT = [&]() -> uint16_t {
        ip += 2;
        return static_cast<uint16_t>((*(ip - 2) << 8) | *(ip - 1));
    };

    auto READ_STRING = [&]() -> ObjString* {
        return static_cast<ObjString*>(READ_CONSTANT().asObj());
    };

    auto BINARY_OP = [&](OpCode op) {
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
            std::cerr << "Operands must be two numbers." << std::endl;
            return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }

        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());

        switch (op) {
            case OpCode::OP_ADD:
                push(NUMBER_VAL(a + b));
                break;
            case OpCode::OP_SUBTRACT:
                push(NUMBER_VAL(a - b));
                break;
            case OpCode::OP_MULTIPLY:
                push(NUMBER_VAL(a * b));
                break;
            case OpCode::OP_DIVIDE:
                if (b == 0) {
                    std::cerr << "Division by zero." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(a / b));
                break;
            default:
                break;
        }
        return InterpretResult::INTERPRET_OK;
    };

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "          ";
        for (Value* slot = stack.data(); slot < stackTop(); slot++) {
            std::cout << "[";
            printValue(*slot);
            std::cout << "]";
        }
        std::cout << std::endl;
#endif

        frame = frames.back();
        ip = frame.ip;
        chunk = frame.closure->function->chunk;

        uint8_t instruction = READ_BYTE();
        frame.ip = ip;
        frames.back() = frame;

        switch (instruction) {
            case static_cast<uint8_t>(OpCode::OP_CONSTANT): {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_NIL):
                push(NIL_VAL);
                break;

            case static_cast<uint8_t>(OpCode::OP_TRUE):
                push(BOOL_VAL(true));
                break;

            case static_cast<uint8_t>(OpCode::OP_FALSE):
                push(BOOL_VAL(false));
                break;

            case static_cast<uint8_t>(OpCode::OP_POP):
                pop();
                break;

            case static_cast<uint8_t>(OpCode::OP_GET_LOCAL): {
                uint8_t slot = READ_BYTE();
                frame = frames.back();
                push(frame.slots[slot]);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SET_LOCAL): {
                uint8_t slot = READ_BYTE();
                frame = frames.back();
                frame.slots[slot] = peek(0);
                frames.back() = frame;
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_GET_GLOBAL): {
                ObjString* name = READ_STRING();
                Value value;
                if (!globals.get(name, value)) {
                    std::cerr << "Undefined variable '" << name->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL): {
                ObjString* name = READ_STRING();
                globals.set(name, peek(0));
                pop();
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SET_GLOBAL): {
                ObjString* name = READ_STRING();
                if (globals.set(name, peek(0))) {
                    std::cerr << "Undefined variable '" << name->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_GET_UPVALUE): {
                uint8_t slot = READ_BYTE();
                frame = frames.back();
                ObjUpvalue* upvalue = frame.closure->upvalues[slot];
                push(*upvalue->location);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SET_UPVALUE): {
                uint8_t slot = READ_BYTE();
                frame = frames.back();
                ObjUpvalue* upvalue = frame.closure->upvalues[slot];
                *upvalue->location = peek(0);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_GET_PROPERTY): {
                if (!IS_INSTANCE(peek(0))) {
                    std::cerr << "Only instances have properties." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();
                Value value;
                if (instance->fields->get(name, value)) {
                    pop();
                    push(value);
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    std::cerr << "Undefined property '" << name->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SET_PROPERTY): {
                if (!IS_INSTANCE(peek(1))) {
                    std::cerr << "Only instances have fields." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(1));
                ObjString* name = READ_STRING();
                instance->fields->set(name, peek(0));

                Value value = pop();
                pop();
                push(value);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_GET_SUPER): {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());
                if (!bindMethod(superclass, name)) {
                    std::cerr << "Undefined property '" << name->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_EQUAL): {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_GREATER): {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    std::cerr << "Operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(BOOL_VAL(a > b));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_LESS): {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    std::cerr << "Operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(BOOL_VAL(a < b));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_ADD): {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    std::cerr << "Operands must be two numbers or two strings." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SUBTRACT):
                if (BINARY_OP(OpCode::OP_SUBTRACT) == InterpretResult::INTERPRET_RUNTIME_ERROR) {
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;

            case static_cast<uint8_t>(OpCode::OP_MULTIPLY):
                if (BINARY_OP(OpCode::OP_MULTIPLY) == InterpretResult::INTERPRET_RUNTIME_ERROR) {
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;

            case static_cast<uint8_t>(OpCode::OP_DIVIDE):
                if (BINARY_OP(OpCode::OP_DIVIDE) == InterpretResult::INTERPRET_RUNTIME_ERROR) {
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;

            case static_cast<uint8_t>(OpCode::OP_NOT):
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case static_cast<uint8_t>(OpCode::OP_NEGATE):
                if (!IS_NUMBER(peek(0))) {
                    std::cerr << "Operand must be a number." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case static_cast<uint8_t>(OpCode::OP_PRINT): {
                printValue(pop());
                std::cout << std::endl;
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_JUMP): {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE): {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) {
                    ip += offset;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_LOOP): {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_CALL): {
                int argCount = READ_BYTE();
                Value callee = peek(argCount);
                if (!callValue(callee, argCount)) {
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                frame = CallFrame(
                    AS_CLOSURE(callee),
                    ip,
                    stack.data() + stack.size() - argCount - 1
                );
                frames.push_back(frame);
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_INVOKE): {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                Value receiver = peek(argCount);
                if (!IS_INSTANCE(receiver)) {
                    std::cerr << "Only instances have methods." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(receiver);
                Value value;
                if (instance->fields->get(method, value)) {
                    if (!callValue(value, argCount)) {
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    frame = CallFrame(
                        AS_CLOSURE(peek(argCount)),
                        ip,
                        stack.data() + stack.size() - argCount - 1
                    );
                    frames.push_back(frame);
                    break;
                }
                if (!invoke(method, argCount)) {
                    std::cerr << "Undefined property '" << method->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_SUPER): {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());
                if (!bindMethod(superclass, name)) {
                    std::cerr << "Undefined property '" << name->str << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_CLOSURE): {
                ObjFunction* function = static_cast<ObjFunction*>(READ_CONSTANT().asObj());
                ObjClosure* closure = newClosure(function);

                if (function->upvalueCount > 0) {
                    closure->upvalues = new ObjUpvalue*[function->upvalueCount];
                    closure->upvalueCount = function->upvalueCount;
                    for (int i = 0; i < function->upvalueCount; i++) {
                        uint8_t isLocal = READ_BYTE();
                        uint8_t index = READ_BYTE();
                        if (isLocal) {
                            frame = frames.back();
                            closure->upvalues[i] = captureUpvalue(frame.slots + index);
                        } else {
                            frame = frames.back();
                            closure->upvalues[i] = frame.closure->upvalues[index];
                        }
                    }
                }

                push(OBJ_VAL(closure));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_CLOSE_UPVALUE): {
                closeUpvalues(stack.data() + stack.size() - 1);
                pop();
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_RETURN): {
                Value result = pop();

                closeUpvalues(frames.back().slots - 1);
                frames.pop_back();

                if (frames.empty()) {
                    stack.clear();
                    push(result);
                    return InterpretResult::INTERPRET_OK;
                }

                stack.resize(stack.size() - frames.back().closure->function->arity - 1);
                push(result);

                frame = frames.back();
                chunk = frame.closure->function->chunk;
                ip = frame.ip;
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_CLASS): {
                ObjClass* klass = newClass(READ_STRING());
                push(OBJ_VAL(klass));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_INHERIT): {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    std::cerr << "Superclass must be a class." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                subclass->methods->addAll(AS_CLASS(superclass)->methods);
                pop();
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_METHOD): {
                defineMethod(READ_STRING());
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_LIST): {
                int elementCount = READ_BYTE();
                ObjList* list = newList();
                list->elements.reserve(elementCount);
                for (int i = 0; i < elementCount; i++) {
                    list->elements.insert(list->elements.begin(), pop());
                }
                push(OBJ_VAL(list));
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_INDEX): {
                Value indexValue = pop();
                Value object = pop();
                
                if (IS_LIST(object)) {
                    ObjList* list = AS_LIST(object);
                    if (!IS_NUMBER(indexValue)) {
                        std::cerr << "List index must be a number." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    int index = static_cast<int>(AS_NUMBER(indexValue));
                    if (index < 0 || index >= static_cast<int>(list->elements.size())) {
                        std::cerr << "List index out of bounds." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    push(list->elements[index]);
                } else if (IS_MAP(object)) {
                    ObjMap* map = AS_MAP(object);
                    auto it = map->map.find(indexValue);
                    if (it == map->map.end()) {
                        push(NIL_VAL);
                    } else {
                        push(it->second);
                    }
                } else if (IS_STRING(object)) {
                    ObjString* string = AS_STRING(object);
                    if (!IS_NUMBER(indexValue)) {
                        std::cerr << "String index must be a number." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    int index = static_cast<int>(AS_NUMBER(indexValue));
                    if (index < 0 || index >= static_cast<int>(string->str.size())) {
                        std::cerr << "String index out of bounds." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    std::string s(1, string->str[index]);
                    push(OBJ_VAL(copyString(s.data(), s.size())));
                } else {
                    std::cerr << "Only lists, maps, and strings support indexing." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_INDEX_ASSIGN): {
                Value value = pop();
                Value indexValue = pop();
                Value object = pop();

                if (IS_LIST(object)) {
                    ObjList* list = AS_LIST(object);
                    if (!IS_NUMBER(indexValue)) {
                        std::cerr << "List index must be a number." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    int index = static_cast<int>(AS_NUMBER(indexValue));
                    if (index < 0 || index >= static_cast<int>(list->elements.size())) {
                        std::cerr << "List index out of bounds." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    list->elements[index] = value;
                } else if (IS_MAP(object)) {
                    ObjMap* map = AS_MAP(object);
                    map->map[indexValue] = value;
                } else {
                    std::cerr << "Only lists and maps support index assignment." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
        }
    }
}

void VM::resetStack() {
    stack.clear();
    ip = nullptr;
    chunk = nullptr;
}

void VM::push(Value value) {
    stack.push_back(value);
}

Value VM::pop() {
    Value value = stack.back();
    stack.pop_back();
    return value;
}

Value VM::peek(int distance) {
    return stack[stack.size() - 1 - distance];
}

bool VM::call(ObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        std::cerr << "Expected " << closure->function->arity << " arguments but got " << argCount << "." << std::endl;
        return false;
    }

    CallFrame frame(closure, ip, stack.data() + stack.size() - argCount - 1);
    frames.push_back(frame);

    return true;
}

bool VM::callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (AS_OBJ(callee)->type) {
            case ObjType::OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case ObjType::OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee)->function;
                Value result = native(argCount, stack.data() + stack.size() - argCount);
                stack.resize(stack.size() - argCount - 1);
                push(result);
                return true;
            }
            case ObjType::OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(callee);
                ObjInstance* instance = newInstance(klass);
                pop();
                push(OBJ_VAL(instance));
                return callValue(OBJ_VAL(instance), argCount);
            }
            case ObjType::OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                stack[stack.size() - argCount - 1] = *bound->receiver;
                return call(bound->method, argCount);
            }
            default:
                break;
        }
    }
    std::cerr << "Can only call functions and classes." << std::endl;
    return false;
}

ObjUpvalue* VM::captureUpvalue(Value* local) {
    if (openUpvalues.empty()) {
        ObjUpvalue* upvalue = newUpvalue(local);
        openUpvalues.push_back(upvalue);
        return upvalue;
    }

    ObjUpvalue* prevUpvalue = nullptr;
    ObjUpvalue* upvalue = openUpvalues.back();

    while (upvalue != nullptr && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != nullptr && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == nullptr) {
        openUpvalues.push_back(createdUpvalue);
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

void VM::closeUpvalues(Value* last) {
    while (!openUpvalues.empty() && openUpvalues.back()->location >= last) {
        ObjUpvalue* upvalue = openUpvalues.back();
        upvalue->closed = upvalue->location;
        upvalue->location = upvalue->closed;
        openUpvalues.pop_back();
    }
}

void VM::defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    klass->methods->set(name, method);
    pop();
}

bool VM::bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!klass->methods->get(name, method)) {
        return false;
    }

    ObjClosure* closure = AS_METHOD(method)->closure;
    ObjBoundMethod* bound = newBoundMethod(stack.data() + stack.size() - 1, closure);
    pop();
    push(OBJ_VAL(bound));
    return true;
}

bool VM::invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);
    if (!IS_INSTANCE(receiver)) {
        std::cerr << "Only instances have methods." << std::endl;
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);
    Value value;
    if (instance->fields->get(name, value)) {
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

bool VM::invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!klass->methods->get(name, method)) {
        std::cerr << "Undefined property '" << name->str << "'." << std::endl;
        return false;
    }
    return callValue(method, argCount);
}

void VM::concatenate() {
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    std::string result = a->str + b->str;
    pop();
    pop();

    push(OBJ_VAL(copyString(result.data(), result.size())));
}

Chunk* VM::currentChunk() {
    return chunk;
}

bool VM::isFalsey(Value value) {
    return value.isNil() || (value.isBool() && !value.asBool());
}

bool VM::isStringsOrNumbers(Value a, Value b) {
    return (IS_STRING(a) && IS_STRING(b)) || (IS_NUMBER(a) && IS_NUMBER(b));
}

void VM::binaryOp(OpCode op) {
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
        std::cerr << "Operands must be numbers." << std::endl;
        return;
    }

    double b = AS_NUMBER(pop());
    double a = AS_NUMBER(pop());

    switch (op) {
        case OpCode::OP_ADD:
            push(NUMBER_VAL(a + b));
            break;
        case OpCode::OP_SUBTRACT:
            push(NUMBER_VAL(a - b));
            break;
        case OpCode::OP_MULTIPLY:
            push(NUMBER_VAL(a * b));
            break;
        case OpCode::OP_DIVIDE:
            push(NUMBER_VAL(a / b));
            break;
        default:
            break;
    }
}

}
