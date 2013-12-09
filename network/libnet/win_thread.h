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
#ifndef __WIN_THREAD_HEADER_H__
#define __WIN_THREAD_HEADER_H__

#include <windows.h>

class Thread {
  HANDLE thread_;
  HANDLE create_event_;
  void (*routine_)(void*);
  void* argument_;

  Thread(const Thread&);
  Thread& operator =(const Thread&);
public:
  explicit Thread(void)
    : thread_(NULL)
    , create_event_(NULL)
    , routine_(NULL)
    , argument_(NULL)
  {
  }

  ~Thread(void)
  {
    Stop();
  }

  inline void Start(void (*routine)(void*) = NULL, void* argument = NULL)
  {
    routine_ = routine;
    argument_ = argument;

    create_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == create_event_)
      return;

    thread_ = (HANDLE)_beginthreadex(NULL, 
        0, &Thread::Routine, this, 0, NULL);
    if (NULL == thread_) 
      abort();

    WaitForSingleObject(create_event_, INFINITE);
    CloseHandle(create_event_);
  }

  inline void Stop(void)
  {
    if (NULL != create_event_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = NULL;
    }
  }
private:
  static UINT WINAPI Routine(void* argument) 
  {
    Thread* self = static_cast<Thread*>(argument);
    if (NULL == self)
      return 0;

    SetEvent(self->create_event_);

    if (NULL != self->routine_)
      self->routine_(self->argument_);

    return 0;
  }
};

#endif  //! __WIN_THREAD_HEADER_H__
