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
#ifndef TPP_THREAD_H_
#define TPP_THREAD_H_

#include <functional>
#include <memory>
#include "tpp_config.h"
#include "tpp_types.h"
#include "tpp_threading_support.h"

namespace tpp {

typedef std::function<void (void*)> ThreadCallback;

class Thread : private UnCopyable {
  __libtpp_thread_t thread_{0};
  ThreadCallback closure_{nullptr};
  void* argument_{nullptr};

  static void thread_closure(void* arg) {
    Thread* t = static_cast<Thread*>(arg);
    if (t && t->closure_)
      t->closure_(t->argument_);
  }
public:
  Thread(const ThreadCallback& closure, void* arguemnt = nullptr)
    : closure_(closure)
    , argument_(arguemnt) {
    int ec = __libtpp_thread_create(&thread_, &Thread::thread_closure, this);
    if (0 != ec)
      __libtpp_throw_error(ec, "Thread construction failed");
  }

  ~Thread(void) {
    __libtpp_thread_detach(&thread_);
  }

  void join(void) {
    __libtpp_thread_join(&thread_);
  }
};

}

#endif // TPP_THREAD_H_
