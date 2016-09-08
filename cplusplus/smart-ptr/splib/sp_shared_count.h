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
#ifndef __SP_SHARED_COUNT_HEADER_H__
#define __SP_SHARED_COUNT_HEADER_H__

#include "sp_counted_impl.h"

namespace sp {

class SharedCount {
  CountedBase* ptr_;
public:
  SharedCount(void)
    : ptr_(nullptr) {
  }

  template <typename Y>
  explicit SharedCount(Y* p)
    : ptr_(nullptr) {
    ptr_ = new CountedImplDelete<Y>(p);
  }

  template <typename Y, typename D>
  SharedCount(Y* p, D d)
    : ptr_(nullptr) {
    ptr_ = new CountedImplDestructor<Y, D>(p, d);
  }

  ~SharedCount(void) {
    if (nullptr != ptr_)
      ptr_->Release();
  }

  SharedCount(const SharedCount& other)
    : ptr_(other.ptr_) {
    if (nullptr != ptr_)
      ptr_->AddRefCopy();
  }

  SharedCount(SharedCount&& other)
    : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  SharedCount& operator=(const SharedCount& other) {
    CountedBase* tmp = other.ptr_;
    if (tmp != ptr_) {
      if (nullptr != tmp)
        tmp->AddRefCopy();
      if (nullptr != ptr_)
        ptr_->Release();
      ptr_ = tmp;
    }

    return *this;
  }

  void Swap(SharedCount& other) {
    CountedBase* tmp = other.ptr_;
    other.ptr_ = ptr_;
    ptr_ = tmp;
  }

  long UseCount(void) const {
    return nullptr != ptr_ ? ptr_->UseCount() : 0;
  }

  bool Unique(void) const {
    return UseCount() == 1;
  }
};

class WeakCount {
};

}

#endif // __SP_SHARED_COUNT_HEADER_H__
