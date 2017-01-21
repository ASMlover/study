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
#include <algorithm>
#include <Chaos/Platform.h>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/IgnoreSigPipe.h>
#include <Neptune/Channel.h>
#include <Neptune/Poller.h>
#include <Neptune/TimerQueue.h>
#include <Neptune/WakeupSignaler.h>
#include <Neptune/EventLoop.h>

namespace Neptune {

__chaos_tl EventLoop* t_loopthread = nullptr;
const int kPollMicrosecond = 10000;

Neptune::IgnoreSigPipe g_ignore_sigpipe;

EventLoop::EventLoop(void)
  : tid_(Chaos::CurrentThread::get_tid())
  , poller_(Poller::get_poller(this))
  , timer_queue_(new TimerQueue(this))
  , wakeup_(new WakeupSignaler())
  , wakeup_channel_(new Channel(this, wakeup_->get_reader()))
{
  CHAOSLOG_DEBUG << "EventLoop::EventLoop - created " << this << " in thread " << tid_;
  if (t_loopthread)
    CHAOSLOG_SYSFATAL << "EventLoop::EventLoop - another EventLoop " << t_loopthread << " exists in thread " << tid_;
  else
    t_loopthread = this;

  wakeup_channel_->bind_read_functor(std::bind(&EventLoop::do_handle_read, this));
  wakeup_channel_->enabled_reading();

#if defined(CHAOS_POSIX)
  CHAOS_UNUSED(init_);
#endif
}

EventLoop::~EventLoop(void) {
  CHAOSLOG_DEBUG << "EventLoop::~EventLoop - " << this << " of thread " << tid_
    << " destructs in thread " << Chaos::CurrentThread::get_tid();

  wakeup_channel_->disabled_all();
  wakeup_channel_->remove();
  t_loopthread = nullptr;
}

void EventLoop::loop(void) {
  CHAOS_CHECK(!looping_, "event loop should not running");
  assert_in_loopthread();

  looping_ = true;
  quit_ = false;
  CHAOSLOG_TRACE << "EventLoop::loop - " << this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->poll(kPollMicrosecond, active_channels_);
    ++iteration_;
    if (Chaos::Logger::get_loglevel() <= Chaos::LoggingLevel::LOGGINGLEVEL_TRACE)
      debug_active_channels();

    event_handling_ = true;
    for (auto ch : active_channels_) {
      current_active_channel_ = ch;
      current_active_channel_->handle_event(poll_return_time_);
    }
    current_active_channel_ = nullptr;
    event_handling_ = false;

    do_pending_functors();
#if !defined(CHAOS_LINUX)
    timer_queue_->poll_timer();
#endif
  }
  CHAOSLOG_TRACE << "EventLoop::loop - " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit(void) {
  quit_ = true;
  if (!in_loopthread())
    wakeup();
}

void EventLoop::wakeup(void) {
  std::uint64_t one = 1;
  int n = wakeup_->set_signal(&one, sizeof(one));
  if (n != sizeof(one))
    CHAOSLOG_ERROR << "EventLoop::wakeup - writes " << n << " bytes instead of `8`";
}

void EventLoop::cancel(Neptune::TimerID timerid) {
  timer_queue_->cancel(timerid);
}

void EventLoop::update_channel(Channel* channel) {
  CHAOS_CHECK(channel->get_loop() == this, "channel operations should in self thread");
  assert_in_loopthread();
  poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel* channel) {
  CHAOS_CHECK(channel->get_loop() == this, "channel operations should in self thread");
  assert_in_loopthread();

  if (event_handling_) {
    CHAOS_CHECK(current_active_channel_ == channel ||
        std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end(),
        "channel should be current thread or not in active channels list");
  }
  poller_->remove_channel(channel);
}

bool EventLoop::has_channel(Channel* channel) {
  CHAOS_CHECK(channel->get_loop() == this, "channel operations should in self thread");
  assert_in_loopthread();
  return poller_->has_channel(channel);
}

std::size_t EventLoop::get_functor_count(void) const {
  Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
  return pending_functors_.size();
}

Neptune::TimerID EventLoop::run_at(Chaos::Timestamp time, const Neptune::TimerCallback& fn) {
  return timer_queue_->add_timer(fn, time, 0.0);
}

Neptune::TimerID EventLoop::run_at(Chaos::Timestamp time, Neptune::TimerCallback&& fn) {
  return timer_queue_->add_timer(std::move(fn), time, 0.0);
}

Neptune::TimerID EventLoop::run_after(double delay, const Neptune::TimerCallback& fn) {
  Chaos::Timestamp time(Chaos::time_add(Chaos::Timestamp::now(), delay));
  return run_at(time, fn);
}

Neptune::TimerID EventLoop::run_after(double delay, Neptune::TimerCallback&& fn) {
  Chaos::Timestamp time(Chaos::time_add(Chaos::Timestamp::now(), delay));
  return run_at(time, std::move(fn));
}

Neptune::TimerID EventLoop::run_every(double interval, const Neptune::TimerCallback& fn) {
  Chaos::Timestamp time(Chaos::time_add(Chaos::Timestamp::now(), interval));
  return timer_queue_->add_timer(fn, time, interval);
}

Neptune::TimerID EventLoop::run_every(double interval, Neptune::TimerCallback&& fn) {
  Chaos::Timestamp time(Chaos::time_add(Chaos::Timestamp::now(), interval));
  return timer_queue_->add_timer(std::move(fn), time, interval);
}

void EventLoop::run_in_loop(const PendFunction& fn) {
  if (in_loopthread())
    fn();
  else
    put_in_loop(fn);
}

void EventLoop::run_in_loop(PendFunction&& fn) {
  if (in_loopthread())
    fn();
  else
    put_in_loop(std::move(fn));
}

void EventLoop::put_in_loop(const PendFunction& fn) {
  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    pending_functors_.push_back(fn);
  }

  if (!in_loopthread() || calling_pending_functors_)
    wakeup();
}

void EventLoop::put_in_loop(PendFunction&& fn) {
  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    pending_functors_.push_back(std::move(fn));
  }

  if (!in_loopthread() || calling_pending_functors_)
    wakeup();
}

EventLoop* EventLoop::get_loop_in_currentthread(void) {
  return t_loopthread;
}

void EventLoop::abort_not_in_loopthread(void) {
  CHAOSLOG_SYSFATAL << "EventLoop::abort_not_in_loopthread - " << this
    << " was created in thread: " << tid_
    << " current thread: " << Chaos::CurrentThread::get_tid();
}

void EventLoop::do_handle_read(void) {
  std::uint64_t one = 1;
  int n = wakeup_->get_signal(sizeof(one), &one);
  if (n != sizeof(one))
    CHAOSLOG_ERROR << "EventLoop::do_handle_read - reads " << n << " bytes instead of `8`";
}

void EventLoop::do_pending_functors(void) {
  std::vector<PendFunction> functors;

  calling_pending_functors_ = true;
  {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    functors.swap(pending_functors_);
  }
  for (auto& fn : functors)
    fn();
  calling_pending_functors_ = false;
}

void EventLoop::debug_active_channels(void) const {
  for (auto ch : active_channels_)
    CHAOSLOG_TRACE << "EventLoop::debug_active_channels - {" << ch->revents_to_string() << "}";
}

}
