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

#include <iomanip>
#include <limits>
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

static constexpr sz_t kALIGNMENT  = 8;
static constexpr safe_t kSafePlaceholder{};

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

inline sz_t as_align(sz_t bytes, sz_t align = kALIGNMENT) noexcept {
  return (bytes + align - 1) & ~(align - 1);
}

inline str_t as_string(double d) noexcept {
  ss_t ss;
  ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
  return ss.str();
}

template <typename T, typename... Args> inline str_t as_string(T&& x, Args&&... args) noexcept {
  ss_t ss;
  ss << std::forward<T>(x);
  ((ss << std::forward<Args>(args)), ...);

  return ss.str();
}

template <typename... Args> inline str_t from_fmt(strv_t fmt, const Args&... args) noexcept {
  int sz = std::snprintf(nullptr, 0, fmt.data(), args...);
  std::unique_ptr<char []> buf{new char[sz + 1]};
  std::snprintf(buf.get(), sz, fmt.data(), args...);
  return str_t(buf.get(), buf.get() + sz);
}

template <typename StringVector, typename StringType, typename DelimType>
inline void string_split(const StringType& str, const DelimType& delims, u32_t max_splits, StringVector& ret) noexcept {
  if (str.empty())
    return;

  u32_t num_splits = 0;
  sz_t start = 0;
  sz_t pos;
  do {
    pos = str.find_first_of(delims, start);

    if (pos == start) {
      ret.push_back(StringType());
      start = pos + 1;
    }
    else if (StringType::npos == pos || (max_splits && num_splits + 1 >= max_splits)) {
      ret.emplace_back(StringType());
      *(ret.rbegin()) = StringType(str.data() + start, str.size() - start);
      break;
    }
    else {
      ret.push_back(StringType());
      *(ret.rbegin()) = StringType(str.data() + start, pos - start);
      start = pos + 1;
    }
    ++num_splits;
  } while (StringType::npos != pos);
}

}
