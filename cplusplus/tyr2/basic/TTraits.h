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
#ifndef __TYR_BASIC_TRAITS_HEADER_H__
#define __TYR_BASIC_TRAITS_HEADER_H__

#include <type_traits>

namespace tyr { namespace basic {

// disable if `traits`
template <bool B, typename T = void>
struct DisableIfImpl {
  typedef T type;
};

template <typename T>
struct DisableIfImpl<true, T> {};

template <typename Cond, typename T = void>
struct DisableIf : public DisableIfImpl<Cond::value, T> {};

// add reference `traits`
template <typename T>
struct AddReferenceImpl {
  typedef T& type;
};

template <typename T>
struct AddReferenceImpl<T&&> {
  typedef T&& type;
};

template <typename T>
struct AddReference {
  typedef typename AddReferenceImpl<T>::type type;
};

template <typename T>
struct AddReference<T&> {
  typedef T& type;
};

template <typename Cond, typename T = void>
using DisableIf_t = typename DisableIf<Cond, T>::type;

template <typename T>
using AddReference_t = typename AddReference<T>::type;

#if __cplusplus < 201402L
  template <typename T>
  using RemoveCV_t = typename std::remove_cv<T>::type;

  template <typename T>
  using RemoveRef_t = typename std::remove_reference<T>::type;

  template <typename T>
  using RemovePtr_t = typename std::remove_pointer<T>::type;

  template <bool B, typename T = void>
  using EnableIf_t = typename std::enable_if<B, T>::type;

  template <typename T>
  using AddPointer_t = typename std::add_pointer<T>::type;
#else
# define RemoveCV_t std::remove_cv_t
# define RemoveRef_t std::remove_reference_t
# define RemovePtr_t std::remove_pointer_t
# define EnableIf_t std::enable_if_t
#endif

}}

#endif // __TYR_BASIC_TRAITS_HEADER_H__
