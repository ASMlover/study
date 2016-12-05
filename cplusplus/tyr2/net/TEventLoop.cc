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
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TEventLoop.h"

namespace tyr { namespace net {

#if defined(TYR_WINDOWS)
__declspec(thread) EventLoop* t_loop_this_thread = nullptr;
#else
__thread EventLoop* t_loop_this_thread = nullptr;
#endif

void EventLoop::abort_not_in_loopthread(void) {
  TL_SYSFATAL << "EventLoop::abort_not_in_loopthread - EventLoop " << this
    << " was created in thread: " << tid_
    << ", current thread id: " << basic::CurrentThread::tid();
}

EventLoop::EventLoop(void)
  : tid_(basic::CurrentThread::tid()) {
  TL_TRACE << "EventLoop created " << this << " in thread " << tid_;
  if (nullptr != t_loop_this_thread)
    TL_SYSFATAL << "Another EventLoop " << t_loop_this_thread << " exists in this thread " << tid_;
  else
    t_loop_this_thread = this;
}

EventLoop::~EventLoop(void) {
  assert(!looping_);
  t_loop_this_thread = nullptr;
}

void EventLoop::loop(void) {
  assert(!looping_);
  assert_in_loopthread();
  looping_ = true;

  SocketSupport::kern_poll(nullptr, 0, 5 * 1000);

  TL_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

}}
