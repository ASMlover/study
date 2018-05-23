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
#include "nyx_timer_manager.h"

namespace nyx { namespace core {

static constexpr std::int64_t kMaxPerTick = 100;
std::int64_t timer_manager::tick_interval_ = kMaxPerTick;


std::uint32_t timer_manager::add_timer_proxy(
    PyObject* proxy, double delay, bool repeat) {
  if (func_.is_none())
    return 0;

  if (delay < 0 ||
      delay >= static_cast<double>(std::numeric_limits<std::int64_t>::max()))
    return 0;
  auto delay_ms = std::max(std::int64_t(delay * 1000000), std::int64_t(1));
  auto now_ms = now();

  std::unique_lock<std::mutex> g(timer_mutex_);
  ++next_timer_id_;
  if (BOOST_UNLIKELY(next_timer_id_ == 0)) {
    next_timer_id_ = 1;
    while (timers_map_.find(next_timer_id_) != timers_map_.end())
      ++next_timer_id_;
  }

  auto timerp = std::make_shared<timer>(next_timer_id_, delay_ms, repeat);
  timerp->set_timer_proxy(proxy);
  timerp->update_expire_time(now_ms);
  timers_set_.insert(timerp);
  timers_map_[next_timer_id_] = timerp;

  return next_timer_id_;
}

void timer_manager::del_timer(std::uint32_t timer_id) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  auto it = timers_map_.find(timer_id);
  if (it != timers_map_.end()) {
    if (BOOST_UNLIKELY(it->second->is_calling())) {
      it->second->cancel();
    }
    else {
      it->second->cancel();
      it->second->clear_timer_proxy();
      timers_set_.erase(it->second);
      timers_map_.erase(it);
    }
  }
}

void timer_manager::stop_all_timers(void) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  stopped_ = true;
  for (auto& t : timers_set_)
    t->cancel();
  timers_set_.clear();
  timers_map_.clear();
}

std::size_t timer_manager::call_expired_timers(void) {
  return 0;
}

std::int64_t timer_manager::get_nearest(void) const {
  std::unique_lock<std::mutex> g(timer_mutex_);
  if (timers_set_.empty())
    return -1;
  return (*timers_set_.begin())->remain_time(now());
}

}}
