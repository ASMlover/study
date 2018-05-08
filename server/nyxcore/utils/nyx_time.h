// Copyright (c) 2018 ASMlover. All rights reserved.
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
#pragma once

#include "../nyx_base.h"
#include <time.h>
#if !defined(_NYXCORE_WINDOWS)
# include <cpuid.h>
# include <sys/time.h>
# include <sys/types.h>
#else
# include <intrin.h>
#endif
#include <chrono>

namespace nyx { namespace time {

struct tm2 : public ::tm {
  std::int32_t tm_usec;
};

constexpr std::uint64_t kStampsPerSecond = 1000000ULL;

inline std::uint32_t chrono_seconds(void) {
  return static_cast<std::uint32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

inline std::uint64_t chrono_milliseconds(void) {
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

inline std::uint64_t chrono_microseconds(void) {
  return static_cast<std::uint64_t>(
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count());
}

inline void localtime(tm2* result) {
  auto now_miscosec = chrono_microseconds();
  auto now_seconds = now_miscosec * 0.000001;
  auto t = static_cast<time_t>(now_seconds);
  result->tm_usec = static_cast<std::uint32_t>((now_seconds - t) * 1000000);
}

inline std::uint64_t timestamp(void) {
  return chrono_microseconds();
}

inline std::uint64_t milliseconds(void) {
  return chrono_milliseconds();
}

inline std::uint64_t microseconds(void) {
  return chrono_microseconds();
}

inline constexpr std::uint64_t stamps_per_second(void) {
  return kStampsPerSecond;
}

inline std::uint64_t begin_stamp(void) {
  static std::uint64_t begin = timestamp();
  return begin;
}

inline std::uint64_t begin_microseconds(void) {
  static std::uint64_t begin = microseconds();
  return begin;
}

#if !defined(_NYXCORE_WINDOWS)
inline bool invariant_tsc_support(void) {
  int _eax, _ebx, _ecx, _edx;
  __cpuid(0x80000007, _eax, _ebx, _ecx, _edx);
  return (_edx & (1 << 8)) != 0;
}

inline std::uint64_t timestamp_rdtscp(void) {
  std::uint32_t lo, hi;
  __asm__ __volatile__(
      "rdtscp" :
      "=a"(lo), "=d"(hi) : :
      "%rcx");
  return static_cast<std::uint64_t>(lo) | (static_cast<std::uint64_t>(hi) << 32);
}
#else
inline bool invariant_tsc_support(void) {
  int info[4]{};
  __cpuid(info, 0x80000007);
  return (info[3] & (1 << 8)) != 0;
}

inline std::uint64_t timestamp_rdtscp(void) {
  unsigned int aux{};
  return __rdtscp(&aux);
}
#endif

}}
