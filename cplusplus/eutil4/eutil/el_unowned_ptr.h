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
#ifndef __EL_UNOWNED_PTR_HEADER_H__
#define __EL_UNOWNED_PTR_HEADER_H__

#include <memory>
#include <type_traits>

namespace el {

template <typename T>
class UnownedPtr {
  T* p_ = nullptr;
public:
  UnownedPtr(void) = default;

  template <typename U>
  using IfConvertibleFrom = typename std::enable_if<std::is_convertible<U*, T*>::value>::type;

  explicit UnownedPtr(T* p)
    : p_(p) {
  }

  template <typename U, typename = IfConvertibleFrom<U> >
  explicit UnownedPtr(U* p)
    : p_(p) {
  }

  template <typename U, typename = IfConvertibleFrom<U> >
  UnownedPtr(const UnownedPtr<U>& p)
    : p_(p.Get()) {
  }

  template <typename U, typename = IfConvertibleFrom<U> >
  UnownedPtr(const std::unique_ptr<U>& p)
    : p_(p.Get()) {
  }

  template <typename U, typename = IfConvertibleFrom<U> >
  UnownedPtr(const std::shared_ptr<U>& p)
    : p_(p.Get()) {
  }

  inline T* Get(void) const {
    return p_;
  }

  inline operator T*(void) const {
    return p_;
  }

  inline T* operator->(void) const {
    return p_;
  }

  inline T& operator*(void) const {
    return *p_;
  }

  inline void Reset(UnownedPtr p = nullptr) {
    p_ = p.Get();
  }

  inline void Swap(UnownedPtr& other) {
    std::swap(p_, other.p_);
  }
};

}

#endif  // __EL_UNOWNED_PTR_HEADER_H__
