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

#include <intrin.h>
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

struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};

template <typename T, typename Distance> struct InputIterator {
  using IteratorCategory  = InputIteratorTag;
  using ValueType         = T;
  using DifferenceType    = Distance;
  using Pointer           = T*;
  using Reference         = T&;
};

struct OutputIterator {
  using IteratorCategory  = OutputIteratorTag;
  using ValueType         = void;
  using DifferenceType    = void;
  using Pointer           = void;
  using Reference         = void;
};

template <typename T, typename Distance> struct ForwardIterator {
  using IteratorCategory  = ForwardIteratorTag;
  using ValueType         = T;
  using DifferenceType    = Distance;
  using Pointer           = T*;
  using Reference         = T&;
};

template <typename T, typename Distance> struct BidirectionalIterator {
  using IteratorCategory  = BidirectionalIteratorTag;
  using ValueType         = T;
  using DifferenceType    = Distance;
  using Pointer           = T*;
  using Reference         = T&;
};

template <typename T, typename Distance> struct RandomAccessIterator {
  using IteratorCategory  = RandomAccessIteratorTag;
  using ValueType         = T;
  using DifferenceType    = Distance;
  using Pointer           = T*;
  using Reference         = T&;
};

template <typename _Category,
          typename _T,
          typename _Distance = std::ptrdiff_t,
          typename _Pointer = _T*,
          typename _Reference = _T&>
struct Iterator {
  using IteratorCategory  = _Category;
  using ValueType         = _T;
  using DifferenceType    = _Distance;
  using Pointer           = _Pointer;
  using Reference         = _Reference;
};

template <typename _Iterator> struct IteratorTraits {
  using IteratorCategory  = typename _Iterator::IteratorCategory;
  using ValueType         = typename _Iterator::ValueType;
  using DifferenceType    = typename _Iterator::DifferenceType;
  using Pointer           = typename _Iterator::Pointer;
  using Reference         = typename _Iterator::Reference;
};

template <typename T> struct IteratorTraits<T*> {
  using IteratorCategory  = RandomAccessIteratorTag;
  using ValueType         = T;
  using DifferenceType    = std::ptrdiff_t;
  using Pointer           = T*;
  using Reference         = T&;
};

template <typename T> struct IteratorTraits<const T*> {
  using IteratorCategory  = RandomAccessIteratorTag;
  using ValueType         = T;
  using DifferenceType    = std::ptrdiff_t;
  using Pointer           = const T*;
  using Reference         = const T&;
};

template <typename _Iterator>
inline typename IteratorTraits<_Iterator>::IteratorCategory
_IteratorCategory(const _Iterator&) noexcept {
  using _Category = typename IteratorTraits<_Iterator>::IteratorCategory;
  return _Category();
}

template <typename _Iterator>
inline typename IteratorTraits<_Iterator>::DifferenceType*
_DistanceType(const _Iterator&) noexcept {
  return Xt::as_type<typename IteratorTraits<_Iterator>::DifferenceType*>(nullptr);
}

template <typename _Iterator>
inline typename IteratorTraits<_Iterator>::ValueType*
_ValueType(const _Iterator&) noexcept {
  return Xt::as_type<typename IteratorTraits<_Iterator>::ValueType*>(nullptr);
}

template <typename T, typename Distance>
inline InputIteratorTag
_IteratorCategory(const InputIterator<T, Distance>&) noexcept {
  return InputIteratorTag();
}

inline OutputIteratorTag _IteratorCategory(const OutputIteratorTag&) noexcept {
  return OutputIteratorTag();
}

template <typename T, typename Distance>
inline ForwardIteratorTag
_IteratorCategory(const ForwardIterator<T, Distance>&) noexcept {
  return ForwardIteratorTag();
}

template <typename T, typename Distance>
inline BidirectionalIteratorTag
_IteratorCategory(const BidirectionalIterator<T, Distance>&) noexcept {
  return BidirectionalIteratorTag();
}

template <typename T, typename Distance>
inline RandomAccessIteratorTag
_IteratorCategory(const RandomAccessIterator<T, Distance>&) noexcept {
  return RandomAccessIteratorTag();
}

template <typename T>
inline RandomAccessIteratorTag _IteratorCategory(const T*) noexcept {
  return RandomAccessIteratorTag();
}

template <typename T, typename Distance>
inline Distance* _DistanceType(const InputIterator<T, Distance>&) noexcept {
  return (Distance*)(nullptr);
}

template <typename T, typename Distance>
inline Distance* _DistanceType(const ForwardIterator<T, Distance>&) noexcept {
  return (Distance*)(nullptr);
}

template <typename T, typename Distance>
inline Distance*
_DistanceType(const BidirectionalIterator<T, Distance>&) noexcept {
  return (Distance*)(nullptr);
}

template <typename T, typename Distance>
inline Distance*
_DistanceType(const RandomAccessIterator<T, Distance>&) noexcept {
  return (Distance*)(nullptr);
}

template <typename T>
inline std::ptrdiff_t* _DistanceType(const T*) noexcept {
  return (std::ptrdiff_t*)(nullptr);
}

template <typename T, typename Distance>
inline T* _ValueType(const InputIterator<T, Distance>&) noexcept {
  return (T*)(nullptr);
}

template <typename T, typename Distance>
inline T* _ValueType(const ForwardIterator<T, Distance>&) noexcept {
  return (T*)(nullptr);
}

template <typename T, typename Distance>
inline T* _ValueType(const BidirectionalIterator<T, Distance>&) noexcept {
  return (T*)(nullptr);
}

