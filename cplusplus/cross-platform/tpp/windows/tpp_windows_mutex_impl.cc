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
#include <Windows.h>
#include <stdint.h>
#include <iostream>

namespace tpp {

class CriticalSection {
  CRITICAL_SECTION cs_;

  CriticalSection(const CriticalSection&) = delete;
  CriticalSection& operator=(const CriticalSection&) = delete;
public:
  CriticalSection(void) {
    InitializeCriticalSectionEx(&cs_, 4000, 0);
  }

  ~CriticalSection(void) {
    DeleteCriticalSection(&cs_);
  }

  void lock(void) {
    EnterCriticalSection(&cs_);
  }

  bool try_lock(void) {
    return TRUE == TryEnterCriticalSection(&cs_);
  }

  void unlock(void) {
    LeaveCriticalSection(&cs_);
  }
};

struct XTime {
  int64_t sec;
  int64_t nanosec;
};

class NonRecursiveMutex {
  CriticalSection cs_{};
  long thread_id_{-1};
  int count_{};

  NonRecursiveMutex(const NonRecursiveMutex&) = delete;
  NonRecursiveMutex& operator=(const NonRecursiveMutex&) = delete;
public:
  NonRecursiveMutex(void) = default;
  ~NonRecursiveMutex(void) = default;

  void lock(void) {
    if (thread_id_ != static_cast<long>(GetCurrentThreadId())) {
      cs_.lock();
      thread_id_ = static_cast<long>(GetCurrentThreadId());
    }
    ++count_;
  }

  bool try_lock(void) {
    int res = WAIT_TIMEOUT;
    if (thread_id_ != static_cast<long>(GetCurrentThreadId())) {
      if (cs_.try_lock())
        res = WAIT_OBJECT_0;
      else
        res = WAIT_TIMEOUT;
    }
    else {
      res = WAIT_OBJECT_0;
    }

    if (res != WAIT_OBJECT_0 && res != WAIT_ABANDONED) {
    }
    else if (1 < ++count_) {
      --count_;
      res = WAIT_TIMEOUT;
    }
    else {
      thread_id_ = static_cast<long>(GetCurrentThreadId());
    }

    return res == WAIT_OBJECT_0 || res == WAIT_ABANDONED;
  }

  void unlock(void) {
    if (0 == --count_) {
      thread_id_ = -1;
      cs_.unlock();
    }
  }

  void show(void) {
    std::cout << "NonRecursiveMutex::show @ThreadID: "
      << thread_id_ << ", @count: " << count_ << std::endl;
  }
};

}

#include <iostream>
void test_NonRecursiveMutex(void) {
  tpp::NonRecursiveMutex mtx;
  mtx.show();

  mtx.lock();
  mtx.show();

  mtx.lock();
  mtx.show();

  mtx.unlock();
  mtx.show();

  mtx.unlock();
  mtx.show();
}
