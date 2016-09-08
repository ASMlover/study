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
#ifndef __TYR_COUNTED_IMPL_HEADER_H__
#define __TYR_COUNTED_IMPL_HEADER_H__

#include "TCountedBase.h"

namespace tyr {

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

template <typename T, class D>
class CountedImplDestructor : public CountedBase {
  T* ptr_;
  D del_;
public:
  explicit CountedImplDestructor(T* p)
    : ptr_(p)
    , del_() {
  }

  CountedImplDestructor(T* p, D& d)
    : ptr_(p)
    , del_(d) {
  }

  virtual void Dispose(void) override {
    if (nullptr != ptr_)
      del_(ptr_);
  }
};

}

#endif // __TYR_COUNTED_IMPL_HEADER_H__
