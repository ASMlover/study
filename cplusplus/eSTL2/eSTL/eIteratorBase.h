// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __ESTL_ITERATOR_BASE_HEADER_H__
#define __ESTL_ITERATOR_BASE_HEADER_H__

namespace estl {

struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};

template <typename _Tp, typename _Distance> struct InputIterator {
  typedef InputIteratorTag  IteratorCategory;
  typedef _T                ValueType;
  typedef _Distance         DifferenceType;
  typedef _TP*              Pointer;
  typedef _TP&              Reference; 
};

struct OutputIterator {
  typedef OutputIteratorTag IteratorCategory;
  typedef void              ValueType;
  typedef void              DifferenceType;
  typedef void              Pointer;
  typedef void              Reference;
};

template <typename _Tp, typename _Distance> struct ForwardIterator {
  typedef ForwardIteratorTag  IteratorCategory;
  typedef _Tp                 ValueType;
  typedef _Distance           DifferenceType;
  typedef _Tp*                Pointer;
  typedef _Tp&                Reference;
};

template <typename _Tp, typename _Distance> struct BidirectionalIterator {
  typedef BidirectionalIteratorTag  IteratorCategory;
  typedef _Tp                       ValueType;
  typedef _Distance                 DifferenceType;
  typedef _Tp*                      Pointer;
  typedef _Tp&                      Reference;
};

template <typename _Tp, typename _Distance> struct RandomAccessIterator {
  typedef RandomAccessIteratorTag IteratorCategory;
  typedef _Tp                     ValueType;
  typedef _Distance               DifferenceType;
  typedef _Tp*                    Pointer;
  typedef _Tp&                    Reference;
};

template <typename _Category, 
          typename _Tp, 
          typename _Distance = ptrdiff_t, 
          typename _Pointer = _Tp*, 
          typename _Reference = _Tp&>
struct Iterator {
  typedef _Category   IteratorCategory;
  typedef _Tp         ValueType;
  typedef _Distance   DifferenceType;
  typedef _Pointer    Pointer;
  typedef _Reference  Reference;
};

#ifdef ESTL_PARTIAL_SPECIALZATION

template <typename _Iterator> struct IteratorTraits {
  typedef typename _Iterator::IteratorCategory  IteratorCategory;
  typedef typename _Iterator::ValueType         ValueType;
  typedef typename _Iterator::DifferenceType    DifferenceType;
  typedef typename _Iterator::Pointer           Pointer;
  typedef typename _Iterator::Reference         Reference;
};

template <typename _Tp> struct IteratorTraits<_Tp*> {
  typedef RandomAccessIteratorTag IteratorCategory;
  typedef _Tp                     ValueType;
  typedef ptrdiff_t               DifferenceType;
  typedef _Tp*                    Pointer;
  typedef _Tp&                    Reference;
};

template <typename _Tp> struct IteratorTraits<const _Tp*> {
  typedef RandomAccessIteratorTag IteratorCategory;
  typedef _Tp                     ValueType;
  typedef ptrdiff_t               DifferenceType;
  typedef const _Tp*              Pointer;
  typedef const _Tp&              Reference;
};

