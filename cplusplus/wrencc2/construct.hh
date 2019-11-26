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

template <typename _T1, typename _T2>
inline void _Construct(_T1* p, const _T2& v) noexcept {
  new ((void*)p) _T1(v);
}

template <typename _T1, typename _T2>
inline void _Construct(_T1* p, _T2&& v) noexcept {
  new ((void*)p) _T1(std::move<_T2>(v));
}

template <typename _T1>
inline void _Construct(_T1* p) noexcept {
  new ((void*)p) _T1();
}

template <typename _Tp>
inline void _Destroy(_Tp* p) noexcept {
  p->~_Tp();
}

template <typename _ForwardIterator>
inline void _DestroyAux(_ForwardIterator first, _ForwardIterator last) noexcept {
  for (; first != last; ++first)
    _Destroy(&*first);
}

template <typename _ForwardIterator>
inline void _Destroy(_ForwardIterator first, _ForwardIterator) noexcept {
  _DestroyAux(first, last);
}

inline void _Destroy(char*, char*) noexcept {}
inline void _Destroy(i8_t*, i8_t*) noexcept {}
inline void _Destroy(u8_t*, u8_t*) noexcept {}
inline void _Destroy(i16_t*, i16_t*) noexcept {}
inline void _Destroy(u16_t*, u16_t*) noexcept {}
inline void _Destroy(i32_t*, i32_t*) noexcept {}
inline void _Destroy(u32_t*, u32_t*) noexcept {}
inline void _Destroy(i64_t*, i64_t*) noexcept {}
inline void _Destroy(u64_t*, u64_t*) noexcept {}
inline void _Destroy(float*, float*) noexcept {}
inline void _Destroy(double*, double*) noexcept {}

template <typename _T1>
inline void construct(_T1* p) noexcept {
  _Construct(p);
}

template <typename _T1, typename _T2>
inline void construct(_T1* p, const _T2& v) noexcept {
  _Construct(p, v);
}

template <typename _T1, typename _T2>
inline void construct(_T1* p, _T2&& v) noexcept {
  _Construct(p, std::move<_T2>(v));
}

template <typename _Tp>
inline void destroy(_Tp* p) noexcept {
  _Destroy(p);
}

template <typename _ForwardIterator>
inline void destroy(_ForwardIterator first, _ForwardIterator last) noexcept {
  _Destroy(first, last);
}

}
