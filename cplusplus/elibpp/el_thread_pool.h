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
#ifndef __EL_THREAD_POOL_HEADER_H__
#define __EL_THREAD_POOL_HEADER_H__

namespace el {

struct Task {
  void (*routine)(void*);
  void* argument;

  Task(void (*worker)(void*) = NULL, void* arg = NULL)
    : routine(worker)
    , argument(arg)
  {
  }
};

class Thread;
class ThreadPool : private NonCopyable {
  enum {
    kDefMinThreadsCount = 8, 
    kDefMaxThreadsCount = 256, 
  };

  Mutex mutex_;
  Condition cond_;
  bool running_;

  std::vector<Thread*> threads_;
  std::queue<Task> tasks_;
public:
  explicit ThreadPool(void);
  ~ThreadPool(void);

  void Start(int thread_count = kDefMinThreadsCount);
  void Stop(void);
  void Run(void (*routine)(void*) = NULL, void* argument = NULL);
private:
  Task TakeTask(void);
  static void Routine(void* argument);
};

}

#endif  //! __EL_THREAD_POOL_HEADER_H__
