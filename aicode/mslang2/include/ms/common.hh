#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cfloat>
#include <cmath>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <variant>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef ERROR
#undef ERROR
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DEBUG
#undef DEBUG
#endif

namespace ms {

#if defined(NDEBUG)
    #define MS_ASSERT(condition) ((void)0)
    #define MS_ASSERT_MSG(condition, msg) ((void)0)
#else
    #define MS_ASSERT(condition) \
        ((condition) ? (void)0 : ms::assertion_failed(#condition, __FILE__, __LINE__))
    #define MS_ASSERT_MSG(condition, msg) \
        ((condition) ? (void)0 : ms::assertion_failed(msg, __FILE__, __LINE__))
#endif

void assertion_failed(const char* condition, const char* file, int line);
void assertion_failed(const char* msg, const char* file, int line);

class Scanner;
class Compiler;
class VM;
class Memory;
class Object;
class Value;
class Table;
class Chunk;
class Debug;
class Logger;
class Import;

}
