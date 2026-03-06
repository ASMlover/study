/**
 * @file test_phase2.cpp
 * @brief Phase 2 单元测试 - 动态数组、值类型系统、内存管理、字符串对象
 */

#include <cassert>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

// 包含所有阶段2头文件
#include "../../src/common.hh"
#include "../../src/dynarray.hh"
#include "../../src/memory.hh"
#include "../../src/value.hh"
#include "../../src/object.hh"

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running test: " #name "... "; \
    test_##name(); \
    std::cout << "PASSED\n"; \
} while(0)

using namespace ms;

// ============================================================================
// DynArray 测试
// ============================================================================

TEST(dynarray_basic) {
    DynArray<int> arr;
    assert(arr.count() == 0);
    assert(arr.capacity() == 0);
    assert(arr.isEmpty());
    
    arr.write(10);
    arr.write(20);
    arr.write(30);
    
    assert(arr.count() == 3);
    assert(!arr.isEmpty());
    assert(arr[0] == 10);
    assert(arr[1] == 20);
    assert(arr[2] == 30);
    
    // 修改元素
    arr[1] = 25;
    assert(arr[1] == 25);
}

TEST(dynarray_iterator) {
    DynArray<int> arr;
    arr.write(1);
    arr.write(2);
    arr.write(3);
    
    // 测试 begin/end 迭代器
    int sum = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        sum += *it;
    }
    assert(sum == 6);
    
    // 测试范围for循环
    sum = 0;
    for (int val : arr) {
        sum += val;
    }
    assert(sum == 6);
    
    // 测试const迭代器
    const DynArray<int>& carr = arr;
    sum = 0;
    for (auto it = carr.begin(); it != carr.end(); ++it) {
        sum += *it;
    }
    assert(sum == 6);
}

TEST(dynarray_grow) {
    DynArray<int> arr;
    
    // 添加元素触发扩容
    for (int i = 0; i < 20; i++) {
        arr.write(i);
    }
    
    assert(arr.count() == 20);
    // 容量应该 >= 20
    assert(arr.capacity() >= 20);
    
    // 验证所有元素
    for (int i = 0; i < 20; i++) {
        assert(arr[i] == i);
    }
}

TEST(dynarray_clear) {
    DynArray<int> arr;
    arr.write(1);
    arr.write(2);
    arr.write(3);
    
    usize oldCapacity = arr.capacity();
    arr.clear();
    
    assert(arr.count() == 0);
    assert(arr.isEmpty());
    // clear不释放内存，容量应保持不变
    assert(arr.capacity() == oldCapacity);
    
    // 可以重新添加元素
    arr.write(4);
    assert(arr.count() == 1);
    assert(arr[0] == 4);
}

TEST(dynarray_move) {
    DynArray<int> arr1;
    arr1.write(1);
    arr1.write(2);
    arr1.write(3);
    
    // 测试移动构造
    DynArray<int> arr2(std::move(arr1));
    assert(arr2.count() == 3);
    assert(arr2[0] == 1);
    assert(arr2[1] == 2);
    assert(arr2[2] == 3);
    
    // arr1应该被置空
    assert(arr1.count() == 0);
    assert(arr1.capacity() == 0);
    assert(arr1.isEmpty());
    
    // 测试移动赋值
    DynArray<int> arr3;
    arr3.write(100);
    arr3 = std::move(arr2);
    
    assert(arr3.count() == 3);
    assert(arr3[0] == 1);
    assert(arr3[1] == 2);
    assert(arr3[2] == 3);
    
    assert(arr2.count() == 0);
    assert(arr2.capacity() == 0);
}

TEST(dynarray_with_value) {
    // 测试存储Value类型的DynArray
    DynArray<Value> arr;
    arr.write(nilVal());
    arr.write(boolVal(true));
    arr.write(numberVal(3.14));
    
    assert(arr.count() == 3);
    assert(arr[0].isNil());
    assert(arr[1].isBool());
    assert(arr[1].asBool() == true);
    assert(arr[2].isNumber());
    assert(arr[2].asNumber() == 3.14);
}

// ============================================================================
// Value 类型系统测试
// ============================================================================

TEST(value_nil) {
    Value v1;  // 默认构造为nil
    assert(v1.isNil());
    assert(!v1.isBool());
    assert(!v1.isNumber());
    assert(!v1.isObj());
    
    Value v2 = nilVal();
    assert(v2.isNil());
    
    // nil相等比较
    assert(v1.equals(v2));
    assert(v1 == v2);
}

