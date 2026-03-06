#pragma once

#include "common.hh"
#include "memory.hh"

#include <string_view>
#include <unordered_map>
#include <vector>

namespace ms {

// 前向声明 - 在value.hh中定义
class Value;

// 对象类型枚举
enum class ObjType : u8 {
    String,
    Function,
    Native,
    Closure,
    Upvalue,
    Class,
    Instance,
    BoundMethod,
    Module
};

// 对象基类 - 所有堆分配对象的基类
struct Obj {
    ObjType type;
    bool isMarked;      // GC标记
    Obj* next;          // GC链表（用于追踪所有对象）
    
    explicit Obj(ObjType type);
    virtual ~Obj() = default;
};

// 前向声明具体对象类型（供Value使用）
struct ObjString;
struct ObjFunction;
struct ObjNative;
struct ObjClosure;
struct ObjUpvalue;
struct ObjClass;
struct ObjInstance;
struct ObjBoundMethod;
struct ObjModule;

// 字符串对象
struct ObjString : Obj {
    size_t length;
    char* chars;
    u32 hash;           // 预计算哈希值
    
    // 构造函数（哈希由调用者提供或计算）
    ObjString(const char* chars, size_t length, u32 hash);
    ~ObjString() override;
    
    // 获取字符串内容作为string_view
    [[nodiscard]] std::string_view asView() const {
        return std::string_view(chars, length);
    }
};

// 字符串操作函数

/**
 * @brief 创建字符串（自动驻留）
 * @param chars 字符数组
 * @param length 字符长度
 * @return 驻留后的字符串对象
 */
ObjString* copyString(const char* chars, size_t length);

/**
 * @brief 创建字符串（自动驻留）
 * @param str 字符串视图
 * @return 驻留后的字符串对象
 */
inline ObjString* copyString(std::string_view str) {
    return copyString(str.data(), str.size());
}

/**
 * @brief 拼接两个字符串
 * @param a 第一个字符串
 * @param b 第二个字符串
 * @return 拼接后的新字符串（已驻留）
 */
ObjString* concatenateStrings(ObjString* a, ObjString* b);

/**
 * @brief 获取对象类型名称
 * @param type 对象类型
 * @return 类型名称字符串
 */
const char* getObjTypeName(ObjType type);

/**
 * @brief 打印对象
 * @param value 包含对象的值
 */
void printObject(const Value& value);

// ============================================================================
// GC相关函数（供memory模块使用）
// ============================================================================

/**
 * @brief 获取GC对象链表头
 */
Obj* getObjectsHead();

/**
 * @brief 设置GC对象链表头
 */
void setObjectsHead(Obj* head);

/**
 * @brief 从驻留池中移除未标记的字符串（GC sweep阶段使用）
 */
void removeUnmarkedStrings();

/**
 * @brief 标记字符串对象（GC mark阶段使用）
 */
void markString(ObjString* str);

} // namespace ms

// ============================================================================
// 类型检查宏（需要在value.hh定义后使用）
// ============================================================================

// 这些宏依赖于Value类的实现，将在value.hh中提供对应的isString/asString函数
// 这里先声明宏，实际的函数声明在value.hh中

#define IS_STRING(value)     ((value).isObjType(ObjType::String))
#define AS_STRING(value)     ((value).as<ObjString>())
#define AS_CSTRING(value)    ((value).as<ObjString>()->chars)
#define AS_OBJ(value)        ((value).asObj())
