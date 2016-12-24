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
#ifndef __TYR_NET_TIMERQUEUE_HEADER_H__
#define __TYR_NET_TIMERQUEUE_HEADER_H__

#include <set>
#include <vector>
#include "../basic/TMutex.h"
#include "../basic/TTimestamp.h"
#include "TCallbacks.h"
#include "TChannel.h"

namespace tyr { namespace net {

class EventLoop;
class TimerID;
class Timer;

class TimerQueue : private basic::UnCopyable {
  typedef std::pair<basic::Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerSet;
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  EventLoop* loop_{};
  const int timerfd_;
  Channel timerfd_channel_;
  TimerSet timers_;
  bool calling_expired_timers_{};
  ActiveTimerSet active_timers_;
  ActiveTimerSet cancelling_timers_;
public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue(void);

  TimerID add_timer(const TimerCallback& cb, tyr::basic::Timestamp when, double interval);
  TimerID add_timer(TimerCallback&& cb, tyr::basic::Timestamp when, double interval);
  void cancel(TimerID timerid);
private:
  void add_timer_in_loop(Timer* timer);
  void cancel_in_loop(TimerID timerid);
  void handle_read(void);
  std::vector<TimerQueue::Entry> get_expired(tyr::basic::Timestamp now);
  void reset(const std::vector<Entry>& expired, tyr::basic::Timestamp now);
  bool insert(Timer* timer);
};

}}

#endif // __TYR_NET_TIMERQUEUE_HEADER_H__
