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
#include <cassert>
#include <cmath>
#include "ext_timer.h"
#include "ext_timermgr.h"

namespace ext {

TimerMgr::TimerMgr(void)
  : expires_(100)
  , callback_(boost::python::detail::borrowed_reference(Py_None)) {
}

TimerMgr::~TimerMgr(void) {
  if (!stoped_)
    stop_all_timers();
}

std::uint32_t TimerMgr::add_timer(bool is_repeat, double delay) {
  assert(callback_.ptr() != Py_None);

  double intpart;
  double fractpart = std::modf(delay, &intpart);
  long millisec = std::max((int)(fractpart * 1000), 1);

  std::unique_lock<std::mutex> g(timer_mutex_);
  ++next_id_;
  auto timer = std::make_shared<Timer>(
      next_id_, delay, intpart, millisec, is_repeat);
  timers_[next_id_] = timer;
  timer->start();

  return next_id_;
}

void TimerMgr::del_timer(id_t timer_id) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  auto it = timers_.find(timer_id);
  if (it != timers_.end()) {
    (*it).second->stop();
    timers_.erase(it);
  }
}

void TimerMgr::remove(id_t timer_id) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  timers_.erase(timer_id);
}

void TimerMgr::stop_all_timers(void) {
  std::unique_lock<std::mutex> g(timer_mutex_);
  stoped_ = true;
  for (auto& t : timers_)
    t.second->stop();
  timers_.clear();
}

void TimerMgr::set_callback(boost::python::object& callback) {
  callback_ = callback;
}

std::size_t TimerMgr::call_expired_timers(void) {
  std::size_t count{};
  id_t timer_id{};
  while (expires_.pop(timer_id)) {
    auto* res = PyObject_CallMethod(callback_.ptr(), "on_timer", "I", timer_id);
    if (BOOST_UNLIKELY(res == nullptr))
      PyErr_Print();

    if (BOOST_UNLIKELY(++count > kMaxPerTick))
      break;
  }
  return count;
}

}
