// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __RIGHTVALUE_REFERENCE_HEADER_H__
#define __RIGHTVALUE_REFERENCE_HEADER_H__


template <typename T>
struct RemoveReference {
  typedef T Type;
};

template <typename T> 
struct RemoveReference<T&> {
  typedef T Type;
};

template <typename T>
struct RemoveReference<T&&> {
  typedef T Type;
};


template <typename T>
inline T&& Forward(typename RemoveReference<T>::Type& t) {
  return static_cast<T&&>(t);
}

template <typename T>
inline T&& Forward(typename RemoveReference<T>::Type&& t) {
  return static_cast<T&&>(t);
}


template <typename T>
inline typename RemoveReference<T>::Type&& Move(T&& t) {
  return static_cast<typename RemoveReference<T>::Type&&>(t);
}


template <typename T>
inline void Swap(T& lhs, T& rhs) {
  T tmp = Move(lhs);
  lhs = Move(rhs);
  rhs = Move(tmp);
}


#endif  // __RIGHTVALUE_REFERENCE_HEADER_H__
