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
#include <stdint.h>
#include <memory>
#include "tpp_posix_time.h"
#include "../tpp_threading_support.h"

namespace tpp {

int __libtpp_mutex_init(__libtpp_mutex_t* m) {
  return pthread_mutex_init(m, 0);
}

int __libtpp_mutex_destrpy(__libtpp_mutex_t* m) {
  return pthread_mutex_destroy(m);
}

int __libtpp_mutex_lock(__libtpp_mutex_t* m) {
  return pthread_mutex_lock(m);
}

int __libtpp_mutex_trylock(__libtpp_mutex_t* m) {
  return pthread_mutex_trylock(m);
}

int __libtpp_mutex_unlock(__libtpp_mutex_t* m) {
  return pthread_mutex_unlock(m);
}

int __libtpp_condvar_init(__libtpp_condvar_t* cv) {
  return pthread_cond_init(cv, 0);
}

int __libtpp_condvar_destroy(__libtpp_condvar_t* cv) {
  return pthread_cond_destroy(cv);
}

int __libtpp_condvar_signal(__libtpp_condvar_t* cv) {
  return pthread_cond_signal(cv);
}

int __libtpp_condvar_broadcast(__libtpp_condvar_t* cv) {
  return pthread_cond_broadcast(cv);
}

int __libtpp_condvar_wait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m) {
  return pthread_cond_wait(cv, m);
}

int __libtpp_condvar_timedwait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m, uint64_t nanosec) {
  struct timespec ts;
  __libtpp_gettime(&ts);
  ts.tv_sec += nanosec / NANOSEC;
  ts.tv_nsec = nanosec % NANOSEC;
  return pthread_cond_timedwait(cv, m, &ts);
}

struct __libtpp_invoker {
  void (*__fn)(void*);
  void* __arg;

  __libtpp_invoker(void (*fn)(void*), void* arg)
    : __fn(fn)
    , __arg(arg) {
  }
};

static void* __libtpp_thread_closure(void* arg) {
  std::unique_ptr<__libtpp_invoker> b(static_cast<__libtpp_invoker*>(arg));
  if (nullptr != b->__fn)
    b->__fn(b->__arg);

  return nullptr;
}

int __libtpp_thread_create(__libtpp_thread_t* t, void (*closure)(void*), void* arg) {
  std::unique_ptr<__libtpp_invoker> b(new __libtpp_invoker(closure, arg));
  int ec = pthread_create(t, 0, __libtpp_thread_closure, b.get());
  if (0 == ec)
    b.release();
  else
    __libtpp_throw_error(ec, "__libtpp_thread_create failed");
  return ec;
}

int __libtpp_thread_join(__libtpp_thread_t* t) {
  if (0 != *t) {
    int ec = pthread_join(*t, 0);
    if (0 == ec)
      *t = 0;
    else
      __libtpp_throw_error(ec, "__libtpp_thread_join failed");
  }
}

int __libtpp_thread_detach(__libtpp_thread_t* t) {
  if (0 != *t) {
    int ec = pthread_detach(*t);
    if (0 == ec)
      *t = 0;
    else
      __libtpp_throw_error(ec, "__libtpp_thread_detach failed");
  }
}

int __libtpp_tls_create(__libtpp_tls_key* key, void (*closure)(void*)) {
  return pthread_key_create(key, closure);
}

int __libtpp_tls_delete(__libtpp_tls_key key) {
  return pthread_key_delete(key);
}

int __libtpp_tls_set(__libtpp_tls_key key, void* p) {
  return pthread_setspecific(key, p);
}

void* __libtpp_tls_get(__libtpp_tls_key key) {
  return pthread_getspecific(key);
}

}
