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
#include <time.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include "../basic/TLogging.h"
#include "TKernWrapper.h"
#include "TTimer.h"
#include "TEventLoop.h"
#include "TTimerQueue.h"

namespace tyr { namespace net {

int create_timerfd(void) {
  int timerfd = Kern::create_timer();
  if (timerfd < 0)
    TYRLOG_SYSFATAL << "Failed in Kern::create_timer";

  return timerfd;
}

int64_t get_msec_from_now(basic::Timestamp when) {
  int64_t msec = when.msec_since_epoch() - basic::Timestamp::now().msec_since_epoch();
  if (msec < 100)
    msec = 100;

  return msec;
}

void read_timerfd(int timerfd, basic::Timestamp now) {
  uint64_t how_many;
  int n = Kern::read_timer(timerfd, &how_many, sizeof(how_many));
  TYRLOG_TRACE << "read_timerfd - " << how_many << " at " << now.to_string();
  if (n != sizeof(how_many))
    TYRLOG_ERROR << "read_timerfd reads " << n << " bytes instead of 8";
}

void reset_timerfd(int timerfd, basic::Timestamp expired) {
  int64_t t = get_msec_from_now(expired);
  if (Kern::set_timer(timerfd, t))
    TYRLOG_SYSERR << "Kern::set_timer";
}

// EventLoop* loop_;
// const int timerfd_;
// Channel timerfd_channel_;
// TimerSet timers_;
// ActiveTimerSet avtive_timers_;
// bool calling_expired_timers_;
// ActiveTimerSet cancelling_timers_;
TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop)
  , timerfd_(create_timerfd())
  , timerfd_channel_(loop_, timerfd_)
  , timers_() {
  timerfd_channel_.set_read_callback(std::bind(&TimerQueue::handle_read, this));
  timerfd_channel_.enabled_reading();
}

TimerQueue::~TimerQueue(void) {
  // close(timerfd_);

  for (auto& t : timers_)
    delete t.second;
}

TimerID TimerQueue::add_timer(const TimerCallback& cb, basic::Timestamp when, double interval) {
  Timer* timer = new Timer(cb, when, interval);
  loop_->assert_in_loopthread();
  bool earlist_changed = insert(timer);

  if (earlist_changed)
    reset_timerfd(timerfd_, timer->expiry_time());
  return TimerID(timer);
}
// TimerID add_timer(TimerCallback&& cb, tyr::basic::Timestamp when, double interval);
// void cancel(TimerID timerid);
// void add_timer_in_loop(Timer* timer);
// void cancel_in_loop(TimerID timerid);

void TimerQueue::handle_read(void) {
  loop_->assert_in_loopthread();
  basic::Timestamp now(basic::Timestamp::now());
  read_timerfd(timerfd_, now);

  std::vector<Entry> expired = get_expired(now);
  for (auto& entry : expired)
    entry.second->run();

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(basic::Timestamp now) {
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  auto it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, std::back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, basic::Timestamp now) {
  basic::Timestamp next_expired;

  for (auto& entry : expired) {
    if (entry.second->repeat()) {
      entry.second->restart(now);
      insert(entry.second);
    }
    else {
      delete entry.second;
    }
  }

  if (!timers_.empty())
    next_expired = timers_.begin()->second->expiry_time();
  if (next_expired.is_valid())
    reset_timerfd(timerfd_, next_expired);
}

bool TimerQueue::insert(Timer* timer) {
  bool changed{};
  basic::Timestamp when = timer->expiry_time();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first)
    changed = true;
  auto result = timers_.insert(std::make_pair(when, timer));
  assert(result.second);

  return changed;

  return true;
}

}}
