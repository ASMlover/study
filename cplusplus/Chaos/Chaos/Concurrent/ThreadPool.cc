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
#include <stdio.h>
#include <stdlib.h>
#include <Chaos/Except/Exception.h>
#include <Chaos/IO/ColorIO.h>
#include <Chaos/Concurrent/Thread.h>
#include <Chaos/Concurrent/ThreadPool.h>

namespace Chaos {

bool ThreadPool::is_full(void) const {
  mtx_.assert_locked();
  return tasks_capacity_ > 0 && tasks_.size() >= tasks_capacity_;
}

TaskCallback ThreadPool::fetch_task(void) {
  ScopedLock<Mutex> guard(mtx_);
  while (tasks_.empty() && running_)
    non_empty_.wait();

  TaskCallback task;
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop_front();
    if (tasks_capacity_ > 0)
      non_full_.notify_one();
  }

  return task;
}

void ThreadPool::run_thread_callback(void) {
  try {
    if (thread_init_fn_)
      thread_init_fn_();

    while (running_) {
      TaskCallback task(fetch_task());
      if (task)
        task();
    }
  }
  catch (const Exception& ex) {
    ColorIO::fprintf(stderr,
        ColorIO::ColorType::COLORTYPE_RED,
        "exception caught in ThreadPool %s\n"
        "reason: %s\n"
        "stack traceback: %s\n",
        name_.c_str(),
        ex.what(),
        ex.get_traceback());
    abort();
  }
  catch (const std::exception& ex) {
    ColorIO::fprintf(stderr,
        ColorIO::ColorType::COLORTYPE_RED,
        "exception caught in ThreadPool %s\n"
        "reason: %s\n",
        name_.c_str(),
        ex.what());
    abort();
  }
  catch (...) {
    ColorIO::fprintf(stderr,
        ColorIO::ColorType::COLORTYPE_RED,
        "unknown exception caught in ThreadPool %s\n",
        name_.c_str());
    throw;
  }
}

ThreadPool::ThreadPool(const std::string& name)
  : mtx_()
  , non_empty_(mtx_)
  , non_full_(mtx_)
  , name_(name) {
}

ThreadPool::~ThreadPool(void) {
  if (running_)
    stop();
}

size_t ThreadPool::get_tasks_count(void) const {
  ScopedLock<Mutex> guard(mtx_);
  return tasks_.size();
}

void ThreadPool::start(int threads_num) {
  CHAOS_CHECK(threads_.empty(), "ThreadPool::start - threads queue should be empty");

  running_ = true;
  threads_.reserve(threads_num);
  for (int i = 0; i < threads_num; ++i) {
    char thread_id[32];
    snprintf(thread_id, sizeof(thread_id), "%d", i + 1);
    threads_.emplace_back(new Thread(std::bind(&ThreadPool::run_thread_callback, this), name_ + thread_id));
    threads_[i]->start();
  }

  if (0 == threads_num && thread_init_fn_)
    thread_init_fn_();
}

void ThreadPool::stop(void) {
  {
    ScopedLock<Mutex> guard(mtx_);
    running_ = false;
    non_empty_.notify_all();
  }

  for (auto& thread : threads_)
    thread->join();
}

void ThreadPool::run(const TaskCallback& fn) {
  if (threads_.empty()) {
    fn();
  }
  else {
    ScopedLock<Mutex> guard(mtx_);
    while (is_full())
      non_full_.wait();
    CHAOS_CHECK(!is_full(), "Thread::run(&) - tasks should not be full");

    tasks_.push_back(fn);
    non_empty_.notify_one();
  }
}

void ThreadPool::run(TaskCallback&& fn) {
  if (threads_.empty()) {
    fn();
  }
  else {
    ScopedLock<Mutex> guard(mtx_);
    while (is_full())
      non_full_.wait();
    CHAOS_CHECK(!is_full(), "Thread::run(&&) - tasks should not be full");

    tasks_.push_back(std::move(fn));
    non_empty_.notify_one();
  }
}

}
