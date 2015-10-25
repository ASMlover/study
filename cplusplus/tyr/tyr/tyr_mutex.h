// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_MUTEX_HEADER_H__
#define __TYR_MUTEX_HEADER_H__

namespace tyr {

class Mutex : private UnCopyable {
  std::mutex mutex_;
public:
  void Lock(void) {
    mutex_.lock();
  }

  void Unlock(void) {
    mutex_.Unlock();
  }

  std::mutex* InnerMutex(void) const {
    return &mutex_;
  }
};

class SpinlockMutex : private UnCopyable {
  std::atomic_flag flag_;
public:
  SpinlockMutex(void) tyr_noexcept {
    flag_.clear();
  }

  void Lock(void) {
    while (flag_.test_and_set(std::memory_order_acquire)) {
    }
  }

  void Unlock(void) {
    flag_.clear(std::memory_order_release);
  }
};

}

#endif  // __TYR_MUTEX_HEADER_H__
