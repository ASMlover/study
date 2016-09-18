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
#ifndef __SP_COUNTED_IMPL_HEADER_H__
#define __SP_COUNTED_IMPL_HEADER_H__

#include "sp_counted_base.h"

namespace sp {

template <typename T>
class CountedImplDelete : public CountedBase {
  T* ptr_;
public:
  explicit CountedImplDelete(T* p)
    : ptr_(p) {
  }

  virtual void Dispose(void) override {
    if (nullptr != ptr_)
      delete ptr_;
  }
};

template <typename P, typename D>
class CountedImplDestructor : public CountedBase {
  P ptr_;
  D dtor_;
public:
  explicit CountedImplDestructor(P p)
    : ptr_(p)
    , dtor_() {
  }

  CountedImplDestructor(P p, D& d)
    : ptr_(p)
    , dtor_(d) {
  }

  virtual void Dispose(void) override {
    if (nullptr != ptr_)
      dtor_(ptr_);
  }
};

}

#endif // __SP_COUNTED_IMPL_HEADER_H__
