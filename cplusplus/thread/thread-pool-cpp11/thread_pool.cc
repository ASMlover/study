// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <cstdio>
#include <exception>
#include "thread_pool.h"

ThreadPool::ThreadPool(const std::string& name)
  : mtx_()
  , name_(name) {
}

ThreadPool::~ThreadPool(void) {
  if (running_)
    stop();
}

std::size_t ThreadPool::get_tasks_count(void) const {
  std::unique_lock<std::mutex> guard(mtx_);
  return tasks_.size();
}

void ThreadPool::start(int nthreads) {
  running_ = true;
  threads_.reserve(nthreads);
  for (int i = 0; i < nthreads; ++i)
    threads_.emplace_back(new std::thread(std::bind(&ThreadPool::run_thread_callback, this)));

  if (0 == nthreads && initializer_fn_)
    initializer_fn_();
}

void ThreadPool::stop(void) {
  {
    std::unique_lock<std::mutex> guard(mtx_);
    running_ = false;
    non_empty_.notify_all();
  }

  for (auto& thread : threads_)
    thread->join();
}

void ThreadPool::run(const TaskFunction& fn) {
  if (threads_.empty()) {
    fn();
  }
  else {
    std::unique_lock<std::mutex> guard(mtx_);
    while (is_full())
      non_full_.wait(guard);

    tasks_.push_back(fn);
    non_empty_.notify_one();
  }
}

void ThreadPool::run(TaskFunction&& fn) {
  if (threads_.empty()) {
    fn();
  }
  else {
    std::unique_lock<std::mutex> guard(mtx_);
    while (is_full())
      non_full_.wait(guard);

    tasks_.push_back(std::move(fn));
    non_empty_.notify_one();
  }
}

bool ThreadPool::is_full(void) const {
  return tasks_capacity_ > 0 && tasks_.size() >= tasks_capacity_;
}

ThreadPool::TaskFunction ThreadPool::fetch_task(void) {
  std::unique_lock<std::mutex> guard(mtx_);
  while (tasks_.empty() && running_)
    non_empty_.wait(guard);

  TaskFunction task_fn;
  if (!tasks_.empty()) {
    task_fn = tasks_.front();
    tasks_.pop_front();
    if (tasks_capacity_ > 0)
      non_full_.notify_one();
  }

  return task_fn;
}

void ThreadPool::run_thread_callback(void) {
  try {
    if (initializer_fn_)
      initializer_fn_();

    while (running_) {
      TaskFunction task_fn(fetch_task());
      if (task_fn)
        task_fn();
    }
  }
  catch (const std::exception& ex) {
    std::fprintf(stderr,
        "exception caught in ThreadPool %s\n"
        "reason: %s\n",
        name_.c_str(),
        ex.what());
    std::abort();
  }
  catch (...) {
    std::fprintf(stderr,
        "unknown exception caught in ThreadPool %s\n",
        name_.c_str());
    throw;
  }
}
