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
#ifndef CHAOS_CONCURRENT_THREADPOOL_H
#define CHAOS_CONCURRENT_THREADPOOL_H

#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <vector>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>

namespace Chaos {

typedef std::function<void (void)> TaskCallback;

class Thread;

class ThreadPool : private UnCopyable {
  typedef std::unique_ptr<Thread> ThreadEntity;

  size_t tasks_capacity_{};
  bool running_{};
  mutable Mutex mtx_;
  Condition non_empty_;
  Condition non_full_;
  std::string name_;
  TaskCallback thread_init_fn_;
  std::vector<ThreadEntity> threads_;
  std::deque<TaskCallback> tasks_;
private:
  bool is_full(void) const;
  TaskCallback fetch_task(void);
  void run_thread_callback(void);
public:
  explicit ThreadPool(const std::string& name = "ThreadPool");
  ~ThreadPool(void);

  void set_thread_initializer(const TaskCallback& fn) {
    thread_init_fn_ = fn;
  }

  void set_thread_initializer(TaskCallback&& fn) {
    thread_init_fn_ = std::move(fn);
  }

  const std::string& get_name(void) const {
    return name_;
  }

  void set_tasks_capacity(size_t capacity) {
    // should be called before start
    tasks_capacity_ = capacity;
  }

  size_t get_tasks_count(void) const;
  void start(int threads_num);
  void stop(void);
  void run(const TaskCallback& fn);
  void run(TaskCallback&& fn);
};

}

#endif // CHAOS_CONCURRENT_THREADPOOL_H
