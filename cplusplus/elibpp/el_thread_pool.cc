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
#include "elib_internal.h"
#include "el_thread_pool.h"



namespace el {

ThreadPool::ThreadPool(void)
  : mutex_()
  , cond_(mutex_)
  , running_(false)
{
}

ThreadPool::~ThreadPool(void)
{
  if (running_)
    Stop();
}

void 
ThreadPool::Start(int thread_count)
{
  if (thread_count < kDefMinThreadsCount)
    thread_count = kDefMinThreadsCount;
  if (thread_count > kDefMaxThreadsCount)
    thread_count = kDefMaxThreadsCount;

  running_ = true;
  for (int i = 0; i < thread_count; ++i) {
    threads_.push_back(new Thread(&ThreadPool::Routine, this));
    threads_[i]->Start();
  }
}

void 
ThreadPool::Stop(void)
{
  {
    MutexGuard lock(mutex_);
    running_ = false;
    cond_.SignalAll();
  }

  std::vector<Thread*>::iterator it;
  for (it = threads_.begin(); it != threads_.end(); ++it) {
    (*it)->Join();
    delete (*it);
  }
  threads_.clear();
}

void 
ThreadPool::Run(void (*routine)(void*), void* argument)
{
  if (threads_.empty()) {
    routine(argument);
  }
  else {
    MutexGuard lock(mutex_);

    tasks_.push(Task(routine, argument));
    cond_.Signal();
  }
}


Task 
ThreadPool::TakeTask(void)
{
  MutexGuard lock(mutex_);
  while (tasks_.empty() && running_)
    cond_.Wait();

  Task task;
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop();
  }

  return task;
}

void 
ThreadPool::Routine(void* argument)
{
  ThreadPool* self = static_cast<ThreadPool*>(argument);
  if (NULL == self)
    abort();

  while (self->running_) {
    Task task = self->TakeTask();

    if (NULL != task.routine)
      task.routine(task.argument);
  }
}

}
