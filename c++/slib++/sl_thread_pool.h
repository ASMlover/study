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
#ifndef __SL_THREAD_POOL_HEADER_H__
#define __SL_THREAD_POOL_HEADER_H__

#include <stdlib.h>
#include "sl_array.h"
#include "sl_queue.h"
#include "sl_mutex.h"
#include "sl_condition.h"
#include "sl_thread.h"

namespace sl {

struct task_t {
  void (*routine_)(void*);
  void* argument_;

  task_t(void (*routine)(void*) = NULL, void* arg = NULL)
    : routine_(routine)
    , argument_(arg)
  {
  }
};


class thread_pool_t : noncopyable {
  enum {
    DEF_THREADS_MIN = 8, 
    DEF_THREADS_MAX = 256, 
  };

  bool running_;
  mutex_t mutex_;
  condition_t cond_;
  array_t<thread_t*> threads_;
  queue_t<task_t> tasks_;
public:
  thread_pool_t(void);
  ~thread_pool_t(void);

  void start(int threads_count = DEF_THREADS_MIN);
  void stop(void);
  void run(void (*routine)(void*) = NULL, void* arg = NULL);
private:
  task_t take(void);
  static void s_routine(void* arg);
};

}

#endif  //! __SL_THREAD_POOL_HEADER_H__
