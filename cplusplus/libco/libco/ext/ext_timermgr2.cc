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
#include <algorithm>
#include "ext_utils.h"
#include "ext_timer2.h"
#include "ext_timermgr2.h"

namespace ext {

Timer2Mgr::Timer2Mgr(void)
  : func_(boost::python::detail::borrowed_reference(Py_None)) {
}

Timer2Mgr::~Timer2Mgr(void) {
  if (!stoped_)
    stop_all_timers();
}

id_t Timer2Mgr::add_tiemr_proxy(bool is_repeat, double delay, PyObject* proxy) {
  if (func_.is_none())
    return 0;

  if (delay >= (double)std::numeric_limits<std::int64_t>::max() || delay < 0)
    return 0;

  std::int64_t delay_time =
    std::max(std::int64_t(delay * 1000000), std::int64_t(1));
  std::int64_t now = ext::get_microseconds();

  std::unique_lock<std::mutex> g(timer_mutex_);
  ++next_id_;
  if (BOOST_UNLIKELY(next_id_ == 0)) {
    next_id_ = 1;
    while (timers_.find(next_id_) != timers_.end())
      ++next_id_;
  }

  auto timer = std::make_shared<Timer2>(next_id_, delay_time, is_repeat);
  timer->set_proxy(proxy);
  timer->refresh_expire_time(now);
  timers_set_.insert(timer);
  timers_[next_id_] = timer;

  return next_id_;
}

void Timer2Mgr::del_timer(id_t timer_id) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  auto it = timers_.find(timer_id);
  if (it != timers_.end()) {
    it->second->cancel();
    it->second->clear();
    timers_set_.erase(it->second);
    timers_.erase(it);
  }
}

void Timer2Mgr::stop_all_timers(void) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  stoped_ = true;
  for (auto& t : timers_set_)
    t->cancel();
  timers_set_.clear();
  timers_.clear();
}

void Timer2Mgr::set_functor(boost::python::object& func) {
  func_ = func;
}

void Timer2Mgr::reset_functor(void) {
  func_ = boost::python::object();
}

std::size_t Timer2Mgr::call_expired_timers(void) {
  std::size_t count{};
  std::int64_t now = ext::get_microseconds();
  std::unique_lock<std::mutex> g(timer_mutex_);
  while (!timers_set_.empty()) {
    auto now_timer = *timers_set_.begin();
    if (!now_timer->is_expired(now))
      break;
    timers_set_.erase(now_timer);

    if (BOOST_UNLIKELY(now_timer->is_cancelled())) {
      timers_.erase(now_timer->get_id());
      now_timer->clear();
      continue;
    }
    now_timer->do_function();

    if (BOOST_UNLIKELY(!now_timer->is_repeat())) {
      timers_.erase(now_timer->get_id());
      now_timer->clear();
    }
    else {
      now_timer->refresh_expire_time(now);
      timers_set_.insert(now_timer);
    }

    if (BOOST_UNLIKELY(++count > kMaxPerTick))
      break;
  }

  return count;
}

}
