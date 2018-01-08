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

#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include "ext_helper.h"

namespace ext {

class Timer;

class TimerMgr : private boost::noncopyable {
  bool stoped_{};
  id_t next_id_{};
  std::mutex timer_mutex_;
  std::unordered_map<id_t, std::shared_ptr<Timer>> timers_;
  std::vector<id_t>* expires_{};
  std::vector<id_t>* expires_copy_{};
  std::mutex expires_mutex_;
  PyObject* callback_{};

  TimerMgr(void);
  ~TimerMgr(void);
public:
  static TimerMgr& instance(void) {
    static TimerMgr s;
    return s;
  }

  id_t add_timer(bool is_repeat, double delay);
  void del_timer(id_t timer_id);
  void remove(id_t timer_id);
  void stop_all_timers(void);
  void set_callback(PyObject* callback);
  std::size_t call_expired_timers(void);

  bool append_expired_timer(id_t timer_id) {
    std::unique_lock<std::mutex> g(expires_mutex_);
    expires_copy_->emplace_back(timer_id);
    return true;
  }
};

}
