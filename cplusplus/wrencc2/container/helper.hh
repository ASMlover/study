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
#include "../common.hh"

namespace wrencc {

struct TrueType {};
struct FalseType {};

template <bool _HasTrivial = false>
struct HasTrivially {
  using HasTrivialDefaultConstructor  = FalseType;
  using HasTrivialCopyConstructor     = FalseType;
  using HasTrivialMoveConstructor     = FalseType;
  using HasTrivialDestructor          = FalseType;
  using HasTrivialCopyAssignment      = FalseType;
  using HasTrivialMoveAssignment      = FalseType;
  using IsPOD                         = FalseType;
};

template <> struct HasTrivially<true> {
  using HasTrivialDefaultConstructor  = TrueType;
  using HasTrivialCopyConstructor     = TrueType;
  using HasTrivialMoveConstructor     = TrueType;
  using HasTrivialDestructor          = TrueType;
  using HasTrivialCopyAssignment      = TrueType;
  using HasTrivialMoveAssignment      = TrueType;
  using IsPOD                         = TrueType;
};

template <typename Tp> struct TypeTraits : public HasTrivially<false> {};
template <> struct TypeTraits<bool> : public HasTrivially<true> {};
template <> struct TypeTraits<char> : public HasTrivially<true> {};
template <> struct TypeTraits<i8_t> : public HasTrivially<true> {};
template <> struct TypeTraits<u8_t> : public HasTrivially<true> {};
template <> struct TypeTraits<i16_t> : public HasTrivially<true> {};
template <> struct TypeTraits<u16_t> : public HasTrivially<true> {};
template <> struct TypeTraits<i32_t> : public HasTrivially<true> {};
template <> struct TypeTraits<u32_t> : public HasTrivially<true> {};
template <> struct TypeTraits<i64_t> : public HasTrivially<true> {};
template <> struct TypeTraits<u64_t> : public HasTrivially<true> {};
template <> struct TypeTraits<float> : public HasTrivially<true> {};
template <> struct TypeTraits<double> : public HasTrivially<true> {};
template <> struct TypeTraits<long double> : public HasTrivially<true> {};
template <typename Tp> struct TypeTraits<Tp*> : public HasTrivially<true> {};

template <typename Tp> struct IsInteger { using Integral = FalseType; };
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

template <typename Tp, typename Distance> struct DefaultIter {
  using ValueType       = Tp;
  using DifferenceType  = Distance;
  using Pointer         = Tp*;
  using Reference       = Tp&;
};

template <typename Tp, typename Distance>
struct InputIterator : public DefaultIter<Tp, Distance> {
  using IteratorCategory = InputIteratorTag;
};

struct OutputIterator {
  using IteratorCategory  = OutputIteratorTag;
  using ValueType         = void;
  using DifferenceType    = void;
  using Pointer           = void;
  using Reference         = void;
};

template <typename Tp, typename Distance>
struct ForwardIterator : public DefaultIter<Tp, Distance> {
  using IteratorCategory = ForwardIteratorTag;
};

template <typename Tp, typename Distance>
struct BidirectionalIterator : public DefaultIter<Tp, Distance> {
  using IteratorCategory = BidirectionalIteratorTag;
};

template <typename Tp, typename Distance>
struct RandomAccessIterator : public DefaultIter<Tp, Distance> {
  using IteratorCategory = RandomAccessIteratorTag;
};

template <typename _Category,
          typename _Tp,
          typename _Distance = std::ptrdiff_t,
          typename _Pointer = _Tp*,
          typename _Reference = _Tp&>
struct Iterator {
  using IteratorCategory  = _Category;
  using ValueType         = _Tp;
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

template <typename Tp> struct IteratorTraits<Tp*> {
  using IteratorCategory  = RandomAccessIteratorTag;
  using ValueType         = Tp;
  using DifferenceType    = std::ptrdiff_t;
  using Pointer           = Tp*;
  using Reference         = Tp&;
};

template <typename Tp> struct IteratorTraits<const Tp*> {
  using IteratorCategory  = RandomAccessIteratorTag;
  using ValueType         = Tp;
  using DifferenceType    = std::ptrdiff_t;
  using Pointer           = const Tp*;
  using Reference         = const Tp&;
};

template <typename _Iter>
inline typename IteratorTraits<_Iter>::IteratorCategory
__Category(const _Iter&) noexcept {
  using _Category = typename IteratorTraits<_Iter>::IteratorCategory;
  return _Category();
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::DifferenceType*
__DistanceType(const _Iter&) noexcept {
  return static_cast<typename IteratorTraits<_Iter>::DifferenceType*>(nullptr);
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::ValueType*
__ValueType(const _Iter&) noexcept {
  return static_cast<typename IteratorTraits<_Iter>::ValueType*>(nullptr);
}

template <typename Tp>
inline void construct(Tp* p) noexcept {
  new ((void*)p) Tp();
}

template <typename T1, typename T2>
inline void construct(T1* p, const T2& v) noexcept {
  new ((void*)p) T1(v);
}

template <typename T1, typename T2>
inline void construct(T1* p, T2&& v) noexcept {
  new ((void*)p) T1(std::forward<T2>(v));
}

template <typename Tp, typename... Args>
inline void construct(Tp* p, Args&&... args) noexcept {
  new ((void*)p) Tp(std::forward<Args>(args)...);
}

template <typename Tp> inline void destroy(Tp* p) noexcept { p->~Tp(); }

template <typename ForwardIter>
inline void destroy(ForwardIter first, ForwardIter last, FalseType) noexcept {
  for (; first != last; ++first)
    destroy(&*first);
}

template <typename ForwardIter>
inline void destroy(ForwardIter first, ForwardIter last, TrueType) noexcept {}

template <typename ForwardIter, typename Tp>
inline void destroy(ForwardIter first, ForwardIter last, Tp*) noexcept {
  using _Trivial = typename TypeTraits<Tp>::HasTrivialDestructor;
  destroy(first, last, _Trivial());
}

template <typename ForwardIter>
inline void destroy(ForwardIter first, ForwardIter last) noexcept {
  destroy(first, last, __ValueType(first));
}

inline void destroy(bool*, bool*) noexcept {}
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

template <typename InputIter, typename OutputIter>
inline void copy(InputIter first,
    InputIter last, OutputIter result, InputIteratorTag) noexcept {
  for (; first != last; ++result, ++first)
    *result = *first;
}

template <typename RandomAccessIter, typename OutputIter>
inline void copy(RandomAccessIter first,
    RandomAccessIter last, OutputIter result, RandomAccessIteratorTag) noexcept {
  for (; first != last; ++first, ++last)
    *result = *first;
}

template <typename Tp>
inline void copy_trivial(const Tp* first, const Tp* last, Tp* result) noexcept {
  std::memmove(result, first, sizeof(Tp) * (last - first));
}

template <typename InputIter, typename OutputIter>
inline void copy(
    InputIter first, InputIter last, OutputIter result, FalseType) noexcept {
  copy(first, last, result, __Category(first));
}

template <typename InputIter, typename OutputIter>
inline void copy(
    InputIter first, InputIter last, OutputIter result, TrueType) noexcept {
  copy(first, last, result, __Category(first));
}

template <typename Tp>
inline void copy(Tp* first, Tp* last, Tp* result, TrueType) noexcept {
  copy_trivial(first, last, result);
}

template <typename Tp>
inline void copy(const Tp* first, const Tp* last, Tp* result, TrueType) noexcept {
  copy_trivial(first, last, result);
}

template <typename InputIter, typename OutputIter, typename Tp>
inline void copy(
    InputIter first, InputIter last, OutputIter result, Tp*) noexcept {
  using _Trivial = typename TypeTraits<Tp>::HasTrivialCopyAssignment;
  copy(first, last, result, _Trivial());
}

template <typename InputIter, typename OutputIter>
inline void copy(InputIter first, InputIter last, OutputIter result) noexcept {
  copy(first, last, result, __ValueType(first));
}

template <typename InputIter, typename ForwardIter>
inline void uninitialized_copy(
    InputIter first, InputIter last, ForwardIter result, TrueType) noexcept {
  copy(first, last, result);
}

template <typename InputIter, typename ForwardIter>
inline void uninitialized_copy(
    InputIter first, InputIter last, ForwardIter result, FalseType) noexcept {
  for (; first != last; ++first, ++result)
    construct(&*result, *first);
}

template <typename InputIter, typename ForwardIter, typename Tp>
inline void uninitialized_copy(
    InputIter first, InputIter last, ForwardIter result, Tp*) noexcept {
  using IsPOD = typename TypeTraits<Tp>::IsPOD;
  uninitialized_copy(first, last, result, IsPOD());
}

template <typename InputIter, typename ForwardIter>
inline void uninitialized_copy(
    InputIter first, InputIter last, ForwardIter result) noexcept {
  uninitialized_copy(first, last, result, __ValueType(result));
}

inline void uninitialized_copy(
    const char* first, const char* last, char* result) noexcept {
  std::memmove(result, first, last - first);
}

template <typename ForwardIter, typename Size, typename T>
inline void fill(ForwardIter first, Size n, const T& v) noexcept {
  for (; n > 0; --n, ++first)
    *first = v;
}

template <typename Size>
inline void fill(char* first, Size n, const char& v) noexcept {
  std::memset(first, (char)v, (sz_t)n);
}

#if defined(WRENCC_MSVC)
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
#else
template <typename Size>
inline void fill(i8_t* first, Size n, const i8_t& v) noexcept {
  std::memset(first, v, (sz_t)n);
}

template <typename Size>
inline void fill(u8_t* first, Size n, const u8_t& v) noexcept {
  std::memset(first, v, (sz_t)n);
}

template <typename Size, typename T>
inline void fill(i16_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  i16_t value = (i16_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosw\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}

template <typename Size, typename T>
inline void fill(u16_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  u16_t value = (u16_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosw\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}

template <typename Size, typename T>
inline void fill(i32_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  i32_t value = (i32_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosl\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}

template <typename Size, typename T>
inline void fill(u32_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  u32_t value = (u32_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosl\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}

template <typename Size, typename T>
inline void fill(i64_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  i64_t value = (i64_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosq\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}

template <typename Size, typename T>
inline void fill(u64_t* first, Size n, T v) noexcept {
  std::uintptr_t count = (std::uintptr_t)n;
  u64_t value = (u64_t)v;

  __asm__ __volatile__("cld\n\t"
      "rep stosq\n\t"
      : "+c" (count), "+D" (first), "=m" (first)
      : "a" (value)
      : "cc");
}
#endif

template <typename ForwardIter, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIter first, Size n, const T& v, TrueType) noexcept {
  fill(first, n, v);
}

template <typename ForwardIter, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIter first, Size n, const T& v, FalseType) noexcept {
  for (; n > 0; --n, ++first)
    construct(&*first, v);
}

template <typename ForwardIter, typename Size, typename T, typename T1>
inline void uninitialized_fill(
    ForwardIter first, Size n, const T& v, T1*) noexcept {
  using IsPOD = typename TypeTraits<T1>::IsPOD;
  uninitialized_fill(first, n, v, IsPOD());
}

template <typename ForwardIter, typename Size, typename T>
inline void uninitialized_fill(
    ForwardIter first, Size n, const T& v) noexcept {
  uninitialized_fill(first, n, v, __ValueType(first));
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
