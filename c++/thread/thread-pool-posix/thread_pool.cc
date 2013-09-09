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
#include "thread.h"
#include "thread_pool.h"


thread_pool_t::thread_pool_t(void)
  : running_(false)
  , mutex_()
  , cond_(mutex_)
  , threads_count_(0)
{
}

thread_pool_t::~thread_pool_t(void)
{
  if (running_)
    stop();
}

void 
thread_pool_t::start(int threads_count)
{
  if (threads_count < DEF_THREADS_MIN)
    threads_count_ = DEF_THREADS_MIN;
  else if (threads_count > DEF_THREADS_MAX)
    threads_count_ = DEF_THREADS_MAX;
  else 
    threads_count_ = threads_count;

  for (int i = 0; i < threads_count_; ++i) {
    threads_.push_back(new thread_t(&thread_pool_t::s_routine, this));
    threads_[i]->start();
  }
}

void 
thread_pool_t::stop(void)
{
  {
    mutex_guard_t lock(mutex_);
    running_ = false;
    cond_.siganl_all();
  }

  for (int i = 0; i < threads_count_; ++i) {
    threads_[i]->join();
    delete threads_[i];
  }
  threads_.clear();
}

void 
thread_pool_t::run(void (*routine)(void*), void* arg)
{
  if (threads_.empty())
    routine(arg);
  else {
    mutex_guard_t lock(mutex_);

    tasks_.push(task_t(routine, arg));
    cond_.signal();
  }
}



task_t 
thread_pool_t::take(void)
{
  mutex_guard_t lock(mutex_);
  while (tasks_.empty() && running_)
    cond_.wait();

  task_t task;
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

  while (self->running_) {
    task_t task = self->take();

    if (NULL != task.routine_)
      task.routine_(task.argument_);
  }
}
