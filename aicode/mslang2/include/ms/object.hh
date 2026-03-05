#pragma once

#include "ms/common.hh"
#include "ms/value.hh"

namespace ms {

class Table;

enum class ObjType : uint8_t {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_METHOD,
    OBJ_BOUND_METHOD,
    OBJ_LIST,
    OBJ_MAP,
};

using NativeFn = Value(*)(int argCount, Value* args);

struct ValueHash {
    size_t operator()(const Value& value) const;
};

struct ValueEqual {
    bool operator()(const Value& a, const Value& b) const;
};

struct ObjUpvalue;

struct Obj {
    ObjType type;
    bool isMarked;
    Obj* next;

    Obj(ObjType t) : type(t), isMarked(false), next(nullptr) {}

    std::string toString() const;
};

struct ObjString : Obj {
    std::string str;
    uint32_t hash;

    explicit ObjString(std::string&& s);
    explicit ObjString(char* chars);
    std::string toString() const;
};

class Chunk;

struct ObjFunction : Obj {
    int arity;
    int upvalueCount;
    Chunk* chunk;
    ObjString* name;

    ObjFunction();
    std::string toString() const;
};

struct ObjNative : Obj {
    NativeFn function;

    explicit ObjNative(NativeFn fn) : Obj(ObjType::OBJ_NATIVE), function(fn) {}
    std::string toString() const;
};

struct ObjClosure : Obj {
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;

    explicit ObjClosure(ObjFunction* fn);
    ~ObjClosure();
    std::string toString() const;
};

struct ObjUpvalue : Obj {
    Value* location;
    Value* closed;
    ObjUpvalue* next;

    explicit ObjUpvalue(Value* loc);
    std::string toString() const;
};

struct ObjClass : Obj {
    ObjString* name;
    Table* methods;

    explicit ObjClass(ObjString* n);
    std::string toString() const;
};

struct ObjInstance : Obj {
    ObjClass* klass;
    Table* fields;

    explicit ObjInstance(ObjClass* k);
    std::string toString() const;
};

struct ObjMethod : Obj {
    ObjClosure* closure;

    explicit ObjMethod(ObjClosure* c);
    std::string toString() const;
};

struct ObjBoundMethod : Obj {
    Value* receiver;
    ObjClosure* method;

    ObjBoundMethod(Value* rec, ObjClosure* m);
    std::string toString() const;
};

struct ObjList : Obj {
    std::vector<Value> elements;

    ObjList();
    std::string toString() const;
};

struct ObjMap : Obj {
    std::unordered_map<Value, Value, ValueHash, ValueEqual> map;

    ObjMap();
    std::string toString() const;
};

ObjString* takeString(std::string* s);
ObjString* copyString(const char* s, size_t length);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);
ObjClosure* newClosure(ObjFunction* function);
ObjUpvalue* newUpvalue(Value* location);
ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);
ObjMethod* newMethod(ObjClosure* closure);
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjList* newList();
ObjMap* newMap();

}
