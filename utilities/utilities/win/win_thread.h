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
#ifndef __UTIL_WIN_THREAD_HEADER_H__
#define __UTIL_WIN_THREAD_HEADER_H__

#ifndef _MT
# error "requires /MDd, /MD, /MTd or /MT compiler options"
#endif


namespace util {


class Thread : private UnCopyable {
  HANDLE              start_event_;
  HANDLE              thread_;
  SmartPtr<Routiner>  routine_;
public:
  Thread(void) 
    : start_event_(NULL)
    , thread_(NULL)
    , routine_(static_cast<Routiner*>(NULL)) {
  }

  ~Thread(void) {
    Join();
  }

  template <typename R>
  void Create(R routine, void* argument = NULL) {
    routine_ = SmartPtr<Routiner>(new ThreadRoutiner<R>(routine, argument));
    if (NULL == routine_.Get())
      return;

    start_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    UTIL_ASSERT(NULL != start_event_);

    thread_ = (HANDLE)_beginthreadex(NULL, 
        0, &Thread::Routine, this, 0, NULL);
    UTIL_ASSERT(NULL != thread_);

    WaitForSingleObject(start_event_, INFINITE);
    CloseHandle(start_event_);
  }

  void Join(void) {
    if (NULL != thread_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = NULL;
    }
  }

  uint32_t GetID(void) const {
    GetCurrentThreadId();
  }

  void Kill(void) {
    if (NULL != thread_) {
      TerminateThread(thread_, 0);

      CloseHandle(thread_);
      thread_ = 0;
    }
  }
private:
  static UINT WINAPI Routine(void* arg) {
    Thread* self = static_cast<Thread*>(arg);
    if (NULL == self)
      return 0;

    SetEvent(self->start_event_);

    self->routine_->Run();

    return 0;
  }
};


}

#endif  // __UTIL_WIN_THREAD_HEADER_H__
