// Copyright (c) 2024 ASMlover. All rights reserved.
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

#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include "macros.hh"

namespace sdb {

class _Safe {};
class _Unsafe {};

using nil_t             = std::nullptr_t;
using byte_t            = std::uint8_t;
using i8_t              = std::int8_t;
using u8_t              = std::uint8_t;
using i16_t             = std::int16_t;
using u16_t             = std::uint16_t;
using i32_t             = std::int32_t;
using u32_t             = std::uint32_t;
using i64_t             = std::int64_t;
using u64_t             = std::uint64_t;
using sz_t              = std::size_t;
#if defined(SDB_POSIX)
  using ssz_t           = std::sszie_t;
#else
  using ssz_t           = std::int64_t;
#endif
using str_t             = std::string;
using cstr_t            = const char*;
using strv_t            = std::string_view;
using ss_t              = std::stringstream;
using safe_t            = _Safe;
using unsafe_t          = _Unsafe;

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

template <typename Object> class Singleton : private UnCopyable {
public:
  static Object& get_instance() noexcept {
    static Object ins;
    return ins;
  }
};

template <typename... Mixins> class MixinClass : public Mixins... {
public:
  constexpr MixinClass() : Mixins()... {}
};

template <typename T, typename X> inline T as_type(X x) noexcept {
  return static_cast<T>(x);
}

template <typename T, typename X> inline T* as_down(X* x) noexcept {
  return dynamic_cast<T*>(x);
}

template <typename T> inline T* as_ptr(void* p) noexcept {
  return reinterpret_cast<T*>(p);
}

template <typename T, typename PTR> inline T* get_rawptr(const PTR& p) noexcept {
  return p.get();
}

}
