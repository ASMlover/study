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
#include "TIgnoreSigPipe.h"
#include "TSocketSupport.h"
#include "TChannel.h"
#include "TPoller.h"
#include "TTimerQueue.h"
#include "TKernWrapper.h"
#include "TEventLoop.h"

namespace tyr { namespace net {

#if defined(TYR_WINDOWS)
__declspec(thread) EventLoop* t_loop_this_thread = nullptr;
#else
__thread EventLoop* t_loop_this_thread = nullptr;
#endif
const int kPollMicroSecond = 10000;

IgnoreSigPipe g_ignore_sigpipe;

void EventLoop::abort_not_in_loopthread(void) {
  TYRLOG_SYSFATAL << "EventLoop::abort_not_in_loopthread - EventLoop " << this
    << " was created in thread: " << tid_
    << ", current thread id: " << basic::CurrentThread::tid();
}

void EventLoop::handle_read(void) {
  // for waked up
  uint64_t one = 1;
  int n = Kern::read_eventfd(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one))
    TYRLOG_ERROR << "EventLoop::handle_read() - reads " << n << " bytes instead of 8";
}

void EventLoop::do_pending_functors(void) {
  std::vector<FunctorCallback> functors;
  calling_pending_functors_ = true;

  {
    basic::MutexGuard guard(mtx_);
    functors.swap(pending_functors_);
  }

  for (auto& fn : functors)
    fn();
  calling_pending_functors_ = false;
}

// ================== PUBLIC ==================
EventLoop::EventLoop(void)
  : tid_(basic::CurrentThread::tid())
  , poller_(new Poller(this))
  , timer_queue_(new TimerQueue(this))
  , wakeup_fd_(Kern::create_eventfd())
  , wakeup_channel_(new Channel(this, wakeup_fd_)) {
  TYRLOG_TRACE << "EventLoop created " << this << " in thread " << tid_;
  if (nullptr != t_loop_this_thread)
    TYRLOG_SYSFATAL << "Another EventLoop " << t_loop_this_thread << " exists in this thread " << tid_;
  else
    t_loop_this_thread = this;

  wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
  wakeup_channel_->enabled_reading();
}

EventLoop::~EventLoop(void) {
  assert(!looping_);
  Kern::close_eventfd(wakeup_fd_);
  t_loop_this_thread = nullptr;
}

void EventLoop::loop(void) {
  assert(!looping_);
  assert_in_loopthread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->poll(kPollMicroSecond, &active_channels_);
    for (auto channel : active_channels_)
      channel->handle_event(poll_return_time_);

    do_pending_functors();
  }

  TYRLOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit(void) {
  quit_ = true;
  if (!in_loopthread())
    wakeup();
}

void EventLoop::update_channel(Channel* channel) {
  assert(channel->get_owner_loop() == this);
  assert_in_loopthread();
  poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel* channel) {
  assert(channel->get_owner_loop() == this);
  assert_in_loopthread();
  poller_->remove_channel(channel);
}

TimerID EventLoop::run_at(basic::Timestamp time, const TimerCallback& fn) {
  return timer_queue_->add_timer(fn, time, 0.0);
}

TimerID EventLoop::run_after(double delay, const TimerCallback& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), delay));
  return run_at(time, fn);
}

TimerID EventLoop::run_every(double interval, const TimerCallback& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), interval));
  return timer_queue_->add_timer(fn, time, interval);
}

void EventLoop::wakeup(void) {
  uint64_t one = 1;
  int n = Kern::write_eventfd(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one))
    TYRLOG_ERROR << "EventLoop::wakeup() - writes " << n << " bytes instead of 8";
}

void EventLoop::run_in_loop(const FunctorCallback& cb) {
  if (in_loopthread())
    cb();
  else
    put_in_loop(cb);
}

void EventLoop::put_in_loop(const FunctorCallback& cb) {
  {
    basic::MutexGuard guard(mtx_);
    pending_functors_.push_back(cb);
  }

  if (!in_loopthread() || calling_pending_functors_)
    wakeup();
}

}}
