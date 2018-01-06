// Copyright (c) 2018 ASMlover. All rights reserved.
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
#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>
#include <boost/lockfree/queue.hpp>
#include <boost/python.hpp>
#include "ext_helper.h"

namespace ext {

class Timer;

using TimerPtr = std::shared_ptr<Timer>;
using TimerMap = std::unordered_map<std::uint32_t, TimerPtr>;

class TimerMgr : private boost::noncopyable {
  bool stoped_{};
  std::uint32_t next_id_{};
  std::mutex timer_mutex_;
  TimerMap timers_;
  boost::lockfree::queue<std::uint32_t> expires_;
  boost::python::object handler_;

  TimerMgr(void);
  ~TimerMgr(void);
public:
  static TimerMgr& instance(void) {
    static TimerMgr s;
    return s;
  }

  std::uint32_t add_timer(bool is_repeat, double delay);
  void del_timer(std::uint32_t timer_id);
  void unreg(std::uint32_t timer_id);
  void stop_all(void);
  void set_handler(boost::python::object& handler);
  std::uint32_t call_expires(void);

  bool append_expire(std::uint32_t timer_id) {
    return expires_.push(timer_id);
  }
};

}
