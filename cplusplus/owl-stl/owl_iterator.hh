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

#include "owl_type_traits.hh"

namespace owl {

template <typename Iter>
inline typename IteratorTraits<Iter>::IteratorCategory iterator_category(const Iter& iter) {
  return typename IteratorTraits<Iter>::IteratorCategory();
}

template <typename Iter>
inline typename IteratorTraits<Iter>::ValueType value_type(const Iter& iter) {
  return IteratorTraits<Iter>::ValueType();
}

template <typename Iter>
inline typename IteratorTraits<Iter>::Pointer pointer(const Iter& iter) {
  return IteratorTraits<Iter>::Pointer();
}

template <typename Iter>
inline typename IteratorTraits<Iter>::Reference reference(const Iter& iter) {
  return IteratorTraits<Iter>::Reference();
}

template <typename Iter, typename Difference>
inline void advance_impl(Iter& iter, Difference n, std::input_iterator_tag) {
  while (n--)
    ++iter;
}

template <typename Iter, typename Difference>
inline void advance_impl(Iter& iter, Difference n, std::bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n--)
      ++iter;
  }
  else {
    while (n++)
      --iter;
  }
}

template <typename Iter, typename Difference>
inline void advance_impl(Iter& iter, Difference n, std::random_access_iterator_tag) {
  iter += n;
}

template <typename Iter, typename Difference>
inline void advance(Iter& iter, Difference n) {
  advance_impl(iter, n, iterator_category(iter));
}

template <typename Iter>
inline typename IteratorTraits<Iter>::DifferenceType distance_impl(Iter first, Iter last, std::input_iterator_tag) {
  typename IteratorTraits<Iter>::DifferenceType n = 0;
  for (; first != last; ++first)
    ++n;
  return n;
}

template <typename Iter>
inline typename IteratorTraits<Iter>::DifferenceType distance_impl(Iter first, Iter last, std::random_access_iterator_tag) {
  return last - first;
}

template <typename Iter>
inline typename IteratorTraits<Iter>::DifferenceType distance(Iter first, Iter last) {
  return distance_impl(first, last, iterator_category(first));
}

}
