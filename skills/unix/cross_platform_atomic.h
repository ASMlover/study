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
#ifndef __CROSS_PLATFORM_ATOMIC_HEADER_H__
#define __CROSS_PLATFORM_ATOMIC_HEADER_H__

#if defined(_WINDOWS_) || defined(_MSC_VER) || defined(WIN32) || defined(WIN64)
# define CROSS_WIN
#endif

#include <stdint.h>

#if defined(CROSS_WIN)
inline uint32_t atomic_inc(void* var) {
  return InterlockedIncrement(static_cast<long*>(var));
}

inline uint32_t atomic_dec(void* var) {
  return InterlockedDecrement(static_cast<long*>(var));
}

inline uint32_t atomic_add(void* var, uint32_t value) {
  return InterlockedExchangeAdd(static_cast<long*>(var), value);
}

inline uint32_t atomic_sub(void* var, int32_t value) {
  return InterlockedExchangeAdd(static_cast<long*>(var), -value);
}

inline uint32_t atomic_set(void* var, uint32_t value) {
  InterlockedExchange(static_cast<long*>(var), static_cast<long>(value));
  return value;
}

inline uint32_t atomic_get(void* var) {
  return InterlockedExchangeAdd(static_cast<long*>(var), 0);
}
#else
inline uint32_t atomic_inc(void* var) {
  return __sync_add_and_fetch(static_cast<uint32_t*>(var), 1);
}

inline uint32_t atomic_dec(void* var) {
  return __sync_add_and_fetch(static_cast<uint32_t*>(var), -1);
}

inline uint32_t atomic_add(void* var, uint32_t value) {
  return __sync_fetch_and_add(static_cast<uint32_t*>(var), value);
}

inline uint32_t atomic_sub(void* var, int32_t value) {
  return __sync_fetch_and_add(static_cast<uint32_t*>(var), -value);
}

inline uint32_t atomic_set(void* var, uint32_t value) {
  __sync_lock_test_and_set(static_cast<uint32_t*>(var), value);
  return value;
}

inline uint32_t atomic_get(void* var) {
  __sync_fetch_and_add(static_cast<uint32_t*>(var), 0);
}
#endif

#endif  // __CROSS_PLATFORM_ATOMIC_HEADER_H__
