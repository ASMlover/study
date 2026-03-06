#pragma once

#include "common.hh"
#include <cstring>

namespace ms {

// 前向声明GC相关函数
void collectGarbage();
size_t getBytesAllocated();
void setNextGC(size_t next);

/**
 * 统一的内存分配/重新分配/释放接口
 * 
 * @param pointer 原指针（nullptr表示新分配）
 * @param oldSize 原大小（0表示新分配）
 * @param newSize 新大小（0表示释放）
 * @return 新分配的内存指针（如果newSize为0则返回nullptr）
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

// 容量增长策略：小于8时直接到8，否则按1.5倍增长
constexpr usize growCapacity(usize capacity) {
    return capacity < 8 ? 8 : static_cast<usize>(capacity * 1.5);
}

// GC相关宏

#define GROW_CAPACITY(capacity) \
    ms::growCapacity(capacity)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)ms::reallocate((pointer), sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    ms::reallocate((pointer), sizeof(type) * (oldCount), 0)

#define ALLOCATE(type, count) \
    (type*)ms::reallocate(nullptr, 0, sizeof(type) * (count))

} // namespace ms
