/*
 * Copyright (c) 2024 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef LOXC_COMMON_H
#define LOXC_COMMON_H

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) ||\
  defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
# define LOXC_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define LOXC_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__MACH__)
# define LOXC_DARWIN
#else
# error "ERROR: Unknown Platform !!!"
#endif

#if !defined(LOXC_WINDOWS)
# define LOXC_POSIX
#endif

#if defined(i386) || defined(__i386) || defined(__i386__) || defined(__i486__) ||\
  defined(__i586__) || defined(__i686__) || defined(_M_IX86) || defined(_X86_) ||\
  defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__)
# define LOXC_ARCH32
#elif defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) ||\
  defined(__amd64) || defined(__amd64__)
# define LOXC_ARCH64
#else
# error "ERROR: Unknown Architecture !!!"
#endif

#if !defined(LOXC_UNUSED)
# define LOXC_UNUSED(x)   ((void)x)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UINT8_COUNT                         (UINT8_MAX + 1)

typedef uint8_t         byte_t;
typedef int8_t          i8_t;
typedef uint8_t         u8_t;
typedef int16_t         i16_t;
typedef uint16_t        u16_t;
typedef int32_t         i32_t;
typedef uint32_t        u32_t;
typedef int64_t         i64_t;
typedef uint64_t        u64_t;
typedef size_t          sz_t;
#if defined(LOXC_ARCH64)
  typedef int64_t       ssz_t;
#else
  typedef int32_t       ssz_t;
#endif
typedef const char*     cstr_t;

static inline u32_t hashString(const char* str, int len) {
  u32_t hash = 2166136261u;
  for (int i = 0; i < len; ++i) {
    hash ^= (u8_t)str[i];
    hash *= 16777619;
  }
  return hash;
}

#endif
