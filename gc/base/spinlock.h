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
#include "base.h"
#if defined(BASE_WINDOWS)
# include <Windows.h>
#endif

namespace details {

inline long atomic_exchange(volatile long* p, long v, long old = 0) {
#if defined(BASE_POSIX)
  return __sync_val_compare_and_swap(p, old, v);
#else
  return InterlockedExchange(p, v);
#endif
}

inline long atomic_test_exchange(volatile long* p, long v, long old = 0) {
#if defined(BASE_POSIX)
  while (__sync_val_compare_and_swap(p, 0, 0) == 1) {
  }
#else
  while (InterlockedCompareExchange(p, 0, 0) == 1) {
  }
#endif
  return atomic_exchange(p, v, old);
}

inline long atomic_test_and_set(volatile long* p, long v) {
#if defined(BASE_POSIX)
  return __sync_lock_test_and_set(p, v);
#else
  return InterlockedExchange(p, v);
#endif
}

inline long atomic_test_and_test_and_set(volatile long* p, long v) {
#if defined(BASE_POSIX)
  while (__sync_val_compare_and_swap(p, 0, 0) == 1) {
  }
#else
  while (InterlockedCompareExchange(p, 0, 0) == 1) {
  }
#endif
  return atomic_test_and_set(p, v);
}

}

class ExchangeSpinlock : private UnCopyable {
  volatile long m_{};
public:
  void lock(void) {
    while (details::atomic_exchange(&m_, 1) == 1) {
    }
  }

  void unlock(void) {
    details::atomic_exchange(&m_, 0, 1);
  }
};

class TestExchangeSpinlock : private UnCopyable {
  volatile long m_{};
public:
  void lock(void) {
    while (details::atomic_test_exchange(&m_, 1) == 1) {
    }
  }

  void unlock(void) {
    details::atomic_exchange(&m_, 0, 1);
  }
};

class TestAndSetSpinlock : private UnCopyable {
  volatile long m_{};
public:
  void lock(void) {
    while (details::atomic_test_and_set(&m_, 1) == 1) {
    }
  }

  void unlock(void) {
    details::atomic_test_and_set(&m_, 0);
  }
};

class TestAndTestAndSetSpinlock : private UnCopyable {
  volatile long m_{};
public:
  void lock(void) {
    while (details::atomic_test_and_test_and_set(&m_, 1) == 1) {
    }
  }

  void unlock(void) {
    details::atomic_test_and_set(&m_, 0);
  }
};

class FastSpinlock : private UnCopyable {
  volatile std::atomic_flag m_;
public:
  FastSpinlock(void) {
    m_.clear();
  }

  void lock(void) {
    while (std::atomic_flag_test_and_set_explicit(&m_,
          std::memory_order_acquire)) {
    }
  }

  void unlock(void) {
    std::atomic_flag_clear_explicit(&m_, std::memory_order_release);
  }
};
