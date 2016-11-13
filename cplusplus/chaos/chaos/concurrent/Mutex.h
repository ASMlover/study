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
#ifndef CHAOS_CONCURRENT_MUTEX_H
#define CHAOS_CONCURRENT_MUTEX_H

#include <atomic>
#include <utility>
#include "../Platform.h"
#include "../UnCopyable.h"
#include "../error/SystemError.h"

#if defined(CHAOS_WINDOWS)
# include "windows/Mutex.h"
#else
# include "posix/Mutex.h"
#endif

namespace chaos {

struct DeferLock {};
struct TryToLock {};
struct AdoptLock {};

template <typename MutexType>
class ScopedLock : private UnCopyable {
  MutexType* m_{};
  bool owned_{};
public:
  explicit ScopedLock(MutexType& m)
    : m_(&m)
    , owned_(true) {
    m_->lock();
  }

  ScopedLock(MutexType& m, DeferLock)
    : m_(&m)
    , owned_(false) {
  }

  ScopedLock(MutexType& m, TryToLock)
    : m_(&m)
    , owned_(m_->try_lock()) {
  }

  ScopedLock(MutexType& m, AdoptLock)
    : m_(&m)
    , owned_(true) {
  }

  ~ScopedLock(void) {
    if (owned_)
      m_->unlock();
  }

  ScopedLock(ScopedLock&& r)
    : m_(r.m_)
    , owned_(r.owned_) {
    r.m_ = nullptr;
    r.owned_ = false;
  }

  ScopedLock& operator=(ScopedLock& r) {
    if (this != &r) {
      if (owned_)
        m_->unlock();
      m_ = r.m_;
      owned_ = r.owned_;
    }
    r.m_ = nullptr;
    r.owned_ = false;
    return *this;
  }

  void swap(ScopedLock& r) {
    std::swap(m_, r.m_);
    std::swap(owned_, r.owned_);
  }

  void lock(void) {
    if (nullptr == m_)
      __chaos_throw_error(EPERM, "ScopedLock::lock: references null mutex");
    if (owned_)
      __chaos_throw_error(EDEADLK, "ScopedLock::lock: already locked");
    m_->lock();
    owned_ = true;
  }

  bool try_lock(void) {
    if (nullptr == m_)
      __chaos_throw_error(EPERM, "ScopedLock::try_lock: references null mutex");
    if (owned_)
      __chaos_throw_error(EDEADLK, "ScopedLock::try_lock: already locked");
    return owned_ = m_->try_lock();
  }

  void unlock(void) {
    if (!owned_)
      __chaos_throw_error(EPERM, "ScopedLock::unlock: not locked");
    m_->unlock();
    owned_ = false;
  }

  MutexType* release(void) {
    MutexType* m = m_;
    m_ = nullptr;
    owned_ = false;
    return m;
  }

  bool owned_lock(void) const {
    return owned_;
  }

  explicit operator bool(void) const {
    return owned_;
  }

  MutexType* get_mutex(void) const {
    return m_;
  }
};

class Mutex : private UnCopyable {
};

class FastMutex : private UnCopyable {
  volatile std::atomic_flag m_;
public:
  FastMutex(void) {
    m_.clear();
  }

  void lock(void) {
    while (std::atomic_flag_test_and_set_explicit(&m_, std::memory_order_acquire)) {
    }
  }

  bool try_lock(void) {
    return !std::atomic_flag_test_and_set_explicit(&m_, std::memory_order_acquire);
  }

  void unlock(void) {
    std::atomic_flag_clear_explicit(&m_, std::memory_order_release);
  }
};

}

#endif // CHAOS_CONCURRENT_MUTEX_H
