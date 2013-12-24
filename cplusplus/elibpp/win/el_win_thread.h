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
#ifndef __EL_WIN_THREAD_HEADER_H__
#define __EL_WIN_THREAD_HEADER_H__

#ifndef _MT
# error "requires /MTd, /MT, /MDd or /MD compiler options"
#endif


namespace el {

class Thread : private NonCopyable {
  HANDLE start_event_;
  HANDLE thread_;
  void (*routine_)(void*);
  void* argument_;
public:
  explicit Thread(void (*routine)(void*) = NULL, void* argument = NULL)
    : start_event_(NULL)
    , thread_(NULL)
    , routine_(routine)
    , argument_(argument)
  {
  }

  ~Thread(void)
  {
    Join();
  }

  inline void Start(void)
  {
    start_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == start_event_)
      abort();

    thread_ = (HANDLE)_beginthreadex(NULL, 
        0, &Thread::Routine, this, 0, NULL);
    if (NULL != thread_)
      WaitForSingleObject(start_event_, INFINITE);

    CloseHandle(start_event_);
  }

  inline void Join(void)
  {
    if (NULL != thread_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = NULL;
    }
  }
private:
  static UINT WINAPI Routine(void* arg)
  {
    Thread* thread = static_cast<Thread*>(arg);
    if (NULL == thread)
      return 0;

    SetEvent(thread->start_event_);
    if (NULL != thread->routine_)
      thread->routine_(thread->argument_);

    return 0;
  }
};

}

#endif  //! __EL_WIN_THREAD_HEADER_H__
