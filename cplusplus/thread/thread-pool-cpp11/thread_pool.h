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
#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool final {
  using TaskFunction = std::function<void (void)>;
  using ThreadEntity = std::unique_ptr<std::thread>;

  std::size_t tasks_capacity_{};
  bool running_{};
  mutable std::mutex mtx_;
  std::condition_variable non_empty_;
  std::condition_variable non_full_;
  std::string name_;
  TaskFunction initializer_fn_{};
  std::vector<ThreadEntity> threads_;
  std::deque<TaskFunction> tasks_;

  bool is_full(void) const;
  TaskFunction fetch_task(void);
  void run_thread_callback(void);

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
public:
  explicit ThreadPool(const std::string& name = "ThreadPool");
  ~ThreadPool(void);

  std::size_t get_tasks_count(void) const;
  void start(int nthreads);
  void stop(void);
  void run(const TaskFunction& fn);
  void run(TaskFunction&& fn);

  void set_thread_initializer(const TaskFunction& fn) {
    initializer_fn_ = fn;
  }

  void set_thread_initializer(TaskFunction&& fn) {
    initializer_fn_ = std::move(fn);
  }

  const std::string& get_name(void) const {
    return name_;
  }

  void set_tasks_capacity(std::size_t capacity) {
    tasks_capacity_ = capacity;
  }
};

#endif // THREADPOOL_H_
