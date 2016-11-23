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
#include <process.h>
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

  enum {
    _MUTEX_SUCCESS,
    _MUTEX_NOMEM,
    _MUTEX_TIMEDOUT,
    _MUTEX_BUSY,
    _MUTEX_ERROR,
  };

  int do_lock(bool for_try = false) {
    int r = WAIT_TIMEOUT;
    long current_tid = static_cast<long>(GetCurrentThreadId());
    if (!for_try) {
      // non recursive mutex
      if (thread_id_ != current_tid)
        cs_.lock();
      r = WAIT_OBJECT_0;
    }
    else {
      if (thread_id_ != current_tid)
        r = cs_.try_lock() ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
      else
        r = WAIT_OBJECT_0;
    }

    if (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED)
      ;
    else if (1 < ++count_)
      r = (--count_, WAIT_TIMEOUT);
    else
      thread_id_ = current_tid;

    switch (r) {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
      return _MUTEX_SUCCESS;
    case WAIT_TIMEOUT:
      return for_try ? _MUTEX_BUSY : _MUTEX_TIMEDOUT;
    default:
      return _MUTEX_ERROR;
    }
  }

  int check_return(int r) {
    if (r != _MUTEX_SUCCESS)
      abort();
    return r;
  }

  int check_return(int r, int other) {
    if (r != _MUTEX_SUCCESS && r != other)
      abort();
    return r;
  }
public:
  NonRecursiveMutex(void) = default;
  ~NonRecursiveMutex(void) = default;

  void lock(void) {
    check_return(do_lock());
  }

  bool try_lock(void) {
    return check_return(do_lock(), _MUTEX_BUSY) == _MUTEX_SUCCESS;
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

tpp::NonRecursiveMutex g_nr_mutex;
tpp::CriticalSection g_mutex;
int g_counter;

static UINT WINAPI thread_closure_nr(void* /*arg*/) {
  for (int i = 0; i < 10; ++i) {
    g_nr_mutex.lock();

    ++g_counter;
    std::cout << "thread_closure_nr use NonRecursiveMutex: "
      << GetCurrentThreadId() << " : " << g_counter << std::endl;

    g_nr_mutex.unlock();
  }
  return 0;
}

static UINT WINAPI thread_closure(void* /*arg*/) {
  for (int i = 0; i < 10; ++i) {
    g_mutex.lock();
    g_mutex.lock();

    ++g_counter;
    std::cout << "thread_closure use CriticalSection: "
      << GetCurrentThreadId() << " : " << g_counter << std::endl;

    g_mutex.unlock();
    g_mutex.unlock();
  }
  return 0;
}

#include <iostream>
void test_NonRecursiveMutex(void) {
  HANDLE t1 = (HANDLE)_beginthreadex(NULL, 0, thread_closure_nr, NULL, 0, NULL);
  HANDLE t2 = (HANDLE)_beginthreadex(NULL, 0, thread_closure_nr, NULL, 0, NULL);

  WaitForSingleObject(t1, INFINITE);
  WaitForSingleObject(t2, INFINITE);

  CloseHandle(t1);
  CloseHandle(t2);
}

void test_RecursiveMutex(void) {
  HANDLE t1 = (HANDLE)_beginthreadex(NULL, 0, thread_closure, NULL, 0, NULL);
  HANDLE t2 = (HANDLE)_beginthreadex(NULL, 0, thread_closure, NULL, 0, NULL);

  WaitForSingleObject(t1, INFINITE);
  WaitForSingleObject(t2, INFINITE);

  CloseHandle(t1);
  CloseHandle(t2);
}
