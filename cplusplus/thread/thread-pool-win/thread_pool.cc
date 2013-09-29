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
#include <assert.h>
#include <windows.h>
#include "mutex.h"
#include "thread.h"
#include "guard.h"
#include "thread_pool.h"



thread_pool_t::thread_pool_t(void)
  : quit_event_(NULL)
{
}

thread_pool_t::~thread_pool_t(void)
{
  if (NULL != quit_event_)
    stop();
}

void 
thread_pool_t::start(int thread_num)
{
  if (thread_num < DEF_THREADS_MIN)
    thread_num = DEF_THREADS_MIN;
  if (thread_num > DEF_THREADS_MAX)
    thread_num = DEF_THREADS_MAX;

  quit_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
  assert(NULL != quit_event_);
  for (int i = 0; i < thread_num; ++i) {
    threads_.push_back(new thread_t(&thread_pool_t::s_routine, this));
    threads_[i]->start();
  }
}

void
thread_pool_t::stop(void)
{
  SetEvent(quit_event_);
  
  int thread_num = (int)threads_.size();
  for (int i = 0; i < thread_num; ++i) {
    threads_[i]->join();
    delete threads_[i];
  }
  threads_.clear();

  CloseHandle(quit_event_);
  quit_event_ = NULL;
}

void 
thread_pool_t::run_task(void (*routine)(void*), void* arg)
{
  if (threads_.empty())
    routine(arg);
  else {
    guard_t lock(mutex_);
    tasks_.push(task_t(routine, arg));
  }
}


task_t 
thread_pool_t::pick(void)
{
  task_t task;

  guard_t lock(mutex_);
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop();
  }

  return task;
}

void 
thread_pool_t::s_routine(void* arg)
{
  thread_pool_t* self = static_cast<thread_pool_t*>(arg);
  assert(NULL != self);

  DWORD tid = GetCurrentThreadId();
  fprintf(stdout, "[%d] running ...\n", tid);
  while (true) {
    if (WAIT_OBJECT_0 == WaitForSingleObject(self->quit_event_, 5))
      break;

    task_t task = self->pick();

    if (NULL != task.routine_)
      task.routine_(task.arg_);
  }
  fprintf(stdout, "[%d] exiting ...\n", tid);
}
