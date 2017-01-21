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
#ifndef NEPTUNE_EVENTLOOPTHREADPOOL_H
#define NEPTUNE_EVENTLOOPTHREADPOOL_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <Chaos/UnCopyable.h>

namespace Neptune {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : private Chaos::UnCopyable {
  using InitiateFunction = std::function<void (EventLoop*)>;

  EventLoop* base_loop_{};
  bool started_{};
  int nthreads_{};
  int next_thread_{};
  std::string name_;
  std::vector<std::unique_ptr<EventLoopThread>> loop_threads_;
  std::vector<EventLoop*> loops_;
public:
  EventLoopThreadPool(EventLoop* base_loop, const std::string& name);
  ~EventLoopThreadPool(void);

  void start(const InitiateFunction& fn = InitiateFunction());
  void start(InitiateFunction&& fn);
  EventLoop* get_next_loop(void);
  EventLoop* get_loop_as_hash(std::size_t hash_code);
  std::vector<EventLoop*> get_all_loops(void);

  void set_nthreads(int nthreads) {
    nthreads_ = nthreads;
  }

  bool is_started(void) const {
    return started_;
  }

  const std::string& get_name(void) const {
    return name_;
  }
};

}

#endif // NEPTUNE_EVENTLOOPTHREADPOOL_H
