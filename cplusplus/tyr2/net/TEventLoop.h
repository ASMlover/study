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
#ifndef __TYR_NET_EVENTLOOP_HEADER_H__
#define __TYR_NET_EVENTLOOP_HEADER_H__

#include <memory>
#include <vector>
#include "../basic/TAny.h"
#include "../basic/TUnCopyable.h"
#include "../basic/TPlatform.h"
#include "../basic/TCurrentThread.h"
#include "../basic/TMutex.h"
#include "../basic/TTimestamp.h"
#include "TSocketInitializer.h"
#include "TTimer.h"

namespace tyr { namespace net {

class Channel;
class Poller;
class TimerQueue;
class WakeupSignaler;

class EventLoop : private basic::UnCopyable {
  typedef std::function<void (void)> FunctorCallback;

  bool looping_{}; // need atomic
  bool quit_{}; // need atomic
  bool event_handling_{}; // need atomic
  bool calling_pending_functors_{}; // need atomic
  int64_t iteration_;
  const pid_t tid_{};
  basic::Timestamp poll_return_time_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  SocketInitializer<> init_; // socket initializer
  std::unique_ptr<WakeupSignaler> wakeup_;
  std::unique_ptr<Channel> wakeup_channel_;
  basic::Any context_;
  std::vector<Channel*> active_channels_;
  Channel* current_active_channel_{};
  mutable basic::Mutex mtx_;
  std::vector<FunctorCallback> pending_functors_; // locked by mtx_

  void abort_not_in_loopthread(void);
  void handle_read(void); // just for waked up
  void do_pending_functors(void);

  void debug_active_channels(void) const; // just for debug print
public:
  EventLoop(void);
  ~EventLoop(void);

  void loop(void);
  void quit(void);
  void wakeup(void);
  void cancel(TimerID timerid);
  void update_channel(Channel* channel);
  void remove_channel(Channel* channel);
  bool has_channel(Channel* channel);

  size_t get_functor_count(void) const;

  TimerID run_at(basic::Timestamp time, const TimerCallback& fn);
  TimerID run_at(basic::Timestamp time, TimerCallback&& fn);
  TimerID run_after(double delay, const TimerCallback& fn);
  TimerID run_after(double delay, TimerCallback&& fn);
  TimerID run_every(double interval, const TimerCallback& fn);
  TimerID run_every(double interval, TimerCallback&& fn);

  void run_in_loop(const FunctorCallback& fn);
  void run_in_loop(FunctorCallback&& fn);
  void put_in_loop(const FunctorCallback& fn);
  void put_in_loop(FunctorCallback&& fn);

  static EventLoop* get_loop_of_current_thread(void);

  void assert_in_loopthread(void) {
    if (!in_loopthread())
      abort_not_in_loopthread();
  }

  bool in_loopthread(void) const {
    return tid_ == basic::CurrentThread::tid();
  }

  basic::Timestamp get_poll_return_time(void) const {
    return poll_return_time_;
  }

  int64_t get_iteration(void) const {
    return iteration_;
  }

  bool get_event_handling(void) const {
    return event_handling_;
  }

  void set_context(const basic::Any& context) {
    context_ = context;
  }

  const basic::Any& get_context(void) const {
    return context_;
  }

  basic::Any* get_mutable_context(void) {
    return &context_;
  }
};

}}

#endif // __TYR_NET_EVENTLOOP_HEADER_H__
