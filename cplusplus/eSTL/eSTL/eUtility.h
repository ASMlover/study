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
#ifndef __ESTL_UTILITY_HEADER_H__
#define __ESTL_UTILITY_HEADER_H__

#include "eConfig.h"

namespace estl {

namespace rel_ops {

  template <typename T>
  inline bool operator!=(const T& x, const T& y) {
    return !(x == y);
  }

  template <typename T>
  inline bool operator>(const T& x, const T& y) {
    return (y < x);
  }

  template <typename T>
  inline bool operator<=(const T& x, const T& y) {
    return !(y < x);
  }

  template <typename T>
  inline bool operator>=(const T& x, const T& y) {
    return !(x < y);
  }

}

template <typename T1, typename T2>
struct Pair {
  typedef T1 FirstType;
  typedef T2 SecondType;

  T1 first;
  T2 second;

  Pair(void);
  explicit Pair(const T1& x);
  Pair(const T1& x, const T2& y);

  template <typename U, typename V>
  explicit Pair(const Pair<U, V>& p);
};

template <typename T>
struct UseSelf {
  typedef T ResultType;

  inline const T& operator()(const T& x) const {
    return x;
  }
};

template <typename PairT>
struct UseFirst {
  typedef typename PairT::FirstType ResultType;

  inline ResultType& operator()(const PairT& x) const {
    return x.first;
  }
};

template <typename PairT>
struct UseSecond {
  typedef typename PairT::SecondType ResultType;

  inline ResultType& operator()(const PairT& x) const {
    return x.second;
  }
};

template <typename T1, typename T2>
inline Pair<T1, T2>::Pair(void)
  : first()
  , second() {
  // Empty
}

template <typename T1, typename T2>
inline Pair<T1, T2>::Pair(const T1& x)
  : first(x)
  , second() {
  // Empty
}

template <typename T1, typename T2>
inline Pair<T1, T2>::Pair(const T1& x, const T2& y)
  : first(x)
  , second(y) {
  // Empty
}

template <typename T1, typename T2>
inline bool operator==(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return ((a.first == b.first) && (a.second == b.second));
}

template <typename T1, typename T2>
inline bool operator<(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return ((a.first < b.first) || 
      (!(b.first < a.first) && (a.second < b.second)));
}

template <typename T1, typename T2>
inline bool operator!=(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return !(a == b);
}

template <typename T1, typename T2>
inline bool operator>(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return b < a;
}

template <typename T1, typename T2>
inline bool operator>=(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return !(a < b);
}

template <typename T1, typename T2>
inline bool operator<=(const Pair<T1, T2>& a, const Pair<T1, T2>& b) {
  return !(b < a);
}

template <typename T1, typename T2>
inline Pair<T1, T2> MakePair(T1 a, T2 b) {
  return Pair<T1, T2>(a, b);
}

template <typename T1, typename T2>
inline Pair<T1, T2> MakePairRef(const T1& a, const T2& b) {
  return Pair<T1, T2>(a, b);
}

}

#endif  // __ESTL_UTILITY_HEADER_H__
