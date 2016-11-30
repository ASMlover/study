// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_TYPES_H
#define CHAOS_TYPES_H

#include <stdint.h>
#include <memory>
#include <Chaos/Copyable.h>
#include <Chaos/UnCopyable.h>

#if !defined(CHAOS_UNUSED)
# define CHAOS_UNUSED(x) ((void)x)
#endif

#if !defined(CHAOS_NANOSEC)
# define CHAOS_NANOSEC ((uint64_t)1e9)
#endif

#if !defined(CHAOS_CHECK)
# include <stdlib.h>
# include <Chaos/IO/ColorIO.h>
# define CHAOS_CHECK(condition, message) do {\
    if (!(condition)) {\
      ColorIO::fprintf(stderr,\
          Chaos::ColorIO::ColorType::COLORTYPE_RED,\
          "[%s:%d] CHECKING FAILED `%s()` - %s\n",\
          __FILE__,\
          __LINE__,\
          __func__,\
          (message));\
      fflush(stderr);\
      abort();\
    }\
} while (0)
#endif

typedef unsigned char byte_t;

namespace Chaos {

template <typename T>
inline const T& chaos_min(const T& a, const T& b) {
  return a < b ? a : b;
}

template <typename T>
inline const T& chaos_max(const T& a, const T& b) {
  return a > b ? a : b;
}

template <typename T>
inline T* get_pointer(const std::unique_ptr<T>& p) {
  return p.get();
}

template <typename T>
inline T* get_pointer(const std::shared_ptr<T>& p) {
  return p.get();
}

template <typename T> struct Identity {
  typedef T Type;
};

template <typename T>
inline T implicit_cast(typename Identity<T>::Type x) {
  return x;
}

template <typename Target, typename Source>
inline Target down_cast(Source& x) {
  return static_cast<Target>(x);
}

template <typename Target, typename Source>
inline Target down_cast(Source* x) {
  return static_cast<Target>(x);
}

template <typename Target, typename Source>
inline std::shared_ptr<Target> down_cast_ptr(const std::shared_ptr<Source>& p) {
  return std::static_pointer_cast<Target>(p);
}

}

#endif // CHAOS_TYPES_H
