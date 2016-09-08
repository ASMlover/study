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
#ifndef TYR_SHARED_COUNT_HEADER_H
#define TYR_SHARED_COUNT_HEADER_H

#include "TCountedImpl.h"

namespace tyr {

class WeakCount;
class SharedCount {
  CountedBase* pi_;

  friend class WeakCount;
public:
  SharedCount(void)
    : pi_(nullptr) {
  }

  template <typename Y>
  explicit SharedCount(Y* p)
    : pi_(nullptr) {
    try {
      pi_ = new CountedImplDelete<Y>(p);
    }
    catch (...) {
      if (nullptr != p)
        delete p;
    }
  }

  template <typename Y, typename D>
  SharedCount(Y* p, D d)
    : pi_(nullptr) {
    try {
      pi_ = new CountedImplDestructor<Y, D>(p, d);
    }
    catch (...) {
      if (nullptr != p)
        d(p);
    }
  }

  ~SharedCount(void) {
    if (nullptr != pi_)
      pi_->Release();
  }

  SharedCount(const SharedCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->AddRefCopy();
  }

  SharedCount(SharedCount&& r)
    : pi_(r.pi_) {
    r.pi_ = nullptr;
  }

  explicit SharedCount(const WeakCount& r);

  SharedCount& operator=(const SharedCount& r) {
    CountedBase* tmp = r.pi_;

    if (tmp != pi_) {
      if (nullptr != tmp)
        tmp->AddRefCopy();
      if (nullptr != pi_)
        pi_->Release();
      pi_ = tmp;
    }

    return *this;
  }

  void Swap(SharedCount& r) {
    CountedBase* tmp = r.pi_;
    r.pi_ = pi_;
    pi_ = tmp;
  }

  uint32_t UseCount(void) const {
    return nullptr != pi_ ? pi_->UseCount() : 0;
  }

  bool Unique(void) const {
    return 1 == UseCount();
  }

  bool Empty(void) const {
    return nullptr == pi_;
  }

  friend inline bool operator==(const SharedCount& a, const SharedCount& b) {
    return a.pi_ == b.pi_;
  }

  friend inline bool operator<(const SharedCount& a, const SharedCount& b) {
    return std::less<CountedBase*>()(a.pi_, b.pi_);
  }
};

class WeakCount {
  CountedBase* pi_;

  friend class SharedCount;
public:
  WeakCount(void)
    : pi_(nullptr) {
  }

  explicit WeakCount(const SharedCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->WeakAddRef();
  }

  WeakCount(const WeakCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->WeakAddRef();
  }

  WeakCount(WeakCount&& r)
    : pi_(r.pi_) {
    r.pi_ = nullptr;
  }

  ~WeakCount(void) {
    if (nullptr != pi_)
      pi_->WeakRelease();
  }

  WeakCount& operator=(const SharedCount& r) {
    CountedBase* tmp = r.pi_;

    if (tmp != pi_) {
      if (nullptr != tmp)
        tmp->WeakAddRef();
      if (nullptr != pi_)
        pi_->WeakRelease();
      pi_ = tmp;
    }

    return *this;
  }

  WeakCount& operator=(const WeakCount& r) {
    CountedBase* tmp = r.pi_;

    if (tmp != pi_) {
      if (nullptr != tmp)
        tmp->WeakAddRef();
      if (nullptr != pi_)
        pi_->WeakRelease();
      pi_ = tmp;
    }

    return *this;
  }

  void Swap(WeakCount& r) {
    CountedBase* tmp = r.pi_;
    r.pi_ = pi_;
    pi_ = tmp;
  }

  uint32_t UseCount(void) const {
    return nullptr != pi_ ? pi_->UseCount() : 0;
  }

  bool Empty(void) const {
    return nullptr == pi_;
  }

  friend inline bool operator==(const WeakCount& a, const WeakCount& b) {
    return a.pi_ == b.pi_;
  }

  friend inline bool operator<(const WeakCount& a, const WeakCount& b) {
    return std::less<CountedBase*>()(a.pi_, b.pi_);
  }
};

inline SharedCount::SharedCount(const WeakCount& r)
  : pi_(r.pi_) {
  if (nullptr != pi_ && !pi_->AddRefLock())
    pi_ = nullptr;
}

}

#endif // TYR_SHARED_COUNT_HEADER_H
