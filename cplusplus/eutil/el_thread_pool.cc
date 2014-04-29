// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include "el_util.h"
#include "el_thread_pool.h"




namespace el {

ThreadPool::ThreadPool(void) 
  : running_(false)
  , locker_()
  , cond_(locker_) {
}

ThreadPool::~ThreadPool(void) {
  if (running_)
    Stop();
}

void ThreadPool::Start(int thread_count) {
  thread_count = MAX(MIN_THREADS, thread_count);
  thread_count = MIN(MAX_THREADS, thread_count);

  running_ = true;
  for (int i = 0; i < thread_count; ++i) {
    threads_.push_back(new Thread());
    threads_[i]->Create(EL_THREAD_CALLBACK(ThreadPool::Routine, this));
  }
}

void ThreadPool::Stop(void) {
  {
    LockerGuard<Mutex> guard(locker_);
    running_ = false;
    cond_.SignalAll();
  }

  for (const auto& trd : threads_) {
    trd->Join();
    delete trd;
  }
  threads_.clear();
}

void ThreadPool::Run(const RoutinerType& routine, void* argument) {
  if (threads_.empty()) {
    routine(argument);
  }
  else {
    LockerGuard<Mutex> guard(locker_);

    tasks_.push(Task(routine, argument));
    cond_.Signal();
  }
}

Task ThreadPool::TakeTask(void) {
  LockerGuard<Mutex> guard(locker_);
  while (tasks_.empty() && running_) 
    cond_.Wait();

  Task task;
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop();
  }

  return task;
}

void ThreadPool::Routine(void* argument) {
  while (running_) {
    Task task = TakeTask();

    if (nullptr != task.routine)
      task.routine(task.argument);
  }
}

}
