#include "object.hh"
#include "value.hh"
#include "logger.hh"

#include <cstdio>
#include <cstring>
#include <string>

using namespace ms;

// 简单的测试断言宏
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            Logger::error(std::string("Test failed: ") + (msg)); \
            std::printf("FAILED: %s\n", (msg)); \
            return false; \
        } \
    } while(0)

bool test_string_creation() {
    Logger::info("Testing string creation...");
    
    ObjString* s1 = copyString("hello", 5);
    TEST_ASSERT(s1 != nullptr, "String should not be null");
    TEST_ASSERT(s1->length == 5, "String length should be 5");
    TEST_ASSERT(std::strcmp(s1->chars, "hello") == 0, "String content should be 'hello'");
    TEST_ASSERT(s1->type == ObjType::String, "Type should be String");
    
    Logger::info("String creation test passed!");
    return true;
}

bool test_string_interning() {
    Logger::info("Testing string interning...");
    
    ObjString* s1 = copyString("hello", 5);
    ObjString* s2 = copyString("hello", 5);
    ObjString* s3 = copyString("world", 5);
    
    // 相同内容应该返回同一对象（驻留）
    TEST_ASSERT(s1 == s2, "Same strings should be interned (same pointer)");
    
    // 不同内容应该返回不同对象
    TEST_ASSERT(s1 != s3, "Different strings should have different pointers");
    
    // 使用string_view重载
    ObjString* s4 = copyString(std::string_view("hello"));
    TEST_ASSERT(s1 == s4, "String view should also be interned");
    
    Logger::info("String interning test passed!");
    return true;
}

bool test_string_concatenation() {
    Logger::info("Testing string concatenation...");
    
    ObjString* s1 = copyString("Hello, ", 7);
    ObjString* s2 = copyString("World!", 6);
    ObjString* concat = concatenateStrings(s1, s2);
    
    TEST_ASSERT(concat != nullptr, "Concatenated string should not be null");
    TEST_ASSERT(concat->length == 13, "Concatenated length should be 13");
    TEST_ASSERT(std::strcmp(concat->chars, "Hello, World!") == 0, "Concatenated content should be 'Hello, World!'");
    
    // 验证结果也被驻留
    ObjString* concat2 = concatenateStrings(s1, s2);
    TEST_ASSERT(concat == concat2, "Concatenated strings should also be interned");
    
    Logger::info("String concatenation test passed!");
    return true;
}

bool test_string_hash() {
    Logger::info("Testing string hash...");
    
    ObjString* s1 = copyString("test", 4);
    ObjString* s2 = copyString("test", 4);
    
    // 相同内容的字符串应该有相同哈希
    TEST_ASSERT(s1->hash == s2->hash, "Same strings should have same hash");
    
    // 哈希值不应为0（除非是空字符串）
    TEST_ASSERT(s1->hash != 0, "Non-empty string should have non-zero hash");
    
    Logger::info("String hash test passed!");
    return true;
}

bool test_obj_type_names() {
    Logger::info("Testing object type names...");
    
    TEST_ASSERT(std::strcmp(getObjTypeName(ObjType::String), "STRING") == 0, "String type name should be STRING");
    TEST_ASSERT(std::strcmp(getObjTypeName(ObjType::Function), "FUNCTION") == 0, "Function type name should be FUNCTION");
    TEST_ASSERT(std::strcmp(getObjTypeName(ObjType::Class), "CLASS") == 0, "Class type name should be CLASS");
    
    Logger::info("Object type names test passed!");
    return true;
}

bool test_string_as_view() {
    Logger::info("Testing string asView...");
    
    ObjString* s1 = copyString("hello", 5);
    std::string_view view = s1->asView();
    
    TEST_ASSERT(view.size() == 5, "View size should be 5");
    TEST_ASSERT(view == "hello", "View content should be 'hello'");
    
    Logger::info("String asView test passed!");
    return true;
}

int main() {
    Logger::setLevel(LogLevel::Info);
    
    std::printf("\n========================================\n");
    std::printf("Running Object System Tests\n");
    std::printf("========================================\n\n");
    
    int passed = 0;
    int failed = 0;
    
    if (test_string_creation()) passed++; else failed++;
    if (test_string_interning()) passed++; else failed++;
    if (test_string_concatenation()) passed++; else failed++;
    if (test_string_hash()) passed++; else failed++;
    if (test_obj_type_names()) passed++; else failed++;
    if (test_string_as_view()) passed++; else failed++;
    
    std::printf("\n========================================\n");
    std::printf("Results: %d passed, %d failed\n", passed, failed);
    std::printf("========================================\n\n");
    
    return failed > 0 ? 1 : 0;
}
