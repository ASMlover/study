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
#ifndef __SL_WIN_THREAD_HEADER_H__
#define __SL_WIN_THREAD_HEADER_H__ 

#if !defined(_MT)
  #error "requires /MTd, /MT, /MDd or /MD compile options"
#endif

#include <windows.h>
#include <process.h>
#include <assert.h>
#include "sl_noncopyable.h"

namespace sl {

class thread_t : noncopyable {
  HANDLE thread_;
  HANDLE create_event_;
  void (*routine_)(void*);
  void* argument_;
public:
  thread_t(void (*routine)(void*) = NULL, void* arg = NULL)
    : thread_(NULL)
    , create_event_(NULL)
    , routine_(routine)
    , argument_(arg)
  {
  }

  ~thread_t(void)
  {
    join();
  }

  void 
  start(void)
  {
    create_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    assert(NULL != create_event_);

    thread_ = (HANDLE)_beginthreadex(NULL, 
        0, &thread_t::s_routine, this, 0, NULL);
    
    if (NULL != thread_)
      WaitForSingleObject(create_event_, INFINITE);
    CloseHandle(create_event_);
  }

  void 
  join(void)
  {
    if (NULL != thread_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = NULL;
    }
  }
private:
  static UINT WINAPI 
  s_routine(void* arg)
  {
    thread_t* self = static_cast<thread_t*>(arg);
    assert(NULL != self);

    SetEvent(self->create_event_);

    if (NULL != self->routine_)
      self->routine_(self->argument_);

    return 0;
  }
};

}

#endif  //! __SL_WIN_THREAD_HEADER_H__
