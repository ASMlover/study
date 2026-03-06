#pragma once

#include "ms/common.hh"

#include <string>
#include <string_view>

namespace ms {

struct Obj;

enum class Type : uint8_t {
    Nil,
    Bool,
    Number,
    Obj,
};

struct Value {
    Type type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;

    Value() : type(Type::Nil), as{ .number = 0 } {}
    Value(double num) : type(Type::Number), as{ .number = num } {}
    Value(bool b) : type(Type::Bool), as{ .boolean = b } {}
    Value(Obj* o) : type(Type::Obj), as{ .obj = o } {}
    Value(Type t) : type(t), as{ .number = 0 } {}

    bool isNil() const { return type == Type::Nil; }
    bool isBool() const { return type == Type::Bool; }
    bool isNumber() const { return type == Type::Number; }
    bool isObj() const { return type == Type::Obj; }

    bool asBool() const { return as.boolean; }
    double asNumber() const { return as.number; }
    Obj* asObj() const { return as.obj; }

    std::string toString() const;
    std::string_view toStringView() const;

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
};

#define IS_NIL(value) ((value).type == ms::Type::Nil)
#define IS_BOOL(value) ((value).type == ms::Type::Bool)
#define IS_NUMBER(value) ((value).type == ms::Type::Number)
#define IS_OBJ(value) ((value).type == ms::Type::Obj)

#define IS_STRING(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_STRING)
#define IS_FUNCTION(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_FUNCTION)
#define IS_NATIVE(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_NATIVE)
#define IS_CLOSURE(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_CLOSURE)
#define IS_CLASS(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_CLASS)
#define IS_INSTANCE(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_INSTANCE)
#define IS_METHOD(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_METHOD)
#define IS_BOUND_METHOD(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_BOUND_METHOD)
#define IS_LIST(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_LIST)
#define IS_MAP(value) (IS_OBJ(value) && AS_OBJ(value)->type == ms::ObjType::OBJ_MAP)

#define AS_STRING(value) (static_cast<ms::ObjString*>(AS_OBJ(value)))
#define AS_FUNCTION(value) (static_cast<ms::ObjFunction*>(AS_OBJ(value)))
#define AS_NATIVE(value) (static_cast<ms::ObjNative*>(AS_OBJ(value)))
#define AS_CLOSURE(value) (static_cast<ms::ObjClosure*>(AS_OBJ(value)))
#define AS_CLASS(value) (static_cast<ms::ObjClass*>(AS_OBJ(value)))
#define AS_INSTANCE(value) (static_cast<ms::ObjInstance*>(AS_OBJ(value)))
#define AS_METHOD(value) (static_cast<ms::ObjMethod*>(AS_OBJ(value)))
#define AS_BOUND_METHOD(value) (static_cast<ms::ObjBoundMethod*>(AS_OBJ(value)))
#define AS_LIST(value) (static_cast<ms::ObjList*>(AS_OBJ(value)))
#define AS_MAP(value) (static_cast<ms::ObjMap*>(AS_OBJ(value)))

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define NIL_VAL ((ms::Value)(ms::Type::Nil))
#define BOOL_VAL(b) ((ms::Value)(bool)(b))
#define NUMBER_VAL(n) ((ms::Value)(double)(n))
#define OBJ_VAL(o) ((ms::Value)(ms::Obj*)(o))

inline bool isFalsey(Value value) {
    return value.isNil() || (value.isBool() && !value.asBool());
}

bool valuesEqual(Value a, Value b);
void printValue(Value value);

}
