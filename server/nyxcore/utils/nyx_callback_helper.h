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
#include "nyx_pyaux.h"

namespace nyx {

class callback_guard : private boost::noncopyable {
  bool calling_{true};
public:
  void disable_callback(void) { calling_ = false; }
  bool is_calling(void) const { return calling_; }
};

using callback_guard_ptr = std::shared_ptr<callback_guard>;

template <typename Handler>
class safe_callback : private boost::noncopyable {
public:
  callback_guard_ptr guard_;
  Handler h_;

  safe_callback(const callback_guard_ptr& guard, const Handler& h)
    : guard_(guard)
    , h_(h) {
  }

  safe_callback(const callback_guard_ptr& guard, Handler&& h)
    : guard_(guard)
    , h_(std::move(h)) {
  }

  template <typename ...Args>
  void operator()(Args&&... args) {
    if (guard_->is_calling())
      h_(std::forward<Args>(args)...);
  }

  template <typename ...Args>
  void operator()(Args&&... args) const {
    if (guard_->is_calling())
      h_(std::forward<Args>(args)...);
  }
};

class destroy_guard : private boost::noncopyable {
  callback_guard_ptr guard_;
public:
  destroy_guard(void)
    : guard_(new callback_guard()) {
  }

  ~destroy_guard(void) {
  }

  void on_guard_destroy(void) {
    guard_->disable_callback();
  }

  callback_guard_ptr get_guard(void) const {
    return guard_;
  }

  bool is_destroyed(void) const {
    return !guard_->is_calling();
  }

  template <typename Handler>
  safe_callback<Handler> make_safe_callback(const Handler& h) {
    return safe_callback<Handler>(guard_, h);
  }

  template <typename Handler>
  safe_callback<Handler> make_safe_callback(Handler&& h) {
    return safe_callback<Handler>(guard_, std::move(h));
  }
};

}
