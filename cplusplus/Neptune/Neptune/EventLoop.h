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
#ifndef NEPTUNE_EVENTLOOP_H
#define NEPTUNE_EVENTLOOP_H

#include <functional>
#include <memory>
#include <vector>
#include <Chaos/Platform.h>
#include <Chaos/UnCopyable.h>
#include <Chaos/Utility/Any.h>
#include <Chaos/Concurrent/CurrentThread.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Datetime/Timestamp.h>
#include <Neptune/NetInitializer.h>
#include <Neptune/Timer.h>

namespace Neptune {

class Channel;
class Poller;
class TimerQueue;
class WakeupSignaler;

class EventLoop : private Chaos::UnCopyable {
  using PendFunction = std::function<void (void)>;

  bool looping_{}; // need atomic
  bool quit_{}; // need atomic
  bool event_handling_{}; // need atomic
  bool calling_pending_functors_{}; // need atomic
  std::int64_t iteration_{};
  const pid_t tid_{};
  Chaos::Timestamp poll_return_time_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  NetInitializer<> init_;
  std::unique_ptr<WakeupSignaler> wakeup_;
  std::unique_ptr<Channel> wakeup_channel_;
  Chaos::Any context_;
  Channel* current_active_channel_{};
  std::vector<Channel*> active_channels_;
  mutable Chaos::Mutex mutex_;
  std::vector<PendFunction> pending_functors_; // locked by mutex_

  void abort_not_in_loopthread(void);
  void do_handle_read(void); // just for wakeup
  void do_pending_functors(void);
  void debug_active_channels(void) const; // just for debug print
public:
  EventLoop(void);
  ~EventLoop(void);

  void loop(void);
  void quit(void);
  void wakeup(void);
  void cancel(Neptune::TimerID timerid);
  void update_channel(Channel* channel);
  void remove_channel(Channel* channel);
  bool has_channel(Channel* channel);
  std::size_t get_functor_count(void) const;

  Neptune::TimerID run_at(Chaos::Timestamp time, const Neptune::TimerCallback& fn);
  Neptune::TimerID run_at(Chaos::Timestamp time, Neptune::TimerCallback&& fn);
  Neptune::TimerID run_after(double delay, const Neptune::TimerCallback& fn);
  Neptune::TimerID run_after(double delay, Neptune::TimerCallback&& fn);
  Neptune::TimerID run_every(double interval, const Neptune::TimerCallback& fn);
  Neptune::TimerID run_every(double interval, Neptune::TimerCallback&& fn);

  void run_in_loop(const PendFunction& fn);
  void run_in_loop(PendFunction&& fn);
  void put_in_loop(const PendFunction& fn);
  void put_in_loop(PendFunction&& fn);

  static EventLoop* get_loop_in_currentthread(void);

  void assert_in_loopthread(void) {
    if (!in_loopthread())
      abort_not_in_loopthread();
  }

  bool in_loopthread(void) const {
    return tid_ == Chaos::CurrentThread::get_tid();
  }

  Chaos::Timestamp get_poll_return_time(void) const {
    return poll_return_time_;
  }

  std::int64_t get_iteration(void) const {
    return iteration_;
  }

  bool get_event_handling(void) const {
    return event_handling_;
  }

  void set_context(const Chaos::Any& context) {
    context_ = context;
  }

  const Chaos::Any& get_context(void) const {
    return context_;
  }

  Chaos::Any* get_mutable_context(void) {
    return &context_;
  }
};

}

#endif // NEPTUNE_EVENTLOOP_H
