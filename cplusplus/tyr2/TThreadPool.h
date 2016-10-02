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
#ifndef __TYR_THREADPOOL_HEADER_H__
#define __TYR_THREADPOOL_HEADER_H__

#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <vector>

#include "TTypes.h"
#include "TMutex.h"
#include "TCondition.h"
#include "TThread.h"

namespace tyr {

typedef std::function<void (void)> TaskCallback;

class ThreadPool : private UnCopyable {
  mutable Mutex mtx_;
  Condition not_empty_;
  Condition not_full_;
  std::string name_;
  TaskCallback thrd_init_cb_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<TaskCallback> tasks_;
  size_t max_tasksz_;
  bool running_;
public:
  explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
  ~ThreadPool(void);

  void set_max_task_size(int max_size) {
    max_tasksz_ = max_size;
  }

  void set_thread_init_callback(TaskCallback&& cb) {
    thrd_init_cb_ = std::move(cb);
  }

  const std::string& name(void) const {
    return name_;
  }

  size_t task_size(void) const;

  void start(int thrd_num);
  void stop(void);
  void run(TaskCallback&& cb);
private:
  bool is_full(void) const;
  void run_in_thread(void);
  TaskCallback take_task(void);
};

}

#endif // __TYR_THREADPOOL_HEADER_H__
