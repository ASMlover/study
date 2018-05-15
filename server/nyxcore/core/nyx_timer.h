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

#include <cstdint>
#include <Python.h>
#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#include "../utils/nyx_pyaux.h"

namespace nyx { namespace core {

class timer : private boost::noncopyable {
  std::uint32_t timer_id_{};
  std::int64_t delay_time_{};
  std::int64_t expire_time_{};
  bool repeat_{};
  bool cancelled_{};
  bool calling_{};
  PyObject* proxy_{};
public:
  timer(std::uint32_t id, std::int64_t delay, bool repeat)
    : timer_id_(id)
    , delay_time_(delay)
    , repeat_(repeat) {
  }

  std::uint32_t get_id(void) const {
    return timer_id_;
  }

  std::int64_t get_expire_time(void) const {
    return expire_time_;
  }

  void update_expire_time(std::int64_t now) {
    expire_time_ = now + delay_time_;
  }

  bool is_expired(std::int64_t now) const {
    return expire_time_ < now;
  }

  std::int64_t remain_time(std::int64_t now) const {
    return expire_time_ <= now ? 0 : expire_time_ - now;
  }

  bool is_repeat(void) const {
    return repeat_;
  }

  bool is_cancelled(void) const {
    return cancelled_;
  }

  bool is_calling(void) const {
    return calling_;
  }

  void cancel(void) {
    cancelled_ = true;
  }

  void set_timer_proxy(PyObject* proxy) {
    clear_timer_proxy();
    Py_INCREF(proxy);
    proxy_ = proxy;
  }

  void clear_timer_proxy(void) {
    if (proxy_ != nullptr) {
      Py_DECREF(proxy_);
      proxy_ = nullptr;
    }
  }

  void call_proxy(void) {
    calling_ = true;
    if (proxy_ != nullptr) {
      _NYXCORE_TRY {
        py::call_method<void>(proxy_, "tick");
      } _NYXCORE_CATCH
    }
    calling_ = false;
  }

  bool operator<(const timer& other) const {
    if (expire_time_ > other.expire_time_)
      return true;
    else if (expire_time_ < other.expire_time_)
      return false;
    else
      return timer_id_ > other.timer_id_;
  }
};

}}
