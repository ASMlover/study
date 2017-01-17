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
#ifndef NEPTUNE_TIMER_H
#define NEPTUNE_TIMER_H

#include <atomic>
#include <Chaos/UnCopyable.h>
#include <Neptune/Callbacks.h>

namespace Neptune {

class Timer;

class TimerID {
  Timer* timer_{};
  std::int64_t sequence_{};

  friend class TimerQueue;
public:
  TimerID(void) = default;

  explicit TimerID(Timer* timer, std::int64_t sequence = 0)
    : timer_(timer)
    , sequence_(sequence) {
  }
};

class Timer : private Chaos::UnCopyable {
  const TimerCallback closure_fn_{};
  Chaos::Timestamp expiry_time_;
  const double interval_{};
  const bool repeat_{};
  const std::int64_t sequence_{};

  static std::atomic<std::int64_t> s_ncreated_;
public:
  Timer(const TimerCallback& fn, Chaos::Timestamp when, double interval)
    : closure_fn_(fn)
    , expiry_time_(when)
    , interval_(interval)
    , repeat_(interval_ > 0)
    , sequence_(++s_ncreated_) {
  }

  Timer(TimerCallback&& fn, Chaos::Timestamp when, double interval)
    : closure_fn_(std::move(fn))
    , expiry_time_(when)
    , interval_(interval)
    , repeat_(interval_ > 0)
    , sequence_(++s_ncreated_) {
  }

  void run(void) const {
    closure_fn_();
  }

  void restart(Chaos::Timestamp now);

  Chaos::Timestamp get_expiry_time(void) const {
    return expiry_time_;
  }

  bool is_repeat(void) const {
    return repeat_;
  }

  std::int64_t get_sequence(void) const {
    return sequence_;
  }

  static std::int64_t get_ncreated(void) {
    return s_ncreated_;
  }
};

}

#endif // NEPTUNE_TIMER_H
