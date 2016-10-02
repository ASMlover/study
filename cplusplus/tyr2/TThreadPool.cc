// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <assert.h>
#include <stdio.h>
#include "TException.h"
#include "TThreadPool.h"

namespace tyr {

ThreadPool::ThreadPool(const std::string& name)
  : mtx_()
  , notify_empty_(mtx_)
  , notify_full_(mtx_)
  , name_(name)
  , max_tasksz_(0)
  , running_(false) {
}

size_t ThreadPool::task_size(void) const {
  MutexGuard guard(mtx_);
  return tasks_.size();
}

void ThreadPool::start(int thrd_num) {
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(thrd_num);
  for (int i = 0; i < thrd_num; ++i) {
    char id[32];
    snprintf(id, sizeof(id), "%d", i + 1);
    threads_.emplace_back(new Thread(std::bind(&ThreadPool::run_in_thread, this), name_ + id));
    threads_[i]->start();
  }

  if (0 == thrd_num && thrd_init_cb_)
    thrd_init_cb_();
}

void ThreadPool::stop(void) {
  {
    MutexGuard guard(mtx_);
    running_ = false;
    notify_empty_.notify_all()
  }

  for (auto& thrd : threads_)
    thrd->join();
}

void ThreadPool::run(TaskCallback&& cb) {
  if (threads_.empty()) {
    cb();
  }
  else {
    MutexGuard guard(mtx_);
    while (is_full())
      notify_full_.wait();
    assert(!is_full());

    tasks_.push_back(std::move(cb));
    notify_empty_.notify();
  }
}

bool ThreadPool::is_full(void) const {
  mtx_.assert_locked();
  return max_tasksz_ > 0 && tasks_.size() >= max_tasksz_;
}

void ThreadPool::run_in_thread(void) {
  try {
    if (thrd_init_cb_)
      thrd_init_cb_();

    while (running_) {
      TaskCallback task(take_task());
      if (task)
        task();
    }
  }
  catch (const Exception& ex) {
    fprintf(stderr,
        "exception caught in ThreadPool %s\n"
        "reason: %s\n"
        "stack trace: %s\n",
        name_.c_str(),
        ex.what(),
        ex.stack_trace());
    abort();
  }
  catch (const std::exception& ex) {
    fprintf(stderr,
        "exception caught in ThreadPool %s\n"
        "reason: %s\n",
        name_.c_str(),
        ex.what());
    abort();
  }
  catch (...) {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw;
  }
}

TaskCallback ThreadPool::take_task(void) {
  MutexGuard guard(mtx_);
  while (tasks_.empty() && running_)
    notify_empty_.wait();

  TaskCallback task;
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop_front();
    if (max_tasksz_ > 0)
      notify_full_.notify();
  }

  return task;
}

}
