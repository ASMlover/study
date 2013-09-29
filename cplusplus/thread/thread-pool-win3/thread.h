//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __THREAD_HEADER_H__
#define __THREAD_HEADER_H__

class thread_t : noncopyable {
  HANDLE thread_; 
  HANDLE create_event_;
  void (*routine_)(void*);
  void* arg_;
public:
  thread_t(void (*routine)(void*), void* arg)
    : thread_(NULL)
    , create_event_(NULL)
    , routine_(routine)
    , arg_(arg)
  {
  }

  ~thread_t(void)
  {
    if (NULL != thread_)
      join();
  }

  inline void 
  start(void)
  {
    create_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    assert(NULL != create_event_);

    thread_ = CreateThread(NULL, 0, &thread_t::s_routine, this, 0, NULL);
    assert(NULL != thread_);

    WaitForSingleObject(create_event_, INFINITE);
    CloseHandle(create_event_);
    create_event_ = NULL;
  }

  inline void 
  join(void)
  {
    if (NULL != thread_) {
      WaitForSingleObject(thread_, INFINITE);
      CloseHandle(thread_);
      thread_ = NULL;
    }
  }
private:
  static DWORD WINAPI 
  s_routine(void* arg)
  {
    thread_t* thread = static_cast<thread_t*>(arg);
    assert(NULL != thread);

    SetEvent(thread->create_event_);

    if (NULL != thread->routine_)
      thread->routine_(thread->arg_);

    return 0;
  }
};

#endif  //! __THREAD_HEADER_H__
