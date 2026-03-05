#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

namespace ms {

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;

constexpr usize STACK_MAX = 256;
constexpr usize FRAMES_MAX = 64;
constexpr usize GC_HEAP_GROW_FACTOR = 2;

struct DebugConfig {
#ifdef DEBUG_SCANNER
    static constexpr bool SCANNER = true;
#else
    static constexpr bool SCANNER = false;
#endif
#ifdef DEBUG_PARSER
    static constexpr bool PARSER = true;
#else
    static constexpr bool PARSER = false;
#endif
#ifdef DEBUG_COMPILE
    static constexpr bool COMPILE = true;
#else
    static constexpr bool COMPILE = false;
#endif
#ifdef DEBUG_TRACE_EXEC
    static constexpr bool TRACE_EXEC = true;
#else
    static constexpr bool TRACE_EXEC = false;
#endif
#ifdef DEBUG_STRESS_GC
    static constexpr bool STRESS_GC = true;
#else
    static constexpr bool STRESS_GC = false;
#endif
#ifdef DEBUG_LOG_GC
    static constexpr bool LOG_GC = true;
#else
    static constexpr bool LOG_GC = false;
#endif
};

#define MS_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "[ASSERTION FAILED] %s:%d: %s\n  Condition: %s\n", __FILE__, __LINE__, (msg), #cond); \
            std::abort(); \
        } \
    } while (false)

#define MS_DISABLE_COPY(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete

#define MS_DISABLE_MOVE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete

#define MS_DISABLE_COPY_AND_MOVE(ClassName) \
    MS_DISABLE_COPY(ClassName); \
    MS_DISABLE_MOVE(ClassName)

template <typename T, usize N>
constexpr usize arraySize(const T (&)[N]) { return N; }

constexpr usize divCeil(usize numerator, usize denominator) {
    return (numerator + denominator - 1) / denominator;
}

constexpr usize alignUp(usize size, usize alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

}
