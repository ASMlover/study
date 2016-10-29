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
#ifndef TPP_WINDOWS_CONTEXT_H_
#define TPP_WINDOWS_CONTEXT_H_

#include <Windows.h>
#include <functional>
#include "../tpp_types.h"

namespace tpp {

struct ScopedContext {
  typedef void* fiber_ptr;

  static __declspec(thread) fiber_ptr _s_native{nullptr};

  ScopedContext(void) {
    _s_native = ConvertThreadToFiber(nullptr);
  }

  ~ScopedContext(void) {
    ConvertFiberToThread();
    _s_native = nullptr;
  }

  static fiber_ptr& get_tls_context(void) {
    return _s_native;
  }
};

class Context {
  std::function<void (void)> closure_;
  void* native_{nullptr};
  uint32_t stack_size_{0};
public:
  Context(const std::function<void (void)>& closure, uint32_t stack_size)
    : closure_(closure)
    , stack_size_(stack_size) {
    SIZE_T commit_size = 4 * 1024;
    native_ = CreateFiberEx(commit_size,
        std::max<uint32_t>(stack_size, commit_size), FIBER_FLAG_FLOAT_SWITCH,
        (LPFIBER_START_ROUTIN)&Context::fiber_closure, &closure_);
    if (nullptr == native_) {
      int ec = static_cast<int>(GetLastError());
      __libtpp_throw_error(ec, "make context failed");
    }
  }

  ~Context(void) {
    if (nullptr != native_) {
      DeleteFiber(native_);
      native_ = nullptr;
    }
  }

  bool swap_in(void) {
    return SwitchToFiber(native_), true;
  }

  bool swap_out(void) {
    return SwitchToFiber(ScopedContext::get_tls_context()), true;
  }
private:
  static void CALLBACK fiber_closure(void* arg) {
    std::function<void (void)>* fn = (std::function<void (void)>)arg;
    (*fn)();
  }
};

}

#endif // TPP_WINDOWS_CONTEXT_H_
