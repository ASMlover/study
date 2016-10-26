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
#ifndef TPP_POSIX_THREAD_H_
#define TPP_POSIX_THREAD_H_

#include <pthread.h>
#include <functional>
#include "../tpp_types.h"

namespace tpp {

typedef std::function<void (void*)> ThreadCallback;

class Thread : private UnCopyable {
  pthread_t thread_{0};
  ThreadCallback closure_{nullptr};
  void* argument_;
private:
  static void* thread_callback(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    if (nullptr == thread)
      return nullptr;

    if (nullptr != thread->closure_)
      thread->closure_(thread->argument_);

    return nullptr;
  }

  void start_thread(void) {
    int ec = pthread_create(&thread_, 0, &Thread::thread_callback, this);
    if (0 != ec)
      __libtpp_throw_error(ec, "posix.Thread.start_thread: create thread failed");
  }
public:
  Thread(const ThreadCallback& cb, void* arg)
    : closure_(cb)
    , argument_(arg) {
    start_thread();
  }

  ~Thread(void) {
    pthread_detach(thread_);
  }

  void join(void) {
    pthread_join(thread_, 0);
  }
};

}

#endif // TPP_POSIX_THREAD_H_
