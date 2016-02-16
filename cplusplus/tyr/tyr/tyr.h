// Copyright (c) 2015 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef __TYR_HEADER_H__
#define __TYR_HEADER_H__

#if !defined(TYR_OS_WIN) || !defined(TYR_OS_LINUX) || !defined(TYR_OS_MAC)
# undef TYR_OS_NAME
# undef TYR_OS_WIN
# undef TYR_OS_LINUX
# undef TYR_OS_MAC
# if defined(_WINDOWS_) || defined(_WIN32) || defined(_WIN64)\
  || defined(__MINGW32__) || defined(__MINGW64__)
#   define TYR_OS_NAME "Windows"
#   define TYR_OS_WIN
# elif defined(__linux__)
#   define TYR_OS_NAME "Linux"
#   define TYR_OS_LINUX
# elif defined(__APPPLE__) || defined(__MACH__)
#   define TYR_OS_NAME "Mac"
#   define TYR_OS_MAC
# else
#   define TYR_OS_NAME "Unknown"
#   error "DOES NOT SUPPORT UNKNOWN PLATFORM !!!"
# endif
#endif

#if !defined(TYR_CC_VC) || !defined(TYR_CC_GCC) || !defined(TYR_CC_CLANG)
# undef TYR_CC_NAME
# undef TYR_CC_VC
# undef TYR_CC_GCC
# undef TYR_CC_CLANG
# if defined(TYR_OS_WIN) && defined(_MSC_VER)
#   define TYR_CC_NAME "VC"
#   define TYR_CC_VC
# elif defined(TYR_OS_LINUX) && defined(__GNUC__)
#   define TYR_CC_NAME "GCC"
#   define TYR_CC_GCC
# elif defined(TYR_OS_MAC) && defined(__clang__)
#   define TYR_CC_NAME "Clang"
#   define TYR_CC_CLANG
# else
#   define TYR_CC_NAME "Unknown"
#   error "DOES NOT SUPPORT UNKNOWN COMPILER !!!"
# endif
#endif

#if defined(TYR_CC_VC)
# define TYR_CC_VER       _MSC_VER
# define TYR_CC_VER_LIMIT 1700
#elif defined(TYR_CC_GCC)
# define TYR_CC_VER       (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
# define TYR_CC_VER_LIMIT 470
#elif defined(TYR_CC_CLANG)
# define TYR_CC_VER       (__clang_major__ * 100 + __clang_minor__ * 10)
# define TYR_CC_VER_LIMIT 330
#endif

#if TYR_CC_VER < TYR_CC_VER_LIMIT
# error "PLEASE USE A HIGHER VERSION OF COMPILER !!!"
#endif

#if defined(TYR_CC_VC)
# define tyr_noexcept throw()
#else
# define tyr_noexcept noexcept
#endif

#define TYR_CPP0X 1

#if TYR_CPP0X != 1
# if defined(TYR_OS_MAC)
#   define TYR_MAC_SPINLOCK 1
# endif
#endif

// SYSTEM INTERFACES INCLUDING HEADERS
#if defined(TYR_OS_WIN)
# include <Windows.h>
# include <direct.h>
# include <io.h>
# include <process.h>

# undef __func__
# define __func__ __FUNCSIG__
#else
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <fcntl.h>
# include <limits.h>
# include <pthread.h>
# include <unistd.h>

# define MAX_PATH PATH_MAX
# if defined(TYR_OS_MAC)
#   include <libkern/OSAtomic.h>
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#   include <mach/mach_traps.h>
#   include <mach/semaphore.h>
#   include <mach/task.h>
# else
#   include <semaphore.h>
# endif
#endif

// ANSI C HEADERS
#include <assert.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI C++ HEADERS
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

// STL HEADERS
#include <algorithm>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

typedef unsigned char byte_t;

#define TYR_ASSERT(condition) do {\
  if (!(condition)) {\
    fprintf(stderr,\
        "[%s:%d] Assertion failed in %s(): %s\n",\
        __FILE__,\
        __LINE__,\
        __func__,\
        #condition);\
    fflush(stderr);\
    abort();\
  }\
} while (0)

#include "tyr_uncopyable.h"
#include "thread/tyr_thread.h"
#include "memory/tyr_memory.h"
#include "utils/tyr_utils.h"

#endif  // __TYR_HEADER_H__
