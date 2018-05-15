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
#include <vector>
#include <unordered_map>
#include "../utils/nyx_time.h"
#include "nyx_timer.h"

namespace nyx { namespace core {

class timer_manager : private boost::noncopyable {
  using timer_ptr = std::shared_ptr<timer>;

  struct __timer_compare {
    bool operator()(const timer_ptr& x, const timer_ptr& y) const {
      if (x->get_id() == y->get_id()) {
        return false;
      }

      if (x->get_expire_time() != y->get_expire_time())
        return x->get_expire_time() < y->get_expire_time();
      else
        return x->get_id() < y->get_id();
    }
  };

  using __timer_map = std::unordered_map<std::uint32_t, timer_ptr>;
  using __timer_set = std::set<timer_ptr, __timer_compare>;

  bool stopped_{};
  std::uint32_t next_timer_id_{1};
  __timer_map timers_map_;
  __timer_set tiemrs_set_;
  std::mutex timer_mutex_;
  py::object func_;
  static std::int64_t tick_interval_;

  static constexpr std::int64_t kMaxPerTick = 100;
public:
  timer_manager(void);
  ~timer_manager(void);

  static void set_tick_interval(std::int64_t interval) {
    tick_interval_ = interval;
  }

  std::int64_t now(void) const {
    return nyx::time::chrono_microseconds();
  }

  std::uint32_t add_timer_proxy(PyObject* proxy, double delay, bool repeat);
  void del_timer(std::uint32_t timer_id);
  void stop_all_timers(void);
  void set_functor(const py::object& handler);
  void reset_functor(void);
  std::size_t call_expired_timers(void);
  std::int64_t get_nearest(void) const;
};

}}
