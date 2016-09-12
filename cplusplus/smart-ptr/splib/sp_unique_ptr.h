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
#ifndef __SP_UNIQUE_PTR_HEADER_H__
#define __SP_UNIQUE_PTR_HEADER_H__

#include "sp_memory.h"

namespace sp {

template <typename T, typename D = DefaultDelete<T> >
class UniquePtr : private UnCopyable {
  T* ptr_;
  D del_;

  typedef UniquePtr<T, D> SelfType;
public:
  UniquePtr(void)
    : ptr_(nullptr)
    , del_() {
  }

  UniquePtr(std::nullptr_t)
    : ptr_(nullptr)
    , del_() {
  }

  template <typename Y>
  explicit UniquePtr(Y* p = nullptr)
    : ptr_(p)
    , del_() {
  }

  UniquePtr(T* p, D d)
    : ptr_(p)
    , del_(d) {
  }

  UniquePtr(T* p, D&& d)
    : ptr_(p)
    , del_(std::move(d)) {
  }

  ~UniquePtr(void) {
    if (nullptr != ptr_)
      del_(ptr_);
  }

  UniquePtr(UniquePtr&& r)
    : ptr_(r.ptr_)
    , del_(std::move(r.del_)) {
    r.ptr_ = nullptr;
  }

  void Reset(T* p = nullptr) {
    SelfType(p).Swap(*this);
  }

  void Swap(UniquePtr& r) {
    std::swap(ptr_, r.ptr_);
    std::swap(del_, r.del_);
  }

  T* Release(void) {
    T* p = Get();
    ptr_ = nullptr;
    return p;
  }

  D& GetDeleter(void) const {
    return del_;
  }

  UniquePtr& operator=(std::nullptr_t) {
    Reset();
    return *this;
  }

  UniquePtr& operator=(UniquePtr&& r) {
    if (this != &r) {
      Reset(r.Release());
      del_ = std::forward<D>(r.del_);
    }
    return *this;
  }

  T* operator->(void) const {
    return ptr_;
  }

  T& operator*(void) const {
    return *ptr_;
  }

  T* Get(void) const {
    return ptr_;
  }

  explicit operator bool(void) const {
    return nullptr != ptr_;
  }
};

}

#endif // __SP_UNIQUE_PTR_HEADER_H__
