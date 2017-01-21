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
#include <Chaos/Types.h>
#include <Neptune/EventLoop.h>
#include <Neptune/EventLoopThread.h>

namespace Neptune {

EventLoopThread::EventLoopThread(const InitiateFunction& fn, const std::string& name)
  : mutex_()
  , cond_(mutex_)
  , initiate_fn_(fn)
  , thread_(std::bind(&EventLoopThread::thread_callback, this), name) {
}

EventLoopThread::EventLoopThread(InitiateFunction&& fn, const std::string& name)
  : mutex_()
  , cond_(mutex_)
  , initiate_fn_(std::move(fn))
  , thread_(std::bind(&EventLoopThread::thread_callback, this), name) {
}

EventLoopThread::~EventLoopThread(void) {
  exiting_ = true;
  if (loop_) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::start_loop(void) {
  CHAOS_CHECK(!thread_.is_started(), "event loop thread should not started");

  thread_.start();
  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    while (!loop_)
      cond_.wait();
  }

  return loop_;
}

void EventLoopThread::thread_callback(void) {
  EventLoop loop;

  if (initiate_fn_)
    initiate_fn_(&loop);

  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }

  loop_->loop();
  loop_ = nullptr;
}

}