template <typename _Iter>
inline typename IteratorTraits<_Iter>::IteratorCategory
_IteratorCategory(const _Iter&) {
  typedef typename IteratorTraits<_Iter>::IteratorCategory _Category;
  return _Category();
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::DifferenceType*
_DistanceType(const _Iter&) {
  return static_cast<typename IteratorTraits<_Iter>::DifferenceType*>(0);
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::ValueType*
_ValueType(const _Iter&) {
  return static_cast<typename IteratorTraits<_Iter>::ValueType*>(0);
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::IteratorCategory
IteratorCategory(const _Iter& i) {
  return _IteratorCategory(i);
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::DifferenceType*
DistanceType(const _Iter& i) {
  return _DistanceType(i);
}

template <typename _Iter>
inline typename IteratorTraits<_Iter>::ValueType*
ValueType(const _Iter& i) {
  return _ValueType(i);
}

# define ITERATOR_CATEGORY(i) _IteratorCategory(i)
# define DISTANCE_TYPE(i)     _DistanceType(i)
# define VALUE_TYPE(i)        _ValueType(i)

#else  // ESTL_PARTIAL_SPECIALZATION

template <typename _Tp, typename _Distance>
inline InputIteratorTag 
IteratorCategory(const InputIterator<_Tp, _Distance>&) {
  return InputIteratorTag();
}

inline OutputIteratorTag IteratorCategory(const OutputIterator&) {
  return OutputIteratorTag();
}

template <typename _Tp, typename _Distance>
inline ForwardIteratorTag 
IteratorCategory(const ForwardIterator<_Tp, _Distance>&) {
  return ForwardIteratorTag();
}

template <typename _Tp, typename _Distance>
inline BidirectionalIteratorTag
IteratorCategory(const BidirectionalIterator<_Tp, _Distance>&) {
  return BidirectionalIteratorTag();
}

template <typename _Tp, typename _Distance>
inline RandomAccessIteratorTag
IteratorCategory(const RandomAccessIterator<_Tp, _Distance>&) {
  return RandomAccessIteratorTag();
}

template <typename _Tp>
inline RandomAccessIteratorTag IteratorCategory(const _Tp*) {
  return RandomAccessIteratorTag();
}

template <typename _Tp, typename _Distance>
inline _Tp* ValueType(const InputIterator<_Tp, _Distance>&) {
  return (_Tp*)(0);
}

template <typename _Tp, typename _Distance>
inline _Tp* ValueType(const ForwardIterator<_Tp, _Distance>&) {
  return (_Tp*)(0);
}

template <typename _Tp, typename _Distance>
inline _Tp* ValueType(const BidirectionalIterator<_Tp, _Distance>&) {
  return (_Tp*)(0);
}

template <typename _Tp, typename _Distance>
inline _Tp* ValueType(const RandomAccessIterator<_Tp, _Distance>&) {
  return (_Tp*)(0);
}

template <typename _Tp>
inline _Tp* ValueType(const _Tp*) {
  return (_Tp*)(0);
}

template <typename _Tp, typename _Distance>
inline _Distance* DistanceType(const InputIterator<_Tp, _Distance>&) {
  return (_Distance*)(0);
}

template <typename _Tp, typename _Distance>
inline _Distance* DistanceType(const ForwardIterator<_Tp, _Distance>&) {
  return (_Distance*)(0);
}

template <typename _Tp, typename _Distance>
inline _Distance* 
DistanceType(const BidirectionalIterator<_Tp, _Distance>&) {
  return (_Distance*)(0);
}

template <typename _Tp, typename _Distance>
inline _Distance* 
DistanceType(const RandomAccessIterator<_Tp, _Distance>&) {
  return (_Distance*)(0);
}

template <typename _Tp>
inline ptrdiff_t* DistanceType(const _Tp*) {
  return (ptrdiff_t*)(0);
}

# define ITERATOR_CATEGORY(i) IteratorCategory(i)
# define DISTANCE_TYPE(i)     DistanceType(i)
# defien VALUE_TYPE(i)        ValueType(i)

#endif // ESTL_PARTIAL_SPECIALZATION

template <typename _InputIterator, typename _Distance>
inline void __Distance(_InputIterator begin, 
    _InputIterator end, _Distance& n, InputIteratorTag) {
  while (begin != end) {
    ++begin;
    ++n;
  }
}

template <typename _RandomAccessIterator, typename _Distance>
inline void __Distance(_RandomAccessIterator begin, 
    _RandomAccessIterator end, _Distance& n, RandomAccessIteratorTag) {
  n += end - begin;
}

template <typename _InputIterator, typename _Distance>
inline void 
Distance(_InputIterator begin, _InputIterator end, _Distance& n) {
  __Distance(begin, end, n, IteratorCategory(begin));
}

#ifdef ESTL_PARTIAL_SPECIALZATION

template <typename _InputIterator>
inline typename IteratorTraits<_InputIterator>::DifferenceType
__Distance(_InputIterator begin, _InputIterator end, InputIteratorTag) {
  typename IteratorTraits<_InputIterator>::DistanceType n = 0;
  while (begin != end) {
    ++begin;
    ++n;
  }
  return n;
}

template <typename _RandomAccessIterator>
inline typename IteratorTraits<_RandomAccessIterator>::DistanceType
__Distance(_RandomAccessIterator begin, 
    _RandomAccessIterator end, RandomAccessIteratorTag) {
  return end - begin;
}

template <typename _InputIterator>
inline typename IteratorTraits<_InputIterator>::DistanceType
Distance(_InputIterator begin, _InputIterator end) {
  typedef typename IteratorTraits<_InputIterator>::IteratorCategory 
    _Category;
  return __Distance(begin, end, _Category());
}

#endif // ESTL_PARTIAL_SPECIALZATION

template <typename _InputIterator, typename _Distance>
inline void __Advance(_InputIterator& i, _Distance n, InputIteratorTag) {
  while (n--)
    ++i;
}

template <typename _BidirectionalIterator, typename _Distance>
inline void __Advance(
    _BidirectionalIterator& i, _Distance n, BidirectionalIteratorTag) {
  if (n >= 0) {
    while (n--)
      ++i;
  }
  else {
    while (n++)
      --i;
  }
}

template <typename _RandomAccessIterator, typename _Distance>
inline void __Advance(
    _RandomAccessIterator& i, _Distance n, RandomAccessIteratorTag) {
  i += n;
}

template <typename _InputIterator, typename _Distance>
inline void Advance(_InputIterator& i, _Distance n) {
  __Advance(i, n, IteratorCategory(i));
}

}

#endif  // __ESTL_ITERATOR_BASE_HEADER_H__
