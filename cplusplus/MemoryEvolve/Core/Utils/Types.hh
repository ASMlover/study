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

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#if !defined(_MEVO_UNUSED)
# define _MEVO_UNUSED(x) ((void)x)
#endif

#if !defined(_MEVO_CHECK)
# include <cstdlib>
# define _MEVO_CHECK(condition, message) do {\
    if (!(condition)) {\
        std::fprintf(stderr,\
            "[%s:%d] CHECKING FAILED `%s()` - `%s` - %s\n",\
            __FILE__,\
            __LINE__,\
            __func__,\
            #condition,\
            (message));\
        std::fflush(stderr);\
        std::abort();\
    }\
  } while (0)
#endif

#if !defined(_MEVO_COUNTOF)
# define _MEVO_COUNTOF(array) sizeof(_mevo::__countof_impl(array).elements)
#endif

namespace _mevo {

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
using sz_t    = std::size_t;
using str_t   = std::string;
using strv_t  = std::string_view;
using ss_t    = std::stringstream;

template <std::ptrdiff_t N> struct Sizer { char elements[N]; };
template <typename T, std::ptrdiff_t N> Sizer<N> __countof_impl(T (&array)[N]);

template <typename T> struct Identity { typedef T type; };
template <typename T>
inline T implicit_cast(typename Identity<T>::type x) noexcept {
  return x;
}

template <typename T, typename S> inline T as_type(S x) noexcept {
  return static_cast<T>(x);
}

template <typename T, typename S> inline T* down(S* x) noexcept {
  return dynamic_cast<T*>(x);
}

template <typename T>
inline T* get_raw_pointer(const std::unique_ptr<T>& p) noexcept {
  return p.get();
}

template <typename T>
inline T* get_raw_pointer(const std::shared_ptr<T>& p) noexcept {
  return p.get();
}

}
