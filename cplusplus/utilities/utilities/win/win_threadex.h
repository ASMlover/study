// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __UTIL_WIN_THREADEX_HEADER_H__
#define __UTIL_WIN_THREADEX_HEADER_H__


namespace util {

class ThreadEx : private UnCopyable {
  HANDLE       thread_;
  HANDLE       start_event_;
  RoutinerType routine_;
  void*        argument_;
public:
  ThreadEx(void) 
    : thread_(nullptr) 
    , start_event_(nullptr) 
    , routine_(nullptr) 
    , argument_(nullptr) {
  }

  ~ThreadEx(void) {
    Join();
  }

  void Create(RoutinerType routine, void* argument = nullptr) {
    routine_ = routine;
    argument_ = argument;

    start_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    UTIL_ASSERT(nullptr != start_event_);

    thread_ = static_cast<HANDLE>(_beginthreadex(nullptr, 
          0, &ThreadEx::Routine, this, 0, nullptr));
    UTIL_ASSERT(nullptr != thread_);

    WaitForSingleObject(start_event_, INFINITE);
    CloseHandle(start_event_);
  }

  void Join(void) {
    if (nullptr != thread_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = nullptr;
    }
  }

  uint32_t GetID(void) const {
    return GetCurrentThreadId();
  }

  void Kill(void) {
    if (nullptr != thread_) {
      TerminateThread(thread_, 0);

      CloseHandle(thread_, 0);
      thread_ = nullptr;
    }
  }
private:
  static UINT WINAPI Routine(void* argument) {
    ThreadEx* self = static_cast<ThreadEx*>(argument);
    if (nullptr == self)
      return 0;

    SetEvent(self->start_event_);

    if (nullptr != self->routine_) 
      self->routine_(self->argument_);

    return 0;
  }
};

}


#endif  // __UTIL_WIN_THREADEX_HEADER_H__
