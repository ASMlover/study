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
#ifndef __EUTIL_HEADER_H__
#define __EUTIL_HEADER_H__

#if !defined(EUTIL_OS_WIN) || !defined(EUTIL_OS_LINUX) || \
    !defined(EUTIL_OS_MAC)
# undef EUTIL_OS_WIN
# undef EUTIL_OS_LINUX
# undef EUTIL_OS_MAC

# if defined(__WINDOWS__) || defined(_MSC_VER) || defined(_WIN32) || \
     defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
     defined(__WIN32__) || defined(__TOS_WIN__) || defined(__MINGW32) || \
     defined(__MINGW64) || defined(__MINGW32__) || defined(__MINGW64__)
#   define EUTIL_OS_NAME "Windows"
#   define EUTIL_OS_WIN
# elif defined(__linux__) || defined(linux) || defined(__linux) || \
     defined(__LINUX__) || defined(LINUX) || defined(_LINUX)
#   define EUTIL_OS_NAME "Linux"
#   define EUTIL_OS_LINUX
# elif defined(__APPLE__) || defined(__apple__) || defined(__MACH__) || \
     defined(__MACOSX__)
#   define EUTIL_OS_NAME "Mac"
#   define EUTIL_OS_MAC
# else
#   define EUTIL_OS_NAME "Unknown"
#   error "DOES NOT SUPPORT THIS PLATFORM !!!"
# endif
#endif

#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
    defined(__amd64) || defined(__LP64__) || defined(_LP64) || \
    defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(__ia64__) || defined(_IA64) || defined(__IA64__) || \
    defined(__ia64) || defined(_M_IA64)
# define EUTIL_BITS_NAME "64"
# define EUTIL_BITS_64
#elif defined(_WIN32) || defined(WIN32) || defined(__32BIT__) || \
    defined(__ILP32__) || defined(_ILP32) || defined(i386) || \
    defined(__i386__) || defined(__i486__) || defined(__i586__) || \
    defined(__i686__) || defined(__i386) || defined(_M_IX86) || \
    defined(__X86__) || defined(_X86_) || defined(__I86__)
# define EUTIL_BITS_NAME "32"
# define EUTIL_BITS_32
#else
# define EUTIL_BITS_NAME "Unknown"
# error "DOES NOT SUPPORT THIS BITS !!!"
#endif

#if defined(_MSC_VER)
# define EUTIL_COMPILER_NAME "MSVC"
# define EUTIL_COMPILER_MSVC
#elif defined(__MINGW32) || defined(__MINGW64) || \
    defined(__MINGW32__) || defined(__MINGW64__)
# define EUTIL_COMPILER_NAME "MINGW"
# define EUTIL_COMPILER_MINGW
# define EUTIL_COMPILER_GCC
#elif defined(__GNUG__) || defined(__GNUC__)
# define EUTIL_COMPILER_NAME "GCC"
# define EUTIL_COMPILER_GCC
#elif defined(__clang__) || defined(__CLANG__)
# define EUTIL_COMPILER_NAME "CLANG"
# define EUTIL_COMPILER_CLANG
#else
# define EUTIL_COMPILER_NAME "Unknown"
# error "UNKNOWN COMPILER !!!"
#endif

#if defined(EUTIL_COMPILER_GCC)
# define EUTIL_GCC_VER    (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#elif defined(EUTIL_COMPILER_CLANG)
# define EUTIL_CLANG_VER  (__clang_major__ * 100 + __clang_minor__ * 10)
#endif

#if (defined(EUTIL_COMPILER_MSVC) && (_MSC_VER < 1700)) || \
    (defined(EUTIL_COMPILER_GCC) && (EUTIL_GCC_VER < 470)) || \
    (defined(EUTIL_COMPILER_CLANG) && (EUTIL_CLANG_VER < 330))
# error "PLEASE USE A HIGHER VERSION OF THE COMPILER !!!"
#endif

// SYSTEM INTERFACES HEADERS
#if defined(EUTIL_OS_WIN)
# include <direct.h>
# include <io.h>
# include <mmsystem.h>
# include <process.h>
# include <windows.h>

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
# if defined(EUTIL_OS_MAC)
#   include <libkern/OSAtomic.h>
#   include <mach/mach.h>
#   include <mach/mach_time.h>
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
#include <codecvt>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

// STL HEADERS
#include <algorithm>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

#endif  // __EUTIL_HEADER_H__
