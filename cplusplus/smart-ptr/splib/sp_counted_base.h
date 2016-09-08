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
#ifndef __SP_COUNTED_BASE_HEADER_H__
#define __SP_COUNTED_BASE_HEADER_H__

#include "sp.h"

namespace sp {

class CountedBase : private UnCopyable {
  long shared_count_;
  long weak_count_;
  mutable Mutex mutex_;
public:
  CountedBase(void)
    : shared_count_(1)
    , weak_count_(1) {
  }

  virtual ~CountedBase(void) {
  }

  virtual void Dispose(void) = 0;

  virtual void Destroy(void) {
    delete this;
  }

  void AddRefCopy(void) {
    LockerGuard<Mutex> guard(mutex_);
    ++shared_count_;
  }

  bool AddRefLock(void) {
    bool r;
    {
      LockerGuard<Mutex> guard(mutex_);
      r = shared_count_ == 0 ? false : (++shared_count_, true);
    }
    return r;
  }

  void Release(void) {
    long new_shared_count;
    {
      LockerGuard<Mutex> guard(mutex_);
      new_shared_count = --shared_count_;
    }

    if (0 == new_shared_count) {
      Dispose();
      WeakRelease();
    }
  }

  void WeakAddRef(void) {
    LockerGuard<Mutex> guard(mutex_);
    ++weak_count_;
  }

  void WeakRelease(void) {
    long new_weak_count;
    {
      LockerGuard<Mutex> guard(mutex_);
      new_weak_count = --weak_count_;
    }

    if (0 == weak_count_) {
      Destroy();
    }
  }

  long UseCount(void) const {
    long r;
    {
      LockerGuard<Mutex> guard(mutex_);
      r = shared_count_;
    }

    return r;
  }
};

}

#endif // __SP_COUNTED_BASE_HEADER_H__
