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
#ifndef CO_UTILS_H_
#define CO_UTILS_H_

#include <cstdint>
#include <atomic>
#include <memory>

namespace co {

class UnCopyable {
  UnCopyable(const UnCopyable&) = delete;
  UnCopyable& operator=(const UnCopyable&) = delete;
protected:
  UnCopyable(void) = default;
  ~UnCopyable(void) = default;
};

struct FakeLock {
  template <typename _Mutex>
  explicit FakeLock(_Mutex&) = default;
};

class IntrusiveRef : private UnCopyable {
  std::atomic<uint32_t> rc_;
public:
  IntrusiveRef(void)
    : rc_(1) {
  }

  virtual ~IntrusiveRef(void) {
  }

  void increment(void) {
    ++rc_;
  }

  void decrement(void) {
    if (0 == --rc_)
      delete this;
  }
};

template <typename T>
typename std::enable_if<std::is_base_of<IntrusiveRef, T>::value, std::shared_ptr<T>>::type
shared_from_this(T* p) {
  p->increment();
  return std::shared_ptr<T>(p, [](T* self) { self->decrement(); })
}

template <typename T, typename... Args>
typename std::enable_if<std::is_base_of<IntrusiveRef, T>::value, std::shared_ptr<T>>::type
make_shared(Args&&... args) {
  T* p = new T(std::forward<Args>(args)...);
  return std::shared_ptr<T>(p, [](T* self) { self->decrement(); })
}

template <typename T>
typename std::enable_if<std::is_base_of<IntrusiveRef, T>::value>::type
increment_ref(T* p) {
  p->increment();
}

template <typename T>
typename std::enable_if<!std::is_base_of<IntrusiveRef, T>::value>::type
increment_ref(T* p) {
}

template <typename T>
typename std::enable_if<std::is_base_of<IntrusiveRef, T>::value>::type
decrement_ref(T* p) {
  p->decrement();
}

template <typename T>
typename std::enable_if<!std::is_base_of<IntrusiveRef, T>::value>::type
decrement_ref(T* p) {
}

class IntrusiveRefGuard : private UnCopyable {
  IntrusiveRef* ref_;
public:
  template <typename T>
  explicit IntrusiveRefGuard(T* r)
    : ref_(static_cast<IntrusiveRef*>(r)) {
    ref_->increment();
  }

  template <typename T>
  explicit IntrusiveRefGuard(T& r)
    : ref_(static_cast<IntrusiveRef*>(&r)) {
    ref_->increment();
  }

  ~IntrusiveRefGuard(void) {
    ref_->decrement();
  }
};

}

#endif // CO_UTILS_H_
