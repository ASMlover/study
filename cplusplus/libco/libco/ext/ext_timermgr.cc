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
  : expires_(new std::vector<id_t>())
  , expires_copy_(new std::vector<id_t>()) {
}

TimerMgr::~TimerMgr(void) {
  if (!stoped_) {
    for (auto& t : timers_)
      t.second->stop();
    timers_.clear();
  }

  if (expires_) {
    delete expires_;
    expires_ = nullptr;
  }

  std::unique_lock<std::mutex> g(expires_mutex_);
  if (expires_copy_) {
    delete expires_copy_;
    expires_copy_ = nullptr;
  }
}

id_t TimerMgr::add_timer(bool is_repeat, double delay) {
  if (BOOST_UNLIKELY(callback_ == nullptr))
    return 0;

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
  if (BOOST_UNLIKELY(timer_id == 0))
    return;

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
  boost::python::xdecref(callback_);
  callback_ = nullptr;
  for (auto& t : timers_)
    t.second->stop();
  timers_.clear();
}

void TimerMgr::set_callback(PyObject* callback) {
  boost::python::xdecref(callback_);
  callback_ = callback;
  boost::python::xincref(callback_);
}

std::size_t TimerMgr::call_expired_timers(void) {
  if (BOOST_UNLIKELY(stoped_))
    return 0;

  {
    std::unique_lock<std::mutex> g(expires_mutex_);
    if (!expires_copy_->empty())
      std::swap(expires_, expires_copy_);
  }

  for (auto& timer_id : *expires_) {
    auto* res  = PyObject_CallMethod(callback_, "on_timer", "L", timer_id);
    if (BOOST_UNLIKELY(res == nullptr))
      PyErr_Print();
  }
  auto count = expires_->size();
  expires_->clear();

  return count;
}

}
