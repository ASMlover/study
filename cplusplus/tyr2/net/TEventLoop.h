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
#include "../basic/TUnCopyable.h"
#include "../basic/TPlatform.h"
#include "../basic/TCurrentThread.h"
#include "../basic/TMutex.h"
#include "../basic/TTimestamp.h"
#include "TTimer.h"

namespace tyr { namespace net {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : private basic::UnCopyable {
  typedef std::function<void (void)> FunctorCallback;

  bool looping_{}; // need atomic
  bool quit_{}; // need atomic
  bool calling_pending_functors_{}; // need atomic
  const pid_t tid_{};
  basic::Timestamp poll_return_time_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;
  std::vector<Channel*> active_channels_;
  basic::Mutex mtx_;
  std::vector<FunctorCallback> pending_functors_; // locked by mtx_

  void abort_not_in_loopthread(void);
  void handle_read(void); // for waked up
  void do_pending_functors(void);
public:
  EventLoop(void);
  ~EventLoop(void);

  void loop(void);
  void quit(void);
  void wakeup(void);
  void update_channel(Channel* channel);

  TimerID run_at(basic::Timestamp time, const TimerCallback& fn);
  TimerID run_after(double delay, const TimerCallback& fn);
  TimerID run_every(double interval, const TimerCallback& fn);

  void run_in_loop(const FunctorCallback& cb);
  void put_in_loop(const FunctorCallback& cb);

  void assert_in_loopthread(void) {
    if (!in_loopthread())
      abort_not_in_loopthread();
  }

  bool in_loopthread(void) const {
    return tid_ == basic::CurrentThread::tid();
  }
};

}}

#endif // __TYR_NET_EVENTLOOP_HEADER_H__
