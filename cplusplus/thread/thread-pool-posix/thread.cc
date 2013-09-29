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
#include "tools.h"
#include "thread.h"




thread_t::thread_t(void (*routine)(void*), void* arg)
  : thread_id_(0)
  , routine_(routine)
  , argument_(arg)
{
}

thread_t::~thread_t(void)
{
  join();
}

void 
thread_t::start(void)
{
  pthread_call("create thread", 
      pthread_create(&thread_id_, NULL, &thread_t::s_routine, this));
}

void 
thread_t::join(void)
{
  if (0 != thread_id_) {
    pthread_call("thread join", pthread_join(thread_id_, NULL));
    thread_id_ = 0;
  }
}


void* 
thread_t::s_routine(void* arg)
{
  thread_t* thread = static_cast<thread_t*>(arg);
  if (NULL == thread)
    abort();

  if (NULL != thread->routine_)
    thread->routine_(thread->argument_);

  return NULL;
}
