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

namespace co {

struct S_Mutex {
  bool locked_{};

  void lock(void) {
    while (!locked_)
      locked_ = true;
  }

  bool try_lock(void) {
    bool r = !locked_;
    if (r)
      locked_ = true;
    return r;
  }

  void unlock(void) {
    locked_ = false;
  }
};

struct M_Mutex {
  volatile std::atomic_flag m_;

  M_Mutex(void) {
    m_.clear();
  }

  void lock(void) {
    while (std::atomic_flag_test_and_set_explicit(&m_, std::memory_order_acquire)) {
    }
  }

  bool try_lock(void) {
    bool r = !std::atomic_flag_test_and_set_explicit(&m_, std::memory_order_acquire);
    return r;
  }

  void unlock(void) {
    std::atomic_flag_clear_explicit(&m_, std::memory_order_release);
  }
};

#if defined(LIBCO_SINGLE_THREAD)
# define FastMutex S_Mutex
#else
# define FastMutex M_Mutex
#endif

}
