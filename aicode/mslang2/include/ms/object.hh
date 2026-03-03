#pragma once

#include "ms/common.hh"

namespace ms {

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

struct Obj {
    ObjType type;
    bool isMarked;
    Obj* next;

    Obj(ObjType type) : type(type), isMarked(false), next(nullptr) {}
};

struct ObjString : Obj {
    std::string str;

    ObjString(const std::string& s)
        : Obj(ObjType::OBJ_STRING), str(s) {}
};

}
