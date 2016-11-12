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
#ifndef __TYR_NET_TIMER_HEADER_H__
#define __TYR_NET_TIMER_HEADER_H__

#include <atomic>
#include "../basic/TUnCopyable.h"
#include "../basic/TTimestamp.h"
#include "TCallbacks.h"

namespace tyr { namespace net {

class Timer;

class TimerID {
  Timer* timer_{};
  int64_t sequence_{};

  friend class TimerQueue;
public:
  TimerID(void) = default;

  TimerID(Timer* timer, int64_t seq)
    : timer_(timer)
    , sequence_(seq) {
  }
};

class Timer : private tyr::basic::UnCopyable {
  const TimerCallback closure_;
  tyr::basic::Timestamp expiry_time_;
  const double interval_;
  const bool repeat_;
  const int64_t sequence_;

  static std::atomic<int64_t> s_num_created_;
public:
  Timer(const TimerCallback& fn, tyr::basic::Timestamp when, double interval)
    : closure_(fn)
    , expiry_time_(when)
    , interval_(interval)
    , repeat_(interval > 0)
    , sequence_(++s_num_created_) {
  }

  Timer(TimerCallback&& fn, tyr::basic::Timestamp when, double interval)
    : closure_(std::move(fn))
    , expiry_time_(when)
    , interval_(interval)
    , repeat_(interval > 0)
    , sequence_(++s_num_created_) {
  }

  void run(void) const {
    closure_();
  }

  tyr::basic::Timestamp expiry_time(void) const {
    return expiry_time_;
  }

  bool repeat(void) const {
    return repeat_;
  }

  int64_t sequence(void) const {
    return sequence_;
  }

  void restart(tyr::basic::Timestamp now);

  static int64_t num_created(void) {
    return s_num_created_;
  }
};

}}

#endif // __TYR_NET_TIMER_HEADER_H__