TEST(value_bool) {
    Value v1(true);
    assert(v1.isBool());
    assert(!v1.isNil());
    assert(!v1.isNumber());
    assert(!v1.isObj());
    assert(v1.asBool() == true);
    
    Value v2(false);
    assert(v2.isBool());
    assert(v2.asBool() == false);
    
    Value v3 = boolVal(true);
    assert(v3.isBool());
    assert(v3.asBool() == true);
    
    // bool相等比较
    assert(v1 == v3);
    assert(v1 != v2);
    assert(v1 == Value(true));
    assert(v2 == Value(false));
}

TEST(value_number) {
    Value v1(3.14);
    assert(v1.isNumber());
    assert(!v1.isNil());
    assert(!v1.isBool());
    assert(!v1.isObj());
    assert(v1.asNumber() == 3.14);
    
    Value v2(42.0);
    assert(v2.asNumber() == 42.0);
    
    Value v3 = numberVal(-123.456);
    assert(v3.isNumber());
    assert(v3.asNumber() == -123.456);
    
    // 特殊数值
    Value v4(0.0);
    assert(v4.asNumber() == 0.0);
    
    Value v5(-0.0);
    assert(v5.asNumber() == -0.0);
    
    // number相等比较
    assert(v1 == Value(3.14));
    assert(v1 != v2);
    assert(v4 == v4);  // 自反性
}

TEST(value_object) {
    ObjString* str = copyString("hello", 5);
    Value v1(str);
    
    assert(v1.isObj());
    assert(v1.isObjType(ObjType::String));
    assert(!v1.isNil());
    assert(!v1.isBool());
    assert(!v1.isNumber());
    
    assert(v1.asObj() == reinterpret_cast<Obj*>(str));
    assert(v1.as<ObjString>() == str);
    
    // 从null指针构造应得到nil
    Value v2(static_cast<Obj*>(nullptr));
    assert(v2.isNil());
    
    // obj相等比较（比较的是指针）
    ObjString* str2 = copyString("hello", 5);  // 相同内容，驻留后返回同一指针
    Value v3(str2);
    assert(v1 == v3);  // 相同指针
    
    ObjString* str3 = copyString("world", 5);
    Value v4(str3);
    assert(v1 != v4);  // 不同指针
}

TEST(value_nan_boxing) {
    // 验证NaN Boxing：Value应该只有8字节
    static_assert(sizeof(Value) == sizeof(uint64_t));
    assert(sizeof(Value) == 8);
    
    // 验证不同类型可以正确存储和恢复
    Value nil_v;
    Value bool_v(true);
    Value num_v(3.14159);
    
    // 通过bits()检查编码（调试用）
    uint64_t nil_bits = nil_v.bits();
    uint64_t true_bits = bool_v.bits();
    uint64_t num_bits = num_v.bits();
    
    // nil的bits应该是特定的编码值
    assert(nil_bits == 0x7FFC000000000001ULL);
    
    // true的bits应该是特定的编码值
    assert(true_bits == 0x7FFC000000000003ULL);
    
    // 数字直接存储其位表示
    double num = 3.14159;
    uint64_t expected_num_bits;
    std::memcpy(&expected_num_bits, &num, sizeof(num));
    assert(num_bits == expected_num_bits);
}

TEST(value_equality) {
    // nil相等
    assert(nilVal() == nilVal());
    
    // bool相等
    assert(boolVal(true) == boolVal(true));
    assert(boolVal(false) == boolVal(false));
    assert(boolVal(true) != boolVal(false));
    
    // number相等
    assert(numberVal(1.0) == numberVal(1.0));
    assert(numberVal(1.0) != numberVal(2.0));
    
    // 不同类型不等
    assert(nilVal() != boolVal(false));
    assert(boolVal(true) != numberVal(1.0));
    assert(numberVal(0.0) != nilVal());
    
    // 边界情况：NaN比较（IEEE 754规定NaN != NaN）
    // Value的equals使用asNumber()比较，所以NaN != NaN
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    Value nan1(nan_val);
    Value nan2(nan_val);
    // NaN不等于自身（IEEE 754行为）
    assert(nan1 != nan2);
    // 但可以通过bits验证它们是相同的NaN值
    assert(nan1.bits() == nan2.bits());
}

