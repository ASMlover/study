#include "memory.hh"
#include "logger.hh"
#include <sstream>

namespace ms {

// 当前分配的字节数
static size_t bytesAllocated_ = 0;

// 下次GC触发阈值
static size_t nextGC_ = 1024 * 1024; // 初始1MB

size_t getBytesAllocated() {
    return bytesAllocated_;
}

void setNextGC(size_t next) {
    nextGC_ = next;
}

/**
 * 检查是否需要触发GC
 * 在DEBUG_STRESS_GC模式下，每次分配都触发GC
 * 正常情况下，当bytesAllocated超过nextGC时触发
 */
static void maybeTriggerGC(size_t newSize) {
    if constexpr (DebugConfig::STRESS_GC) {
        if (newSize > 0) {
            if constexpr (DebugConfig::LOG_GC) {
                Logger::debug("[GC] Stress GC triggered");
            }
            collectGarbage();
        }
    } else {
        if (bytesAllocated_ + newSize > nextGC_ && newSize > 0) {
            if constexpr (DebugConfig::LOG_GC) {
                std::ostringstream oss;
                oss << "[GC] Triggering GC: allocated=" << (bytesAllocated_ + newSize)
                    << ", nextGC=" << nextGC_;
                Logger::debug(oss.str());
            }
            collectGarbage();
        }
    }
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // 分配前检查是否需要GC
    if (newSize > oldSize) {
        maybeTriggerGC(newSize - oldSize);
    }

    // 释放内存
    if (newSize == 0) {
        std::free(pointer);
        bytesAllocated_ -= oldSize;
        return nullptr;
    }

    // 分配或重新分配内存
    void* result = std::realloc(pointer, newSize);
    
    if (result == nullptr) {
        std::ostringstream oss;
        oss << "Memory allocation failed: " << newSize << " bytes";
        Logger::fatal(oss.str());
        std::exit(1);
    }

    // 如果是新分配，清零新内存
    if (oldSize == 0) {
        std::memset(result, 0, newSize);
    }

    // 更新分配字节数统计
    bytesAllocated_ = bytesAllocated_ - oldSize + newSize;

    return result;
}

// GC函数的前向声明实现（供后续完整GC模块实现）
// 这里提供一个基本实现，防止链接错误
void collectGarbage() {
    // 占位实现，后续由完整GC模块替换
    // 目前仅调整GC阈值
    if constexpr (DebugConfig::LOG_GC) {
        Logger::debug("[GC] collectGarbage called (stub implementation)");
    }
    
    // 更新下次GC阈值
    nextGC_ = (bytesAllocated_ + 1) * GC_HEAP_GROW_FACTOR;
    
    if constexpr (DebugConfig::LOG_GC) {
        std::ostringstream oss;
        oss << "[GC] Updated nextGC to " << nextGC_;
        Logger::debug(oss.str());
    }
}

} // namespace ms
