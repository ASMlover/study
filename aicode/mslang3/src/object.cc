#include "object.hh"

#include "value.hh"
#include "logger.hh"

#include <cstdio>
#include <cstring>

namespace ms {

// ============================================================================
// 字符串驻留池
// ============================================================================

// 使用哈希表实现字符串驻留：哈希值 -> 相同哈希的字符串列表
// 使用开放寻址处理哈希冲突
static std::unordered_map<u32, std::vector<ObjString*>> stringTable;

// 全局对象链表头（GC用）
static Obj* objectsHead = nullptr;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief FNV-1a哈希算法
 */
static u32 hashString(const char* key, size_t length) {
    u32 hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= static_cast<u8>(key[i]);
        hash *= 16777619;
    }
    return hash;
}

/**
 * @brief 在驻留池中查找相同内容的字符串
 */
static ObjString* findInternedString(const char* chars, size_t length, u32 hash) {
    auto it = stringTable.find(hash);
    if (it == stringTable.end()) {
        return nullptr;
    }
    
    // 检查相同哈希的字符串列表
    for (ObjString* str : it->second) {
        if (str->length == length && std::memcmp(str->chars, chars, length) == 0) {
            return str;  // 找到已驻留的字符串
        }
    }
    
    return nullptr;
}

/**
 * @brief 将字符串加入驻留池
 */
static void internString(ObjString* str) {
    stringTable[str->hash].push_back(str);
}

/**
 * @brief 将对象加入GC链表
 */
static void addToObjList(Obj* obj) {
    obj->next = objectsHead;
    objectsHead = obj;
}

// ============================================================================
// Obj实现
// ============================================================================

Obj::Obj(ObjType type) 
    : type(type)
    , isMarked(false)
    , next(nullptr) {
    addToObjList(this);
}

// ============================================================================
// ObjString实现
// ============================================================================

ObjString::ObjString(const char* sourceChars, size_t length, u32 hash)
    : Obj(ObjType::String)
    , length(length)
    , hash(hash) {
    // 分配内存并复制字符串内容（包含结尾的\0）
    chars = ALLOCATE(char, length + 1);
    std::memcpy(chars, sourceChars, length);
    chars[length] = '\0';
}

ObjString::~ObjString() {
    // 释放字符数组内存
    FREE_ARRAY(char, chars, length + 1);
}

// ============================================================================
// 字符串操作函数实现
// ============================================================================

ObjString* copyString(const char* chars, size_t length) {
    // 计算哈希值
    u32 hash = hashString(chars, length);
    
    // 检查是否已存在相同内容的字符串
    ObjString* interned = findInternedString(chars, length, hash);
    if (interned != nullptr) {
        return interned;
    }
    
    // 创建新字符串
    ObjString* str = new ObjString(chars, length, hash);
    
    // 加入驻留池
    internString(str);
    
    #ifdef DEBUG_LOG_GC
    Logger::debug("Allocated string: '{}' (hash: {}, length: {})", 
                  std::string_view(chars, length), hash, length);
    #endif
    
    return str;
}

ObjString* concatenateStrings(ObjString* a, ObjString* b) {
    size_t newLength = a->length + b->length;
    
    // 分配临时缓冲区
    char* buffer = ALLOCATE(char, newLength + 1);
    
    // 复制两个字符串
    std::memcpy(buffer, a->chars, a->length);
    std::memcpy(buffer + a->length, b->chars, b->length);
    buffer[newLength] = '\0';
    
    // 调用copyString进行驻留
    ObjString* result = copyString(buffer, newLength);
    
    // 释放临时缓冲区
    FREE_ARRAY(char, buffer, newLength + 1);
    
    return result;
}

const char* getObjTypeName(ObjType type) {
    switch (type) {
        case ObjType::String:       return "STRING";
        case ObjType::Function:     return "FUNCTION";
        case ObjType::Native:       return "NATIVE";
        case ObjType::Closure:      return "CLOSURE";
        case ObjType::Upvalue:      return "UPVALUE";
        case ObjType::Class:        return "CLASS";
        case ObjType::Instance:     return "INSTANCE";
        case ObjType::BoundMethod:  return "BOUND_METHOD";
        case ObjType::Module:       return "MODULE";
        default:                    return "UNKNOWN";
    }
}

void printObject(const Value& value) {
    // 这个函数需要在value.hh中Value类定义后才能完全实现
    // 这里先提供一个基础实现
    // 注意：由于Value类定义在value.hh中，我们需要包含它
    // 这个函数的具体实现在value.cc中会更完整
    
    // 简单处理字符串类型
    if (IS_STRING(value)) {
        ObjString* str = AS_STRING(value);
        std::printf("%.*s", static_cast<int>(str->length), str->chars);
    } else {
        std::printf("[object %s]", getObjTypeName(AS_OBJ(value)->type));
    }
}

// ============================================================================
// GC相关函数
// ============================================================================

/**
 * @brief 获取GC对象链表头（供GC模块使用）
 */
Obj* getObjectsHead() {
    return objectsHead;
}

/**
 * @brief 设置GC对象链表头（供GC模块使用）
 */
void setObjectsHead(Obj* head) {
    objectsHead = head;
}

/**
 * @brief 从驻留池中移除未标记的字符串（GC sweep阶段使用）
 */
void removeUnmarkedStrings() {
    for (auto& [hash, strings] : stringTable) {
        strings.erase(
            std::remove_if(strings.begin(), strings.end(),
                [](ObjString* str) { return !str->isMarked; }),
            strings.end()
        );
    }
    
    // 清理空桶
    for (auto it = stringTable.begin(); it != stringTable.end(); ) {
        if (it->second.empty()) {
            it = stringTable.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief 标记字符串对象（GC mark阶段使用）
 */
void markString(ObjString* str) {
    if (str == nullptr || str->isMarked) return;
    
    str->isMarked = true;
    
    #ifdef DEBUG_LOG_GC
    Logger::debug("GC mark string: '{}'", str->asView());
    #endif
}

} // namespace ms
