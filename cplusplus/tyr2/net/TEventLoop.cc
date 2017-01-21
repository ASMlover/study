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
#include <algorithm>
#include "../basic/TLogging.h"
#include "TIgnoreSigPipe.h"
#include "TSocketSupport.h"
#include "TChannel.h"
#include "TPoller.h"
#include "TTimerQueue.h"
#include "TKernWrapper.h"
#include "TWakeupSignaler.h"
#include "TEventLoop.h"

namespace tyr { namespace net {

#if defined(TYR_WINDOWS)
__declspec(thread) EventLoop* t_loop_this_thread = nullptr;
#else
__thread EventLoop* t_loop_this_thread = nullptr;
#endif
const int kPollMicroSecond = 10000;

IgnoreSigPipe g_ignore_sigpipe;

EventLoop::EventLoop(void)
  : tid_(basic::CurrentThread::tid())
  , poller_(Poller::new_default_poller(this))
  , timer_queue_(new TimerQueue(this))
  , wakeup_(new WakeupSignaler())
  , wakeup_channel_(new Channel(this, wakeup_->get_reader_fd())) {
  TYRLOG_DEBUG << "EventLoop::EventLoop - created " << this << " in thread " << tid_;
  if (nullptr != t_loop_this_thread) {
    TYRLOG_SYSFATAL << "EventLoop::EventLoop - another EventLoop "
      << t_loop_this_thread << " exists in this thread " << tid_;
  }
  else {
    t_loop_this_thread = this;
  }

  wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
  wakeup_channel_->enabled_reading();

  UNUSED(init_);
}

EventLoop::~EventLoop(void) {
  TYRLOG_DEBUG << "EventLoop::~EventLoop - " << this << " of thread " << tid_
    << " destructs in thread " << basic::CurrentThread::tid();
  wakeup_channel_->disabled_all();
  wakeup_channel_->remove();
  t_loop_this_thread = nullptr;
}

void EventLoop::loop(void) {
  assert(!looping_);
  assert_in_loopthread();
  looping_ = true;
  quit_ = false;
  TYRLOG_TRACE << "EventLoop::loop - " << this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->poll(kPollMicroSecond, &active_channels_);
    ++iteration_;
    if (basic::Logger::log_level() <= basic::LoggingLevel::LOGGINGLEVEL_TRACE)
      debug_active_channels();

    event_handling_ = true;
    for (auto channel : active_channels_) {
      current_active_channel_ = channel;
      current_active_channel_->handle_event(poll_return_time_);
    }
    current_active_channel_ = nullptr;
    event_handling_ = false;

    do_pending_functors();
#if !defined(TYR_LINUX)
    timer_queue_->poll_timer();
#endif
  }

  TYRLOG_TRACE << "EventLoop::loop - " << this << " stop looping";
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

  if (event_handling_) {
    assert(current_active_channel_ == channel ||
        std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
  }
  poller_->remove_channel(channel);
}

bool EventLoop::has_channel(Channel* channel) {
  assert(channel->get_owner_loop() == this);
  assert_in_loopthread();
  return poller_->has_channel(channel);
}

size_t EventLoop::get_functor_count(void) const {
  basic::MutexGuard guard(mtx_);
  return pending_functors_.size();
}

TimerID EventLoop::run_at(basic::Timestamp time, const TimerCallback& fn) {
  return timer_queue_->add_timer(fn, time, 0.0);
}

TimerID EventLoop::run_at(basic::Timestamp time, TimerCallback&& fn) {
  return timer_queue_->add_timer(std::move(fn), time, 0.0);
}

TimerID EventLoop::run_after(double delay, const TimerCallback& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), delay));
  return run_at(time, fn);
}

TimerID EventLoop::run_after(double delay, TimerCallback&& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), delay));
  return run_at(time, std::move(fn));
}

TimerID EventLoop::run_every(double interval, const TimerCallback& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), interval));
  return timer_queue_->add_timer(fn, time, interval);
}

TimerID EventLoop::run_every(double interval, TimerCallback&& fn) {
  basic::Timestamp time(basic::add_time(basic::Timestamp::now(), interval));
  return timer_queue_->add_timer(std::move(fn), time, interval);
}

void EventLoop::wakeup(void) {
  uint64_t one = 1;
  int n = wakeup_->set_signal(&one, sizeof(one));
  if (n != sizeof(one))
    TYRLOG_ERROR << "EventLoop::wakeup - writes " << n << " bytes instead of 8";
}

void EventLoop::cancel(TimerID timerid) {
  timer_queue_->cancel(timerid);
}

void EventLoop::run_in_loop(const FunctorCallback& fn) {
  if (in_loopthread())
    fn();
  else
    put_in_loop(fn);
}

void EventLoop::run_in_loop(FunctorCallback&& fn) {
  if (in_loopthread())
    fn();
  else
    put_in_loop(std::move(fn));
}

void EventLoop::put_in_loop(const FunctorCallback& fn) {
  {
    basic::MutexGuard guard(mtx_);
    pending_functors_.push_back(fn);
  }

  if (!in_loopthread() || calling_pending_functors_)
    wakeup();
}

void EventLoop::put_in_loop(FunctorCallback&& fn) {
  {
    basic::MutexGuard guard(mtx_);
    pending_functors_.push_back(std::move(fn));
  }

  if (!in_loopthread() || calling_pending_functors_)
    wakeup();
}

EventLoop* EventLoop::get_loop_of_current_thread(void) {
  return t_loop_this_thread;
}

void EventLoop::abort_not_in_loopthread(void) {
  TYRLOG_SYSFATAL << "EventLoop::abort_not_in_loopthread - EventLoop " << this
    << " was created in thread: " << tid_
    << ", current thread id: " << basic::CurrentThread::tid();
}

void EventLoop::handle_read(void) {
  // for waked up
  uint64_t one = 1;
  int n = wakeup_->get_signal(&one, sizeof(one));
  if (n != sizeof(one))
    TYRLOG_ERROR << "EventLoop::handle_read - reads " << n << " bytes instead of 8";
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

void EventLoop::debug_active_channels(void) const {
  for (auto ch : active_channels_)
    TYRLOG_TRACE << "EventLoop""debug_active_channels - {" << ch->revents_to_string() << "}";
}

}}
