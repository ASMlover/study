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
#ifndef TPP_POSIX_CONTEXT_H_
#define TPP_POSIX_CONTEXT_H_

#include <ucontext.h>
#include <functional>
#include "../tpp_types.h"
#include "../tpp_context_support.h"

namespace tpp {

class Context {
  ucontext_t context_;
  std::function<void (void)> closure_;
  char* stack_{nullptr};
  uint32_t stack_size_{0};
  uint32_t protect_page_{0};
public:
  Context(const std::function<void (void)>& closure, uint32_t stack_size)
    : closure_(closure)
    , stack_size_(stack_size) {
    int ec = getcontext(&context_);
    if (0 != ec)
      __libtpp_throw_error(ec, "make context failed");

    stack_ = (char*)malloc(stack_size_);
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_link = nullptr;

    makecontext(&context_, (void(*)(void))&Context::context_closure, 1, &closure_);

    uint32_t protect_page = __libtpp_get_protect_stack_page();
    if (0 != protect_page) {
      if (0 == __libtpp_protect_stack(stack_, stack_size_, protect_page))
        protect_page_ = protect_page;
    }
  }

  ~Context(void) {
    if (nullptr != stack_) {
      if (0 != protect_page_)
        __libtpp_unprotect_stack(stack_, protect_page_);
      free(stack_);
      stack_ = nullptr;
    }
  }

  ucontext_t& get_tls_context(void) {
    static __thread ucontext_t _s_main_context;
    return _s_main_context;
  }

  bool swap_in(void) {
    return 0 == swapcontext(&get_tls_context(), &context_);
  }

  bool swap_out(void) {
    return 0 == swapcontext(&context_, &get_tls_context());
  }
private:
  static void context_closure(std::function<void (void)>* fn) {
    (*fn)();
  }
};

}

#endif // TPP_POSIX_CONTEXT_H_
