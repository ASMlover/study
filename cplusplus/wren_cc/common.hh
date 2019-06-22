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
  Copyable(void) noexcept = default;
  ~Copyable(void) noexcept = default;
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
  UnCopyable(void) noexcept = default;
  ~UnCopyable(void) noexcept = default;
};

namespace Xt {
  template <typename T, typename U>
  inline T as_type(U x) { return static_cast<T>(x); }

  template <typename T, typename U>
  inline T* cast(U* x) { return static_cast<T*>(x); }

  template <typename T, typename U>
  inline T* down(U* x) { return dynamic_cast<T*>(x); }

  inline str_t to_string(double d) {
    std::stringstream ss;
    ss << d;
    return ss.str();
  }
}

}

// define this to stress test the GC, it will perform a collection before
// every allocation
#define GC_STRESS

#define NAN_TAGGING

#ifndef NDEBUG
# define ASSERT(cond, msg) if (!(cond)) {\
  std::cerr << "ASSERT FAIL `" << __FILE__ << "`: "\
    << __LINE__ << " - " << msg << std::endl;\
  std::abort();\
}
#else
# define ASSERT(cond, msg) ((void)0)
#endif
