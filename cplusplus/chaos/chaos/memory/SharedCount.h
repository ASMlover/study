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
#ifndef CHAOS_MEMORY_SHAREDCOUNT_H
#define CHAOS_MEMORY_SHAREDCOUNT_H

#include <stdint.h>
#include <atomic>
#include <algorithm>
#include "../UnCopyable.h"

namespace chaos {

class CountedBase : private UnCopyable {
  std::atmoic<uint32_t> shared_count_{1};
  std::atmoic<uint32_t> weak_count_{1};
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
    if (0 == --weak_count_)
      destroy();
  }

  uint32_t use_count(void) const {
    return shared_count_;
  }
};

template <typename T>
class CountedDelete : public CountedBase {
  T* px_{};
public:
  explicit CountedDelete(T* p)
    : px_(p) {
  }

  virtual void dispose(void) override {
    if (nullptr != px_)
      delete px_;
  }
};

template <typename T, typename D>
class CountedDestructor : public CountedBase {
  T* px_{};
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
  CountedBase* pc_{};

  friend class WeakCount;
public:
  SharedCount(void) = default;
};

}

#endif // CHAOS_MEMORY_SHAREDCOUNT_H
