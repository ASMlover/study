#include "value.hh"

#include <cstdio>

namespace ms {

bool Value::isObjType(ObjType type) const {
    return isObj() && asObj()->type == type;
}

void printValue(const Value& value) {
    if (value.isNil()) {
        std::printf("nil");
    } else if (value.isBool()) {
        std::printf(value.asBool() ? "true" : "false");
    } else if (value.isNumber()) {
        std::printf("%g", value.asNumber());
    } else if (value.isObj()) {
        printObject(value);
    }
}

uint32_t hashValue(const Value& value) {
    if (value.isNil()) {
        return 0;
    } else if (value.isBool()) {
        return value.asBool() ? 1 : 0;
    } else if (value.isNumber()) {
        // 对double的位表示进行哈希
        double num = value.asNumber();
        uint64_t bits;
        std::memcpy(&bits, &num, sizeof(num));
        return static_cast<uint32_t>(bits ^ (bits >> 32));
    } else if (value.isObj()) {
        // 对于对象，使用对象指针的哈希
        Obj* obj = value.asObj();
        if (obj->type == ObjType::String) {
            return static_cast<ObjString*>(obj)->hash;
        }
        uint64_t ptr = reinterpret_cast<uint64_t>(obj);
        return static_cast<uint32_t>(ptr ^ (ptr >> 32));
    }
    return 0;
}

} // namespace ms
