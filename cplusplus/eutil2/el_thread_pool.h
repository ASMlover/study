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
#ifndef __EL_THREAD_POOL_HEADER_H__
#define __EL_THREAD_POOL_HEADER_H__

namespace el {

struct Worker {
  RoutinerType routine;
  void*        argument;

  Worker(const RoutinerType& worker = nullptr, void* arg = nullptr) 
    : routine(worker)
    , argument(arg) {
  }

  void Run(void) {
    if (nullptr != routine) 
      routine(argument);
  }
};

class ThreadPool : private UnCopyable {
  enum {MIN_THREADS = 8, MAX_THREADS = 64};
  typedef std::shared_ptr<Thread> ThreadPtr;

  bool      running_;
  Mutex     mutex_;
  Condition cond_;

  std::vector<ThreadPtr> threads_;
  std::queue<Worker>     workers_;
public:
  ThreadPool(void);
  ~ThreadPool(void);

  void Start(uint32_t thread_count = MIN_THREADS);
  void Stop(void);
  void Run(const RoutinerType& routine, void* argument = nullptr);
private:
  Worker GetWorker(void);
  void Routine(void* argument);
};

}

#endif  // __EL_THREAD_POOL_HEADER_H__
