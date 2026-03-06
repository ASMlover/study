#pragma once

#include "common.hh"
#include "object.hh"

#include <cstdint>
#include <cstring>

namespace ms {

// 前向声明
struct Obj;
struct ObjString;

// Value类型枚举
enum class ValueType : u8 {
    Nil,
    Bool,
    Number,
    Obj
};

// 使用NaN Boxing技术的Value类
class Value {
private:
    // IEEE 754 double的位表示
    uint64_t bits_;
    
    // 特殊编码常量
    static constexpr uint64_t QNAN_MASK = 0x7FFC000000000000ULL;
    static constexpr uint64_t TAG_NIL   = 0x01;  // 001
    static constexpr uint64_t TAG_FALSE = 0x02;  // 010
    static constexpr uint64_t TAG_TRUE  = 0x03;  // 011
    static constexpr uint64_t TAG_OBJ   = 0x04;  // 100
    static constexpr uint64_t TAG_MASK  = 0x07;
    
    // 使用NaN的低3位作为类型标签
    static constexpr uint64_t NIL_BITS  = QNAN_MASK | TAG_NIL;
    static constexpr uint64_t TRUE_BITS = QNAN_MASK | TAG_TRUE;
    static constexpr uint64_t FALSE_BITS= QNAN_MASK | TAG_FALSE;
    static constexpr uint64_t OBJ_MASK  = QNAN_MASK | TAG_OBJ;
    
    // 私有构造函数
    explicit Value(uint64_t bits) : bits_(bits) {}
    
public:
    // 默认构造函数 - nil
    Value() : bits_(NIL_BITS) {}
    
    // 从bool构造
    Value(bool b) : bits_(b ? TRUE_BITS : FALSE_BITS) {}
    
    // 从double构造
    Value(double num) : bits_(encodeDouble(num)) {}
    
    // 从Obj*构造
    Value(Obj* obj) : bits_(obj == nullptr ? NIL_BITS : encodeObj(obj)) {}
    
    // 拷贝构造函数
    Value(const Value& other) = default;
    
    // 赋值运算符
    Value& operator=(const Value& other) = default;
    
    // 类型检查
    [[nodiscard]] bool isNil() const { return bits_ == NIL_BITS; }
    [[nodiscard]] bool isBool() const { return (bits_ | 1) == TRUE_BITS; }
    [[nodiscard]] bool isNumber() const { return (bits_ & QNAN_MASK) != QNAN_MASK; }
    [[nodiscard]] bool isObj() const { return (bits_ & (QNAN_MASK | TAG_MASK)) == OBJ_MASK; }
    [[nodiscard]] bool isObjType(ObjType type) const;
    
    // 类型转换
    [[nodiscard]] bool asBool() const {
        MS_ASSERT(isBool(), "Value is not a bool");
        return bits_ == TRUE_BITS;
    }
    
    [[nodiscard]] double asNumber() const {
        MS_ASSERT(isNumber(), "Value is not a number");
        return decodeDouble(bits_);
    }
    
    [[nodiscard]] Obj* asObj() const {
        MS_ASSERT(isObj(), "Value is not an object");
        return decodeObj(bits_);
    }
    
    template<typename T>
    [[nodiscard]] T* as() const {
        return static_cast<T*>(asObj());
    }
    
    // 获取内部位表示（用于调试）
    [[nodiscard]] uint64_t bits() const { return bits_; }
    
    // 相等比较
    [[nodiscard]] bool equals(const Value& other) const {
        // 对于数字，使用位级比较处理NaN
        if (isNumber() && other.isNumber()) {
            return asNumber() == other.asNumber();
        }
        return bits_ == other.bits_;
    }
    
    bool operator==(const Value& other) const { return equals(other); }
    bool operator!=(const Value& other) const { return !equals(other); }
    
private:
    // 编码/解码辅助函数
    static uint64_t encodeDouble(double num) {
        uint64_t bits;
        static_assert(sizeof(bits) == sizeof(num), "Size mismatch");
        std::memcpy(&bits, &num, sizeof(num));
        return bits;
    }
    
    static double decodeDouble(uint64_t bits) {
        double num;
        static_assert(sizeof(bits) == sizeof(num), "Size mismatch");
        std::memcpy(&num, &bits, sizeof(bits));
        return num;
    }
    
    static uint64_t encodeObj(Obj* obj) {
        return OBJ_MASK | reinterpret_cast<uint64_t>(obj);
    }
    
    static Obj* decodeObj(uint64_t bits) {
        return reinterpret_cast<Obj*>(bits & ~OBJ_MASK);
    }
};

// 辅助函数：创建特定类型的值
[[nodiscard]] inline Value nilVal() { return Value(); }
[[nodiscard]] inline Value boolVal(bool b) { return Value(b); }
[[nodiscard]] inline Value numberVal(double n) { return Value(n); }
[[nodiscard]] inline Value objVal(Obj* obj) { return Value(obj); }

// 类型检查辅助函数
[[nodiscard]] inline bool isNil(const Value& value) { return value.isNil(); }
[[nodiscard]] inline bool isBool(const Value& value) { return value.isBool(); }
[[nodiscard]] inline bool isNumber(const Value& value) { return value.isNumber(); }
[[nodiscard]] inline bool isObj(const Value& value) { return value.isObj(); }
[[nodiscard]] inline bool isString(const Value& value) { return value.isObjType(ObjType::String); }

// 值转换辅助函数
[[nodiscard]] inline bool asBool(const Value& value) { return value.asBool(); }
[[nodiscard]] inline double asNumber(const Value& value) { return value.asNumber(); }
[[nodiscard]] inline Obj* asObj(const Value& value) { return value.asObj(); }
[[nodiscard]] inline ObjString* asString(const Value& value) { return value.as<ObjString>(); }

// 打印值
void printValue(const Value& value);

// 值的哈希函数（用于哈希表）
[[nodiscard]] uint32_t hashValue(const Value& value);

} // namespace ms
