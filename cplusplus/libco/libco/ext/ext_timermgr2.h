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
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "ext_helper.h"
#include "ext_timer2.h"

namespace ext {

class Timer2;

struct Timer2Cmp {
  bool operator()(const std::shared_ptr<Timer2>& x,
      const std::shared_ptr<Timer2>& y) const {
    if (x->get_id() == y->get_id()) {
      return false;
    }
    else {
      if (x->get_expire_time() != y->get_expire_time())
        return x->get_expire_time() < y->get_expire_time();
      else
        return x->get_id() < y->get_id();
    }
  }
};

class Timer2Mgr : private boost::noncopyable {
  bool stoped_{};
  id_t next_id_{1};
  std::unordered_map<id_t, std::shared_ptr<Timer2>> timers_;
  std::set<std::shared_ptr<Timer2>, Timer2Cmp> timers_set_;
  // std::unordered_set<std::shared_ptr<Timer2>, Timer2Cmp> timers_set_;
  std::mutex timer_mutex_;
  boost::python::object func_;

  static constexpr std::size_t kMaxPerTick = 100;
public:
  Timer2Mgr(void);
  ~Timer2Mgr(void);

  id_t add_tiemr_proxy(bool is_repeat, double delay, PyObject* proxy);
  void del_timer(id_t timer_id);
  void stop_all_timers(void);

  void set_functor(boost::python::object& func);
  void reset_functor(void);

  std::size_t call_expired_timers(void);
};

}
