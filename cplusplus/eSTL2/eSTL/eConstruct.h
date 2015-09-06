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
#ifndef __ESTL_CONSTRUCT_HEADER_H__
#define __ESTL_CONSTRUCT_HEADER_H__

#include "eTypeTraits.h"

namespace estl {

template <typename _T1, typename _T2>
inline void Construct(_T1* p, const _T2& value) {
  new ((void*)p) _T1(value);
}

template <typename _T1>
inline void Construct(_T1* p) {
  new ((void*)p) _T1();
}

template <typename _Tp>
inline void Destroy(_Tp* p) {
  p->~_Tp();
}

template <typename _ForwardIterator>
inline void Destroy(_ForwardIterator begin, _ForwardIterator end) {
  _Destroy(begin, end);
}

template <typename _ForwardIterator>
inline void 
_Destroy(_ForwardIterator begin, _ForwardIterator end, TrueType) {
}

template <typename _ForwardIterator>
inline void 
_Destroy(_ForwardIterator begin, _ForwardIterator end, FalseType) {
  for (; begin != end; ++begin)
    Destroy(&*first);
}

template <typename _ForwardIterator, typename _Tp>
inline void _Destroy(_ForwardIterator begin, _ForwardIterator end, _Tp*) {
  typedef typename TypeTraits<_Tp>::HasTrivialDestructor TrivialDestructor;
  _Destroy(begin, end, TrivialDestructor());
}

template <typename _ForwardIterator>
inline void _Destroy(_ForwardIterator begin, _ForwardIterator end) {
  // _Destroy(begin, end, VALUE_TYPE(begin));
}

inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
inline void _Destroy(wchar_t*, wchar_t*) {}

}

#endif  // __ESTL_CONSTRUCT_HEADER_H__
