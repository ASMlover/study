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
#ifndef TPP_TYPES_H_
#define TPP_TYPES_H_

#include <stdint.h>

#if !defined(TPP_UNUSED)
# define TPP_UNUSED(x) ((void)x)
#endif

typedef unsigned char byte_t;

#if !defined(__tpp_throw)
# include <system_error>
inline void __tpp_throw(int err, const char* what) {
  throw std::system_error(std::error_code(err, std::system_category()), what);
}
#endif

namespace tpp {

class UnCopyable {
  UnCopyable(const UnCopyable&) = delete;
  UnCopyable& operator=(const UnCopyable&) = delete;
protected:
  UnCopyable(void) = default;
  ~UnCopyable(void) = default;
};


template <typename T>
inline const T& tpp_min(const T& a, const T& b) {
  return a < b ? a : b;
}

template <typename T>
inline const T& tpp_max(const T& a, const T& b) {
  return a > b ? a : b;
}

template <typename T> struct Identity {
  typedef T type;
};

template <typename T>
inline T implicit_cast(typename Identity<T>::type x) {
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

}

#endif // TPP_TYPES_H_
