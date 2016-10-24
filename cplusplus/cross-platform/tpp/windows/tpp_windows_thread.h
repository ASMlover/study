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
#ifndef TPP_WINDOWS_THREAD_H_
#define TPP_WINDOWS_THREAD_H_

#include <Windows.h>
#include <process.h>
#include <functional>
#include "../tpp_types.h"

namespace tpp {

typedef std::function<void (void*)> ThreadCallback;

class Thread : private UnCopyable {
  HANDLE entry_event_{nullptr};
  HANDLE exit_event_{nullptr};
  HANDLE thread_{nullptr};
  ThreadCallback closure_{nullptr};
  void* argument_{nullptr};
private:
  static void CALLBACK apc_callback(ULONG_PTR) {
  }

  static UINT WINAPI thread_callback(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    if (nullptr == thread)
      return 0;

    SetEvent(thread->entry_event_);
    if (nullptr != thread->closure_)
      thread->closure_(thread->argument_);

    SetEvent(thread->exit_event_);
    SleepEx(INFINITE, TRUE);
    return 0;
  }

  void start_thread(void) {
    entry_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == entry_event_)
      __tpp_throw(GetLastError(), "windows.Thread.start_thread: entry event failed");

    exit_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == exit_event_)
      __tpp_throw(GetLastError(), "windows.Thread.start_thread: exit event failed");

    thread_ = reinterpret_cast<HANDLE>(
        _beginthreadex(nullptr, 0, &Thread::thread_callback, this, 0, nullptr));
    if (nullptr == thread_) {
      int err = static_cast<int>(GetLastError());
      if (nullptr != entry_event_)
        CloseHandle(entry_event_);
      if (nullptr != exit_event_)
        CloseHandle(exit_event_);
      __tpp_throw(err, "windows.Thread.start_thread: create thread failed");
    }

    if (entry_event_) {
      WaitForSingleObject(entry_event_, INFINITE);
      CloseHandle(entry_event_);
    }
  }
public:
  Thread(const ThreadCallback& cb, void* arg)
    : closure_(cb)
    , argument_(arg) {
    start_thread();
  }

  ~Thread(void) {
    CloseHandle(thread_);
  }

  void join(void) {
    HANDLE handles[2] = {exit_event_, thread_};
    WaitForMultipleObjects(2, handles, FALSE, INFINITE);
    CloseHandle(exit_event_);

    QueueUserAPC(&Thread::apc_callback, thread_, 0);
    WaitForSingleObject(thread_, INFINITE);
  }
};

}

#endif // TPP_WINDOWS_THREAD_H_
