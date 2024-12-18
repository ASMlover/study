// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_BASIC_WEAKCALLBACK_HEADER_H__
#define __TYR_BASIC_WEAKCALLBACK_HEADER_H__

#include <functional>
#include <memory>

namespace tyr { namespace basic {

template <typename T, typename... Args>
class WeakCallback {
  std::weak_ptr<T> wk_obj_;
  std::function<void (T*, Args...)> fn_;
public:
  WeakCallback(const std::weak_ptr<T>& wk_obj, const std::function<void (T*, Args...)>& fn)
    : wk_obj_(wk_obj)
    , fn_(fn) {
  }

  void operator()(Args&&... args) const {
    std::shared_ptr<T> obj(wk_obj_.lock());
    if (obj)
      fn_(obj.get(), std::forward<Args>(args)...);
  }
};

template <typename T, typename... Args>
inline WeakCallback<T, Args...> make_weak_callback(const std::shared_ptr<T>& obj, void (T::*fn)(Args...)) {
  return WeakCallback<T, Args...>(obj, fn);
}

template <typename T, typename... Args>
inline WeakCallback<T, Args...> make_weak_callback(const std::shared_ptr<T>& obj, void (T::*fn)(Args...) const) {
  return WeakCallback<T, Args...>(obj, fn);
}

}}

#endif // __TYR_BASIC_WEAKCALLBACK_HEADER_H__