template <typename T, typename Distance>
inline T* _ValueType(const RandomAccessIterator<T, Distance>&) noexcept {
  return (T*)(nullptr);
}

template <typename T>
inline T* _ValueType(const T*) noexcept { return (T*)(nullptr;) }

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
inline void destroy_aux(
    ForwardIterator first, ForwardIterator last, FalseType) noexcept {
  for (; first != last; ++first)
    destroy(&*first);
}

template <typename ForwardIterator>
inline void destroy_aux(
    ForwardIterator first, ForwardIterator last, TrueType) noexcept {
}

template <typename ForwardIterator, typename T>
inline void destroy(ForwardIterator first, ForwardIterator last, T*) noexcept {
  using _TrivialDestructor = typename TypeTraits<T>::HasTrivialDestructor;
  destroy_aux(first, last, _TrivialDestructor());
}

template <typename ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) noexcept {
  destroy(first, last, _ValueType(first));
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

template <typename InputIterator, typename OutputIterator>
inline void copy(InputIterator first,
    InputIterator last, OutputIterator result, InputIteratorTag) noexcept {
  for (; first != last; ++result, ++first)
    *result = *first;
}

template <typename RandomAccessIterator, typename OutputIterator>
inline void
copy(RandomAccessIterator first, RandomAccessIterator last,
    OutputIterator result, RandomAccessIteratorTag) noexcept {
  for (; first != last; ++first, ++last)
    *result = *first;
}

template <typename T>
inline void copy_trivial(const T* first, const T* last, T* result) noexcept {
  std::memmove(result, first, sizeof(T) * (last - first));
}

template <typename InputIterator, typename OutputIterator>
inline void copy(InputIterator first,
    InputIterator last, OutputIterator result, FalseType) noexcept {
  copy(first, last, result, _IteratorCategory(first));
}

template <typename InputIterator, typename OutputIterator>
inline void copy(InputIterator first,
    InputIterator last, OutputIterator result, TrueType) noexcept {
  copy(first, last, result, _IteratorCategory(first));
}

template <typename T>
inline void copy(T* first, T* last, T* result, TrueType) noexcept {
  copy_trivial(first, last, result);
}

template <typename T>
inline void copy(const T* first, const T* last, T* result, TrueType) noexcept {
  copy_trivial(first, last, result);
}

template <typename InputIterator, typename OutputIterator, typename T>
inline void copy(InputIterator first,
    InputIterator last, OutputIterator result, T*) noexcept {
  using _Trivial = typename TypeTraits<T>::HasTrivialAssignOperator;
  copy(first, last, result, _Trivial());
}

template <typename InputIterator, typename OutputIterator>
inline void
copy(InputIterator first, InputIterator last, OutputIterator result) noexcept {
  copy(first, last, result, _ValueType(first));
}

template <typename InputIterator, typename ForwardIterator>
inline void uninitialized_copy(InputIterator first,
    InputIterator last, ForwardIterator result, TrueType) noexcept {
  copy(first, last, result);
}

template <typename InputIterator, typename ForwardIterator>
inline void uninitialized_copy(InputIterator first,
    InputIterator last, ForwardIterator result, FalseType) noexcept {
  for (; first != last; ++first, ++result)
    construct(&*result, *first);
}

template <typename InputIterator, typename ForwardIterator, typename T>
inline void uninitialized_copy(InputIterator first,
    InputIterator last, ForwardIterator result, T*) noexcept {
  using IsPOD = typename TypeTraits<T>::IsPOD;
  uninitialized_copy(first, last, result, IsPOD());
}

template <typename InputIterator, typename ForwardIterator>
inline void uninitialized_copy(
    InputIterator first, InputIterator last, ForwardIterator result) noexcept {
  uninitialized_copy(first, last, result, _ValueType(result));
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

template <typename Size>
inline void fill(char* first, Size n, const char* v) noexcept {
  std::memset(first, (char)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(i8_t* first, Size n, T v) noexcept {
  __stosb((u8_t*)first, (u8_t)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(u8_t* first, Size n, T v) noexcept {
  __stosb(first, (u8_t)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(i16_t* first, Size n, T v) noexcept {
  __stosw((i16_t*)first, (u16_t)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(u16_t* first, Size n, T v) noexcept {
  __stosw(first, (u16_t)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(i32_t* first, Size n, T v) noexcept {
  __stosd((unsigned long*)first, (unsigned long)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(u32_t* first, Size n, T v) noexcept {
  __stosd((unsigned long*)first, (unsigned long)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(i64_t* first, Size n, T v) noexcept {
  __stosq((u64_t*)first, (u64_t)v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(u64_t* first, Size n, T v) noexcept {
  __stosq(first, (u64_t)v, (sz_t)n);
}

template <typename ForwardIterator, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIterator first, Size n, const T& v, TrueType) noexcept {
  fill(first, n, v);
}

template <typename ForwardIterator, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIterator first, Size n, const T& v, FalseType) noexcept {
  for (; n > 0; --n, ++first)
    construct(&*first, v);
}

template <typename ForwardIterator, typename Size, typename T, typename T1>
inline void uninitialized_fill(
    ForwardIterator first, Size n, const T& v, T1*) noexcept {
  using IsPOD = typename TypeTraits<T1>::IsPOD;
  uninitialized_fill(first, n, v, IsPOD());
}

template <typename ForwardIterator, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIterator first, Size n, const T& v) noexcept {
  uninitialized_fill(first, n, v, _ValueType(first));
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
