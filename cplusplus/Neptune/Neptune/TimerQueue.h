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
#ifndef NEPTUNE_TIMERQUEUE_H
#define NEPTUNE_TIMERQUEUE_H

#include <cstdint>
#include <set>
#include <vector>
#include <Chaos/UnCopyable.h>
#include <Chaos/Datetime/Timestamp.h>
#include <Neptune/Callbacks.h>
#include <Neptune/Channel.h>

namespace Neptune {

class EventLoop;
class Timer;
class TimerID;

class TimerQueue : private Chaos::UnCopyable {
  using Entry = std::pair<Chaos::Timestamp, Timer*>;
  using TimerSet = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, std::int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  EventLoop* loop_{};
  const int timerfd_{};
  Channel timerfd_channel_;
  TimerSet timers_;
  bool calling_expired_timers_{};
  ActiveTimerSet active_timers_;
  ActiveTimerSet cancelling_timers_{};

  void do_handle_read(void);
  void poll_timer_internal(bool need_read = false);
  void add_timer_in_loop(Timer* timer);
  void cancel_in_loop(TimerID timerid);
  bool insert(Timer* timer);
  void reset(const std::vector<Entry>& expired, Chaos::Timestamp now);
  std::vector<Entry> get_expired(Chaos::Timestamp now);
public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue(void);

  TimerID add_timer(const Neptune::TimerCallback& fn, Chaos::Timestamp when, double interval);
  TimerID add_timer(Neptune::TimerCallback&& fn, Chaos::Timestamp when, double interval);
  void cancel(TimerID timerid);

  void poll_timer(void); // Just for Windows and Darwin
};

}

#endif // NEPTUNE_TIMERQUEUE_H
