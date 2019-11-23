// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace wrencc {

using nil_t   = std::nullptr_t;
using byte_t  = std::uint8_t;
using i8_t    = std::int8_t;
using u8_t    = std::uint8_t;
using i16_t   = std::int16_t;
using u16_t   = std::uint16_t;
using i32_t   = std::int32_t;
using u32_t   = std::uint32_t;
using i64_t   = std::int64_t;
using u64_t   = std::uint64_t;
using ssz_t   = std::int32_t;
using sz_t    = std::size_t;
using str_t   = std::string;
using strv_t  = std::string_view;

class Copyable {
protected:
  Copyable() noexcept = default;
  ~Copyable() noexcept = default;
  Copyable(const Copyable&) noexcept = default;
  Copyable(Copyable&&) noexcept = default;
  Copyable& operator=(const Copyable&) noexcept = default;
  Copyable& operator=(Copyable&&) noexcept = default;
};

class UnCopyable {
  UnCopyable(const UnCopyable&) noexcept = delete;
  UnCopyable(UnCopyable&&) noexcept = delete;
  UnCopyable& operator=(const UnCopyable&) noexcept = delete;
  UnCopyable& operator=(UnCopyable&&) noexcept = delete;
protected:
  UnCopyable() noexcept = default;
  ~UnCopyable() noexcept = default;
};

namespace Xt {
  template <typename T, typename U>
  inline T as_type(U x) noexcept { return static_cast<T>(x); }

  template <typename T, typename U>
  inline T* cast(U* x) noexcept { return static_cast<T*>(x); }

  template <typename T, typename U>
  inline T* down(U* x) noexcept { return dynamic_cast<T*>(x); }

  template <typename T>
  inline double do_decimal(T x) noexcept { return as_type<double>(x); }

  inline str_t to_string(double d) noexcept {
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
    return ss.str();
  }

  union NumericBits {
    u64_t u64;
    u32_t u32[2];
    double num;
  };

  inline u32_t hash_u64(u64_t hash) noexcept {
    // from V8's computeLongHash() which in turn cites:
    // Thomas Wang, Integer Hash Functions
    // http://www.concentric.net/~Ttwang/tech/inthash.htm
    hash = ~hash + (hash << 18);
    hash = hash ^ (hash >> 31);
    hash = hash * 21;
    hash = hash ^ (hash >> 11);
    hash = hash + (hash << 6);
    hash = hash ^ (hash >> 22);
    return as_type<u32_t>(hash & 0x3fffffff);
  }

  inline u32_t hash_numeric(double d) noexcept {
    NumericBits bits;
    bits.num = d;
    return hash_u64(bits.u64);
  }
}

#ifndef NDEBUG
# define ASSERT(cond, msg) do {\
  if (!(cond)) {\
    std::cerr << "[" << __FILE__ << ": " << __LINE__ << "] "\
              << "Assert failed in " << __func__ << "(): "\
              << msg << std::endl;\
    std::abort();\
  }\
} while (false)

# define UNREACHABLE() do {\
  std::cerr << "[" << __FILE__ << ": " << __LINE__ << "]"\
            << "This code should not be reached in " << __func__ << "()"\
            << std::endl;\
  std::abort();\
} while (false)
#else
# define ASSERT(cond, msg)  ((void)0)
# if defined(_MSC_VER)
#   define UNREACHABLE()    __assume(0)
# elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#   define UNREACHABLE()    __builtin_unreachable()
# else
#   define UNREACHABLE()
# endif
#endif

}
