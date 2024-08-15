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

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || \
  defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
# define EV_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define EV_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__MACH__)
# define EV_DARWIN
#else
# error "Unknown Platform !!!"
#endif

#if !defined(EV_WINDOWS)
# define EV_POSIX
#endif

#if defined(i386) || defined(__i386__) || defined(__i486__) || \
  defined(__i586__) || defined(__i686__) || defined(__i386) || \
  defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL__) || \
  defined(__I86__) || defined(__INTEL__)
# define EV_ARCH32
#elif defined(__x86_64) || defined(__x86_64__) || \
  defined(__amd64__) || defined(__amd64) || defined(_M_X64)
# define EV_ARCH64
#else
# error "Unknown Architecture !!!"
#endif

#if !defined(EV_UNUSED)
# define EV_UNUSED(x)             ((void)x)
#endif

#if !defined(EV_NANOSEC)
# define EV_NANOSEC               ((ev::u64_t)1e9)
#endif

#if !defined(EV_COUNTOF)
# define EV_COUNTOF(array)        sizeof(ev::__countof_impl(array).elements)
#endif

#include <string>
#include <string_view>
#include <sstream>

namespace ev {

class _Safe {};
class _Unsafe {};

using nil_t                       = std::nullptr_t;
using byte_t                      = std::uint8_t;
using i8_t                        = std::int8_t;
using u8_t                        = std::uint8_t;
using i16_t                       = std::int16_t;
using u16_t                       = std::uint16_t;
using i32_t                       = std::int32_t;
using u32_t                       = std::uint32_t;
using i64_t                       = std::int64_t;
using u64_t                       = std::uint64_t;
using sz_t                        = std::size_t;
#if defined(EV_POSIX)
  using ssz_t                     = ssize_t;
#else
# if defined(EV_ARCH64)
    using ssz_t                   = std::int64_t;
# else
    using ssz_t                   = std::int32_t;
# endif
#endif
using str_t                       = std::string;
using strv_t                      = std::string_view;
using cstr_t                      = const char*;
using ss_t                        = std::stringstream;
using safe_t                      = _Safe;
using unsafe_t                    = _Unsafe;

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

template <std::ptrdiff_t N> struct Sizer { char elements[N]; };
template <typename T, std::ptrdiff_t N> Sizer<N> __countof_impl(T (&array)[N]);

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
