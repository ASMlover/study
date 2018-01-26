// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <atomic>
#include <memory>

namespace co {

template <typename _Mutex>
struct FakeLocker {
  explicit FakeLocker(_Mutex&) {}
};

struct Ref {
  std::atomic<long> ref_;

  Ref(const Ref&) = delete;
  Ref& operator=(const Ref&) = delete;

  Ref(void)
    : ref_(1) {
  }

  virtual ~Ref(void) {
  }

  void inc_ref(void) {
    ++ref_;
  }

  void dec_ref(void) {
    if (--ref_ == 0)
      delete this;
  }
};

template <typename T>
typename std::enable_if<std::is_base_of<Ref, T>::value, std::shared_ptr<T>>::type
shared_from_this(T* p) {
  p->inc_ref();
  return std::shared_ptr<T>(p, [](T* self) { self->dec_ref(); });
}

template <typename T>
typename std::enable_if<std::is_base_of<Ref, T>::value>::type
inc_ref(T* p) {
  p->inc_ref();
}

template <typename T>
typename std::enable_if<!std::is_base_of<Ref, T>::value>::type
inc_ref(T* /*p*/) {
}

template <typename T>
typename std::enable_if<std::is_base_of<Ref, T>::value>::type
dec_ref(T* p) {
  p->dec_ref();
}

template <typename T>
typename std::enable_if<!std::is_base_of<Ref, T>::value>::type
dec_ref(T* /*p*/) {
}

class RefGuard {
  Ref* ref_{};

  RefGuard(const RefGuard&) = delete;
  RefGuard& operator=(const RefGuard&) = delete;
public:
  template <typename T>
  explicit RefGuard(T* p)
    : ref_(static_cast<Ref*>(p)) {
    ref_->inc_ref();
  }

  template <typename T>
  explicit RefGuard(T& o)
    : ref_(static_cast<Ref*>(&o)) {
    ref_->inc_ref();
  }

  ~RefGuard(void) {
    ref_->dec_ref();
  }
};

}
