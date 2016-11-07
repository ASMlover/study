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
#ifndef TPP_BASIC_COUNTED_H_
#define TPP_BASIC_COUNTED_H_

#include <stdint.h>
#include <atomic>
#include <algorithm>

namespace tpp { namespace basic {

class CountedBase {
  std::atomic<uint32_t> shared_count_{1};
  std::atomic<uint32_t> weak_count_{1};

  CountedBase(const CountedBase&) = delete;
  CountedBase& operator=(const CountedBase&) = delete;
public:
  CountedBase(void) = default;
  virtual ~CountedBase(void) {}

  virtual void dispose(void) = 0;

  void destroy(void) {
    delete this;
  }

  void add_ref(void) {
    ++shared_count_;
  }

  bool add_ref_lock(void) {
    return 0 == shared_count_ ? false : (++shared_count_, true);
  }

  void del_ref(void) {
    if (0 == --shared_count_) {
      dispose();
      del_weak_ref();
    }
  }

  void add_weak_ref(void) {
    ++weak_count_;
  }

  void del_weak_ref(void) {
    if (0 == --weak_count_) {
      destroy();
    }
  }

  uint32_t use_count(void) const {
    return shared_count_;
  }
};

template <typename T>
class CountedDelte : public CountedBase {
  T* px_{nullptr};
public:
  explicit CountedDelte(T* p)
    : px_(p) {
  }

  virtual void dispose(void) override {
    if (nullptr != px_)
      delete px_;
  }
};

template <typename T, typename D>
class CountedDestructor : public CountedBase {
  T* px_{nullptr};
  D del_;
public:
  explicit CountedDestructor(T* p)
    : px_(p)
    , del_() {
  }

  CountedDestructor(T* p, D& d)
    : px_(p)
    , del_(d) {
  }

  virtual void dispose(void) override {
    if (nullptr != px_)
      del_(px_);
  }
};

class WeakCount;
class SharedCount {
  CountedBase* pi_{nullptr};

  friend class WeakCount;
public:
  SharedCount(void) = default;

  template <typename Y>
  explicit SharedCount(Y* p) {
    try {
      pi_ = new CountedDelte<Y>(p);
    }
    catch (...) {
      if (nullptr != p)
        delete p;
    }
  }

  template <typename Y, typename D>
  SharedCount(Y* p, D d) {
    try {
      pi_ = new CountedDestructor<Y, D>(p, d);
    }
    catch (...) {
      if (nullptr != p)
        d(p);
    }
  }

  ~SharedCount(void) {
    if (nullptr != pi_)
      pi_->del_ref();
  }

  SharedCount(const SharedCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->add_ref();
  }

  SharedCount(SharedCount&& r)
    : pi_(r.pi_) {
    r.pi_ = nullptr;
  }

  SharedCount(const WeakCount& r);

  SharedCount& operator=(const SharedCount& r) {
    CountedBase* tmp = r.pi_;
    if (pi_ != tmp) {
      if (nullptr != tmp)
        tmp->add_ref();
      if (nullptr != pi_)
        pi_->del_ref();
      pi_ = tmp;
    }
    return *this;
  }

  SharedCount& operator=(SharedCount&& r) {
    CountedBase* tmp = r.pi_;
    if (pi_ != tmp) {
      if (nullptr != pi_)
        pi_->del_ref();
      pi_ = tmp;
    }
    r.pi_ = nullptr;
    return *this;
  }

  void swap(SharedCount& r) {
    std::swap(pi_, r.pi_);
  }

  uint32_t use_count(void) const {
    return nullptr != pi_ ? pi_->use_count() : 0;
  }

  bool unique(void) const {
    return 1 == use_count();
  }

  bool empty(void) const {
    return nullptr == pi_;
  }

  bool operator==(const SharedCount& r) const {
    return pi_ ==  r.pi_;
  }

  bool operator<(const SharedCount& r) const {
    return std::less<CountedBase*>()(pi_, r.pi_);
  }
};

class WeakCount {
  CountedBase* pi_{nullptr};

  friend class SharedCount;
public:
  WeakCount(void) = default;

  explicit WeakCount(const SharedCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->add_weak_ref();
  }

  ~WeakCount(void) {
    if (nullptr != pi_)
      pi_->del_weak_ref();
  }

  WeakCount(const WeakCount& r)
    : pi_(r.pi_) {
    if (nullptr != pi_)
      pi_->add_weak_ref();
  }

  WeakCount(WeakCount&& r)
    : pi_(r.pi_) {
    r.pi_ = nullptr;
  }

  WeakCount& operator=(const WeakCount& r) {
    CountedBase* tmp = r.pi_;
    if (tmp != pi_) {
      if (nullptr != tmp)
        tmp->add_weak_ref();
      if (nullptr != pi_)
        pi_->del_weak_ref();
      pi_ = tmp;
    }
    return *this;
  }

  WeakCount& operator=(WeakCount&& r) {
    CountedBase* tmp = r.pi_;
    if (tmp != pi_) {
      if (nullptr != pi_)
        pi_->del_weak_ref();
      pi_ = tmp;
    }
    r.pi_ = nullptr;
    return *this;
  }

  WeakCount& operator=(const SharedCount& r) {
    CountedBase* tmp = r.pi_;
    if (tmp != pi_) {
      if (nullptr != tmp)
        tmp->add_weak_ref();
      if (nullptr != pi_)
        pi_->del_weak_ref();
      pi_ = tmp;
    }
    return *this;
  }

  void swap(WeakCount& r) {
    std::swap(pi_, r.pi_);
  }

  uint32_t use_count(void) const {
    return nullptr != pi_ ? pi_->use_count() : 0;
  }

  bool empty(void) const {
    return nullptr == pi_;
  }

  bool operator==(const WeakCount& r) const {
    return pi_ == r.pi_;
  }

  bool operator<(const WeakCount& r) const {
    return std::less<CountedBase*>()(pi_, r.pi_);
  }
};

SharedCount::SharedCount(const WeakCount& r)
  : pi_(r.pi_) {
  if (nullptr != pi_ && !pi_->add_ref_lock())
    pi_ = nullptr;
}

}}

#endif // TPP_BASIC_COUNTED_H_
