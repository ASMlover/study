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

#include "common.hh"

namespace wrencc {

struct TrueType {};
struct FalseType {};

template <typename T> struct TypeTraits {
  using HasTrivialDefaultConstructor  = FalseType;
  using HasTrivialCopyConstructor     = FalseType;
  using HasTrivialAssignOperator      = FalseType;
  using HasTrivialDestructor          = FalseType;
  using IsPOD                         = FalseType;
};

template <> struct TypeTraits<bool> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<i8_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<u8_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<i16_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<u16_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<i32_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<u32_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<i64_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<u64_t> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<float> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<double> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<long double> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <typename T> struct TypeTraits<T*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <typename T> struct TypeTraits<const T*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<char*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<const char*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<i8_t*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <> struct TypeTraits<u8_t*> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialAssignOperator      = TrueType;
  using HasTrivialDestructor          = TrueType;
  using IsPOD                         = TrueType;
};

template <typename T> struct IsInteger { using Integral = FalseType; };
template <> struct IsInteger<bool> { using Integral = TrueType; };
template <> struct IsInteger<char> { using Integral = TrueType; };
template <> struct IsInteger<i8_t> { using Integral = TrueType; };
template <> struct IsInteger<u8_t> { using Integral = TrueType; };
template <> struct IsInteger<i16_t> { using Integral = TrueType; };
template <> struct IsInteger<u16_t> { using Integral = TrueType; };
template <> struct IsInteger<i32_t> { using Integral = TrueType; };
template <> struct IsInteger<u32_t> { using Integral = TrueType; };
template <> struct IsInteger<i64_t> { using Integral = TrueType; };
template <> struct IsInteger<u64_t> { using Integral = TrueType; };

template <typename T>
inline void construct(T* p) noexcept {
  new ((void*)p) T();
}

template <typename T1, typename T2>
inline void construct(T1* p, const T2& v) noexcept {
  new ((void*)p) T1(v);
}

template <typename T1, typename T2>
inline void construct(T1* p, T2&& v) noexcept {
  new ((void*)p) T1(std::forward<T2>(v));
}

template <typename T1, typename... Args>
inline void construct(T1* p, Args&&... args) noexcept {
  new ((void*)p) T1(std::forward<Args>(args)...);
}

template <typename T>
inline void destroy(T* p) noexcept {
  p->~T();
}

template <typename ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) noexcept {
  for (; first != last; ++first)
    destroy(&*first);
}

inline void destroy(char*, char*) noexcept {}
inline void destroy(i8_t*, i8_t*) noexcept {}
inline void destroy(u8_t*, u8_t*) noexcept {}
inline void destroy(i16_t*, i16_t*) noexcept {}
inline void destroy(u16_t*, u16_t*) noexcept {}
inline void destroy(i32_t*, i32_t*) noexcept {}
inline void destroy(u32_t*, u32_t*) noexcept {}
inline void destroy(i64_t*, i64_t*) noexcept {}
inline void destroy(u64_t*, u64_t*) noexcept {}
inline void destroy(float*, float*) noexcept {}
inline void destroy(double*, double*) noexcept {}

template <typename InputIterator, typename ForwardIterator>
inline void copy(
    InputIterator first, InputIterator last, ForwardIterator result) noexcept {
  for (; first != last; ++first, ++result)
    *result = *first;
}

template <typename InputIterator, typename ForwardIterator>
inline void uninitialized_copy(
    InputIterator first, InputIterator last, ForwardIterator result) {
  ForwardIterator pos = result;
  try {
    for (; first != last; ++first, ++pos)
      construct(&*pos, *first);
  }
  catch (...) {
    destroy(result, pos);
    throw;
  }
}

inline void uninitialized_copy(
    const char* first, const char* last, char* result) noexcept {
  std::memmove(result, first, last - first);
}

template <typename ForwardIterator, typename Size, typename T>
inline void fill(ForwardIterator first, Size n, const T& v) noexcept {
  for (; n > 0; --n, ++first)
    *first = v;
}

template <typename Size, typename T>
inline void fill(i64_t* first, Size n, T v) noexcept {
  __stosq((u64_t*)first, (u64_t)v, (sz_t)n);
}

template <typename ForwardIterator, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIterator first, Size n, const T& v) noexcept {
  for (; n > 0; --n, ++first)
    construct(&*first, v);
}

template <typename T>
class SimpleAlloc final : private UnCopyable {
public:
  static T* allocate() noexcept {
    return (T*)std::malloc(sizeof(T));
  }

  static T* allocate(sz_t n) noexcept {
    return 0 == n ? 0 : (T*)std::malloc(n * sizeof(T));
  }

  static void deallocate(T* p) noexcept {
    std::free(p);
  }

  static void deallocate(T* p, sz_t /*n*/) noexcept {
    std::free(p);
  }

  static T* reallocate(T* p, sz_t new_count) noexcept {
    return (T*)std::realloc(p, new_count * sizeof(T));
  }
};

}