TEST(value_hash) {
    // nil的哈希
    assert(hashValue(nilVal()) == 0);
    
    // bool的哈希
    assert(hashValue(boolVal(false)) == 0);
    assert(hashValue(boolVal(true)) == 1);
    
    // number的哈希
    uint32_t h1 = hashValue(numberVal(3.14));
    uint32_t h2 = hashValue(numberVal(3.14));
    assert(h1 == h2);  // 相同值的哈希相同
    
    uint32_t h3 = hashValue(numberVal(2.71));
    assert(h1 != h3);  // 不同值的哈希不同
    
    // string的哈希
    ObjString* s1 = copyString("test", 4);
    Value v1(s1);
    assert(hashValue(v1) == s1->hash);
}

// ============================================================================
// 内存管理测试
// ============================================================================

TEST(memory_reallocate) {
    // 获取初始分配量
    size_t initialBytes = getBytesAllocated();
    
    // 分配内存
    int* ptr = static_cast<int*>(reallocate(nullptr, 0, sizeof(int) * 10));
    assert(ptr != nullptr);
    
    // 写入数据
    for (int i = 0; i < 10; i++) {
        ptr[i] = i;
    }
    
    // 重新分配（扩大）
    int* newPtr = static_cast<int*>(reallocate(ptr, sizeof(int) * 10, sizeof(int) * 20));
    assert(newPtr != nullptr);
    
    // 检查原有数据
    for (int i = 0; i < 10; i++) {
        assert(newPtr[i] == i);
    }
    
    // 释放内存
    void* freed = reallocate(newPtr, sizeof(int) * 20, 0);
    assert(freed == nullptr);
}

TEST(memory_stats) {
    // 记录初始状态
    size_t initialBytes = getBytesAllocated();
    
    // 分配一些内存
    void* ptr1 = reallocate(nullptr, 0, 100);
    size_t afterAlloc = getBytesAllocated();
    
    // 重新分配
    void* ptr2 = reallocate(ptr1, 100, 200);
    size_t afterRealloc = getBytesAllocated();
    
    // 释放
    reallocate(ptr2, 200, 0);
    size_t afterFree = getBytesAllocated();
    
    // 验证统计
    assert(afterAlloc >= initialBytes + 100);
    assert(afterFree == initialBytes);
}

TEST(memory_grow_capacity) {
    // 测试容量增长策略
    assert(growCapacity(0) == 8);    // 小于8时返回8
    assert(growCapacity(5) == 8);    // 小于8时返回8
    assert(growCapacity(8) == 12);   // 8 * 1.5 = 12
    assert(growCapacity(16) == 24);  // 16 * 1.5 = 24
    assert(growCapacity(100) == 150); // 100 * 1.5 = 150
}

// ============================================================================
// 字符串对象测试
// ============================================================================

TEST(string_create) {
    ObjString* str = copyString("hello", 5);
    
    assert(str != nullptr);
    assert(str->type == ObjType::String);
    assert(str->length == 5);
    assert(std::strncmp(str->chars, "hello", 5) == 0);
    assert(str->chars[5] == '\0');  // null终止
    assert(str->hash != 0);  // 应有预计算的哈希值
}

TEST(string_interning) {
    // 相同内容的字符串应该返回同一指针
    ObjString* s1 = copyString("intern", 6);
    ObjString* s2 = copyString("intern", 6);
    
    assert(s1 == s2);  // 同一指针（驻留）
    assert(s1->hash == s2->hash);
    
    // 不同内容应该返回不同指针
    ObjString* s3 = copyString("other", 5);
    assert(s1 != s3);
    
    // 相同哈希但不同内容的情况
    ObjString* s4 = copyString("hello", 5);
    ObjString* s5 = copyString("world", 5);
    assert(s4 != s5);
}

TEST(string_concat) {
    ObjString* hello = copyString("Hello, ", 7);
    ObjString* world = copyString("World!", 6);
    
    ObjString* result = concatenateStrings(hello, world);
    
    assert(result != nullptr);
    assert(result->length == 13);
    assert(std::string(result->chars) == "Hello, World!");
    
    // 结果应该也被驻留
    ObjString* same = copyString("Hello, World!", 13);
    assert(result == same);
}

TEST(string_as_view) {
    ObjString* str = copyString("view test", 9);
    std::string_view sv = str->asView();
    
    assert(sv.size() == 9);
    assert(sv == "view test");
}

