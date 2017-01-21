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
#ifndef NEPTUNE_EVENTLOOPTHREAD_H
#define NEPTUNE_EVENTLOOPTHREAD_H

#include <functional>
#include <string>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Condition.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Thread.h>

namespace Neptune {

class EventLoop;

class EventLoopThread : private Chaos::UnCopyable {
  using InitiateFunction = std::function<void (EventLoop*)>;

  EventLoop* loop_{};
  bool exiting_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition cond_;
  InitiateFunction initiate_fn_{};
  Chaos::Thread thread_;

  void thread_callback(void);
public:
  EventLoopThread(const InitiateFunction& fn, const std::string& name = std::string());
  EventLoopThread(InitiateFunction&& fn, const std::string& name = std::string());
  ~EventLoopThread(void);
  EventLoop* start_loop(void);
};

}

#endif // NEPTUNE_EVENTLOOPTHREAD_H
