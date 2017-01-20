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
#include <functional>
#include <iterator>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Chaos/Kern/KernCommon.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/EventLoop.h>
#include <Neptune/Timer.h>
#include <Neptune/TimerQueue.h>

namespace Neptune {

namespace Unexposed {
  int timerfd_create(void) {
    int timerfd = Chaos::timer::kern_open();
    if (timerfd < 0)
      CHAOSLOG_SYSFATAL << "timerfd_create - create timerfd failed";
    return timerfd;
  }

  void timerfd_read(int timerfd, Chaos::Timestamp now) {
    std::uint64_t how_many;
    ssize_t n = Chaos::timer::kern_gettime(timerfd, sizeof(how_many), &how_many);
    CHAOSLOG_TRACE << "Unexposed::timerfd_read - " << how_many << " at " << now.to_string();
    if (n != sizeof(how_many))
      CHAOSLOG_ERROR << "Unexposed::timerfd_read - reads " << n << " bytes instead of 8";
  }

  void timerfd_reset(int timerfd, Chaos::Timestamp expired) {
    std::int64_t msec = expired.msec_since_epoch() - Chaos::Timestamp::now().msec_since_epoch();
    if (msec < 100)
      msec = 100;

    if (Chaos::timer::kern_settime(timerfd, msec) != 0)
      CHAOSLOG_SYSERR << "Unexposed::timerfd_reset";
  }
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop)
  , timerfd_(Unexposed::timerfd_create())
  , timerfd_channel_(loop_, timerfd_) {
  if (timerfd_ > 0) {
    timerfd_channel_.bind_read_functor(std::bind(&TimerQueue::do_handle_read, this));
    timerfd_channel_.enabled_reading();
  }
}

TimerQueue::~TimerQueue(void) {
  if (timerfd_ > 0) {
    timerfd_channel_.disabled_all();
    timerfd_channel_.remove();
    Chaos::timer::kern_close(timerfd_);
  }

  for (auto& t : timers_)
    delete t.second;
}

TimerID TimerQueue::add_timer(const Neptune::TimerCallback& fn, Chaos::Timestamp when, double interval) {
  Timer* timer = new Timer(fn, when, interval);
  loop_->run_in_loop(std::bind(&TimerQueue::add_timer_in_loop, this, timer));
  return TimerID(timer, timer->get_sequence());
}

TimerID TimerQueue::add_timer(Neptune::TimerCallback&& fn, Chaos::Timestamp when, double interval) {
  Timer* timer = new Timer(std::move(fn), when, interval);
  loop_->run_in_loop(std::bind(&TimerQueue::add_timer_in_loop, this, timer));
  return TimerID(timer, timer->get_sequence());
}

void TimerQueue::cancel(TimerID timerid) {
  loop_->run_in_loop(std::bind(&TimerQueue::cancel_in_loop, this, timerid));
}

void TimerQueue::poll_timer(void) {
#if defined(CHAOS_LINUX)
  CHAOSLOG_SYSFATAL << "TimerQueue::poll_timer - unable to call in Linux";
#else
  poll_timer_internal(false);
#endif
}

void TimerQueue::do_handle_read(void) {
  poll_timer_internal(true);
}

void TimerQueue::poll_timer_internal(bool need_read) {
  loop_->assert_in_loopthread();
  Chaos::Timestamp now(Chaos::Timestamp::now());

  if (need_read)
    Unexposed::timerfd_read(timerfd_, now);

  calling_expired_timers_ = true;
  cancelling_timers_.clear();
  std::vector<Entry> expired_entries = get_expired(now);
  for (auto& entry : expired_entries)
    entry.second->run();
  calling_expired_timers_ = false;

  reset(expired_entries, now);
}

void TimerQueue::add_timer_in_loop(Timer* timer) {
  loop_->assert_in_loopthread();
  bool earlist_changed = insert(timer);
  if (earlist_changed && timerfd_ > 0)
    Unexposed::timerfd_reset(timerfd_, timer->get_expiry_time());
}

void TimerQueue::cancel_in_loop(TimerID timerid) {
  loop_->assert_in_loopthread();
  CHAOS_CHECK(timers_.size() == active_timers_.size(), "");
  ActiveTimer timer(timerid.timer_, timerid.sequence_);
}

bool TimerQueue::insert(Timer* timer) {
  loop_->assert_in_loopthread();
  CHAOS_CHECK(timers_.size() == active_timers_.size(), "");

  bool changed = false;
  Chaos::Timestamp when = timer->get_expiry_time();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first)
    changed = true;

  {
    auto result = timers_.insert(Entry(when, timer));
    CHAOS_CHECK(result.second, "TimerQueue::insert - insert timer must be valid");
  }
  {
    auto result = active_timers_.insert(ActiveTimer(timer, timer->get_sequence()));
    CHAOS_CHECK(result.second, "TimerQueue::insert - insert active timer must be valid");
  }
  CHAOS_CHECK(timers_.size() == active_timers_.size(), "");

  return changed;
}

void TimerQueue::reset(const std::vector<Entry>& expired_entries, Chaos::Timestamp now) {
  Chaos::Timestamp next_expired;

  for (auto& entry : expired_entries) {
    ActiveTimer timer(entry.second, entry.second->get_sequence());
    if (entry.second->is_repeat()
        && cancelling_timers_.find(timer) == cancelling_timers_.end()) {
      entry.second->restart(now);
      insert(entry.second);
    }
    else {
      delete entry.second;
    }
  }

  if (!timers_.empty())
    next_expired = timers_.begin()->second->get_expiry_time();
  if (next_expired.is_valid() && timerfd_ > 0)
    Unexposed::timerfd_reset(timerfd_, next_expired);
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(Chaos::Timestamp now) {
  std::vector<Entry> expired_entries;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  auto it = timers_.lower_bound(sentry);
  std::copy(timers_.begin(), it, std::back_inserter(expired_entries));
  timers_.erase(timers_.begin(), it);

  for (auto& entry : expired_entries) {
    ActiveTimer timer(entry.second, entry.second->get_sequence());
    std::size_t n = active_timers_.erase(timer);
    CHAOS_CHECK(n == 1, "TimerQueue::get_expired - erased 1 active timer");
  }
  CHAOS_CHECK(timers_.size() == active_timers_.size(), "");

  return expired_entries;
}

}