TEST(string_empty) {
    // 空字符串
    ObjString* empty = copyString("", 0);
    assert(empty != nullptr);
    assert(empty->length == 0);
    assert(empty->chars[0] == '\0');
    
    // 再次获取空字符串应返回同一指针
    ObjString* empty2 = copyString("", 0);
    assert(empty == empty2);
}

TEST(string_special_chars) {
    // 包含特殊字符的字符串
    ObjString* special = copyString("Hello\nWorld\t!", 13);
    assert(special->length == 13);
    assert(std::strncmp(special->chars, "Hello\nWorld\t!", 13) == 0);
    
    // 包含空字符的字符串
    ObjString* withNull = copyString("ab\0cd", 5);
    assert(withNull->length == 5);
    assert(withNull->chars[0] == 'a');
    assert(withNull->chars[1] == 'b');
    assert(withNull->chars[2] == '\0');
    assert(withNull->chars[3] == 'c');
    assert(withNull->chars[4] == 'd');
}

TEST(string_hash_consistency) {
    // 相同内容的字符串哈希值应该相同
    ObjString* s1 = copyString("consistent", 10);
    ObjString* s2 = copyString("consistent", 10);
    
    assert(s1->hash == s2->hash);
    
    // 不同内容的字符串哈希值应该不同（极大概率）
    ObjString* s3 = copyString("different", 9);
    assert(s1->hash != s3->hash);
}

TEST(string_from_string_view) {
    // 使用string_view创建字符串
    std::string_view sv = "string_view test";
    ObjString* str = copyString(sv);
    
    assert(str->length == sv.length());
    assert(str->asView() == sv);
}

// ============================================================================
// 集成测试
// ============================================================================

TEST(integration_value_with_string) {
    // 将字符串对象包装为Value
    ObjString* str = copyString("integrated", 10);
    Value v = objVal(str);
    
    assert(v.isObj());
    assert(isString(v));
    assert(asString(v) == str);
    
    // 通过Value打印
    std::cout << "[String value: ";
    printValue(v);
    std::cout << "] ";
}

TEST(integration_dynarray_with_string_values) {
    DynArray<Value> arr;
    
    arr.write(objVal(copyString("one", 3)));
    arr.write(objVal(copyString("two", 3)));
    arr.write(objVal(copyString("three", 5)));
    
    assert(arr.count() == 3);
    assert(isString(arr[0]));
    assert(isString(arr[1]));
    assert(isString(arr[2]));
    
    assert(asString(arr[0])->asView() == "one");
    assert(asString(arr[1])->asView() == "two");
    assert(asString(arr[2])->asView() == "three");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "=== Phase 2 Unit Tests ===\n\n";
    
    // DynArray测试
    std::cout << "--- DynArray Tests ---\n";
    RUN_TEST(dynarray_basic);
    RUN_TEST(dynarray_iterator);
    RUN_TEST(dynarray_grow);
    RUN_TEST(dynarray_clear);
    RUN_TEST(dynarray_move);
    RUN_TEST(dynarray_with_value);
    
    std::cout << "\n--- Value Tests ---\n";
    // Value测试
    RUN_TEST(value_nil);
    RUN_TEST(value_bool);
    RUN_TEST(value_number);
    RUN_TEST(value_object);
    RUN_TEST(value_nan_boxing);
    RUN_TEST(value_equality);
    RUN_TEST(value_hash);
    
    std::cout << "\n--- Memory Tests ---\n";
    // 内存管理测试
    RUN_TEST(memory_reallocate);
    RUN_TEST(memory_stats);
    RUN_TEST(memory_grow_capacity);
    
    std::cout << "\n--- String Tests ---\n";
    // 字符串测试
    RUN_TEST(string_create);
    RUN_TEST(string_interning);
    RUN_TEST(string_concat);
    RUN_TEST(string_as_view);
    RUN_TEST(string_empty);
    RUN_TEST(string_special_chars);
    RUN_TEST(string_hash_consistency);
    RUN_TEST(string_from_string_view);
    
    std::cout << "\n--- Integration Tests ---\n";
    // 集成测试
    RUN_TEST(integration_value_with_string);
    RUN_TEST(integration_dynarray_with_string_values);
    
    std::cout << "\n=== All tests PASSED ===\n";
    return 0;
}
