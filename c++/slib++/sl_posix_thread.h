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
#ifndef __SL_POSIX_THREAD_HEADER_H__
#define __SL_POSIX_THREAD_HEADER_H__

#include <pthread.h>
#include <assert.h>
#include "sl_noncopyable.h"


namespace sl {

class thread_t : noncopyable {
  pthread_t thread_id_;
  void (*routine_)(void*);
  void* argument_;
public:
  thread_t(void (*routine)(void*) = NULL, void* arg = NULL)
    : thread_id_(0)
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
    int ret = pthread_create(&thread_id_, NULL, &thread_t::s_routine, this);
    assert(0 == ret);
  }

  void 
  join(void)
  {
    if (0 != thread_id_) {
      int ret = pthread_join(thread_id_, NULL);
      assert(0 == ret);

      thread_id_ = 0;
    }
  }
private:
  static void* 
  s_routine(void* arg)
  {
    thread_t* self = static_cast<thread_t*>(arg);
    assert(NULL != self);

    if (NULL != self->routine_)
      self->routine_(self->argument_);

    return NULL;
  }
};

}

#endif  //! __SL_POSIX_THREAD_HEADER_H__
