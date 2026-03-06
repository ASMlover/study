#include "ms/value.hh"
#include "ms/object.hh"

#include <sstream>
#include <iomanip>

namespace ms {

std::string Value::toString() const {
    std::ostringstream oss;
    switch (type) {
        case Type::Nil:
            return "nil";
        case Type::Bool:
            return as.boolean ? "true" : "false";
        case Type::Number:
            oss << std::setprecision(17) << as.number;
            return oss.str();
        case Type::Obj:
            if (as.obj->type == ObjType::OBJ_STRING) {
                return static_cast<ObjString*>(as.obj)->str;
            }
            return "<object>";
    }
    return "<unknown>";
}

std::string_view Value::toStringView() const {
    if (type == Type::Obj && as.obj->type == ObjType::OBJ_STRING) {
        return static_cast<ObjString*>(as.obj)->str;
    }
    return "";
}

bool Value::operator==(const Value& other) const {
    return valuesEqual(*this, other);
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) {
        return false;
    }

    switch (a.type) {
        case Type::Nil:
            return true;
        case Type::Bool:
            return a.asBool() == b.asBool();
        case Type::Number:
            return a.asNumber() == b.asNumber();
        case Type::Obj:
            if (a.asObj()->type == ObjType::OBJ_STRING && 
                b.asObj()->type == ObjType::OBJ_STRING) {
                return static_cast<ObjString*>(a.asObj())->str ==
                       static_cast<ObjString*>(b.asObj())->str;
            }
            return a.asObj() == b.asObj();
    }
    return false;
}

void printValue(Value value) {
    switch (value.type) {
        case Type::Nil:
            std::cout << "nil";
            break;
        case Type::Bool:
            std::cout << (value.asBool() ? "true" : "false");
            break;
        case Type::Number:
            std::cout << std::setprecision(17) << value.asNumber();
            break;
        case Type::Obj:
            std::cout << value.asObj()->toString();
            break;
    }
}

}
