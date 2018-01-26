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
#include "ext_helper.h"

namespace ext {

class Timer2
  : public std::enable_shared_from_this<Timer2>
  , public boost::noncopyable {
  id_t id_{};
  std::int64_t delay_time_{};
  std::int64_t expire_time_{};
  bool is_repeat_{};
  bool is_cancelled_{};

  PyObject* func_{};
  PyObject* args_{};
  PyObject* kwargs_{};
public:
  Timer2(id_t id, std::int64_t delay, bool is_repeat);
  ~Timer2(void);

  bool operator<(const Timer2& o) const;
  void clear(void);
  void set_proxy(PyObject* proxy);
  void do_function(void);

  void refresh_expire_time(std::int64_t now) {
    expire_time_ = now + delay_time_;
  }

  std::int64_t remain_time(std::int64_t now) {
    return expire_time_ - now;
  }

  void cancel(void) {
    is_cancelled_ = true;
  }

  id_t get_id(void) const {
    return id_;
  }

  std::int64_t get_expire_time(void) const {
    return expire_time_;
  }

  bool is_expired(std::int64_t now) const {
    return expire_time_ < now;
  }

  bool is_repeat(void) const {
    return is_repeat_;
  }

  bool is_cancelled(void) const {
    return is_cancelled_;
  }
};

}
