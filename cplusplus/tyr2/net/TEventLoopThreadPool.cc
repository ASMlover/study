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
#include <assert.h>
#include <functional>
#include "TEventLoop.h"
#include "TEventLoopThread.h"
#include "TEventLoopThreadPool.h"

namespace tyr { namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop)
  : base_loop_(base_loop) {
}

EventLoopThreadPool::~EventLoopThreadPool(void) {
}

void EventLoopThreadPool::start(void) {
  assert(!started_);
  base_loop_->assert_in_loopthread();

  started_ = true;
  for (int i = 0; i < thread_count_; ++i) {
    EventLoopThread* t = new EventLoopThread();
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->start_loop());
  }
}

EventLoop* EventLoopThreadPool::get_next_loop(void) {
  base_loop_->assert_in_loopthread();
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    loop = loops_[next_thread_];
    ++next_thread_;
    if (static_cast<size_t>(next_thread_) >= loops_.size())
      next_thread_ = 0;
  }

  return loop;
}

}}
