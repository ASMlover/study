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
#ifndef __TYR_NET_EVENTLOOPTHREAD_HEADER_H__
#define __TYR_NET_EVENTLOOPTHREAD_HEADER_H__

#include <functional>
#include <string>
#include "../basic/TUnCopyable.h"
#include "../basic/TMutex.h"
#include "../basic/TCondition.h"
#include "../basic/TThread.h"

namespace tyr { namespace net {

class EventLoop;

class EventLoopThread : private basic::UnCopyable {
  using ThreadInitCallback = std::function<void (EventLoop*)>;

  EventLoop* loop_{};
  bool exiting_{};
  basic::Thread thread_;
  mutable basic::Mutex mtx_;
  basic::Condition cond_;
  ThreadInitCallback thread_init_fn_{};

  void thread_callback(void);
public:
  EventLoopThread(const ThreadInitCallback& fn = ThreadInitCallback(), const std::string& name = std::string());
  ~EventLoopThread(void);
  EventLoop* start_loop(void);
};

}}

#endif // __TYR_NET_EVENTLOOPTHREAD_HEADER_H__
