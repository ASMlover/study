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
#include <process.h>
#include <memory>
#include "../tpp_threading_support.h"

namespace tpp {

__libtpp_thread_id __libtpp_get_thread_id(void) {
  return GetCurrentThreadId();
}

int __libtpp_mutex_init(__libtpp_mutex_t* m) {
  return InitializeCriticalSection(m), 0;
}

int __libtpp_mutex_destrpy(__libtpp_mutex_t* m) {
  return DeleteCriticalSection(m), 0;
}

int __libtpp_mutex_lock(__libtpp_mutex_t* m) {
  if ((DWORD)m->OwningThread != __libtpp_get_thread_id())
    EnterCriticalSection(m);
  return 0;
}

int __libtpp_mutex_trylock(__libtpp_mutex_t* m) {
  if ((DWORD)m->OwningThread != __libtpp_get_thread_id())
    return TryEnterCriticalSection(m) ? 0 : -1;
  return 0;
}

int __libtpp_mutex_unlock(__libtpp_mutex_t* m) {
  return LeaveCriticalSection(m), 0;
}

int __libtpp_condvar_init(__libtpp_condvar_t* cv) {
  return InitializeConditionVariable(cv), 0;
}

int __libtpp_condvar_destroy(__libtpp_condvar_t* /*cv*/) {
  return 0;
}

int __libtpp_condvar_signal(__libtpp_condvar_t* cv) {
  return WakeConditionVariable(cv), 0;
}

int __libtpp_condvar_broadcast(__libtpp_condvar_t* cv) {
  return WakeAllConditionVariable(cv), 0;
}

int __libtpp_condvar_wait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m) {
  return SleepConditionVariableCS(cv, m, INFINITE), 0;
}

int __libtpp_condvar_timedwait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m, uint64_t nanosec) {
  return SleepConditionVariableCS(cv, m, (DWORD)(nanosec / 1e6));
}

struct __libtpp_invoker {
  void (*__fn)(void*);
  void* __arg;
  __libtpp_condvar_t* __cv;
  __libtpp_mutex_t* __m;
  int* __started;

  __libtpp_invoker(void (*fn)(void*), void* arg, __libtpp_condvar_t* cv, __libtpp_mutex_t* m, int* started)
    : __fn(fn)
    , __arg(arg)
    , __cv(cv)
    , __m(m)
    , __started(started) {
  }
};

static UINT WINAPI __libtpp_thread_closure(void* arg) {
  std::unique_ptr<__libtpp_invoker> b(static_cast<__libtpp_invoker*>(arg));

  __libtpp_mutex_lock(b->__m);
  *b->__started = 1;
  __libtpp_condvar_signal(b->__cv);
  __libtpp_mutex_unlock(b->__m);

  if (nullptr != b->__fn)
    b->__fn(b->__arg);

  return 0;
}

int __libtpp_thread_create(__libtpp_thread_t* t, void (*closure)(void*), void* arg) {

  int started = 0;
  __libtpp_condvar_t cv;
  __libtpp_mutex_t m;

  __libtpp_condvar_init(&cv);
  __libtpp_mutex_init(&m);
  std::unique_ptr<__libtpp_invoker> b(new __libtpp_invoker(closure, arg, &cv, &m, &started));

  __libtpp_mutex_lock(&m);
  t->__h = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, __libtpp_thread_closure, b.get(), 0, &t->__id));
  if (nullptr != t->__h) {
    while (!started)
      __libtpp_condvar_wait(&cv, &m);
    b.release();
  }
  __libtpp_mutex_unlock(&m);

  __libtpp_mutex_destrpy(&m);
  __libtpp_condvar_destroy(&cv);

  return 0;
}

int __libtpp_thread_join(__libtpp_thread_t* t) {
  DWORD r;
  if (WAIT_FAILED == WaitForSingleObjectEx(t->__h, INFINITE, FALSE)
      || 0 == GetExitCodeThread(t->__h, &r))
    return -1;
  return CloseHandle(t->__h) ? 0 : -1;
}

int __libtpp_thread_detach(__libtpp_thread_t* t) {
  return CloseHandle(t->__h) ? 0 : -1;
}

int __libtpp_tls_create(__libtpp_tls_key* key, void (*closure)(void*)) {
  return *key = FlsAlloc((PFLS_CALLBACK_FUNCTION)closure), 0;
}

int __libtpp_tls_delete(__libtpp_tls_key key) {
  return FlsFree(key) ? 0 : -1;
}

int __libtpp_tls_set(__libtpp_tls_key key, void* p) {
  return FlsSetValue(key, p) ? 0 : -1;
}

void* __libtpp_tls_get(__libtpp_tls_key key) {
  return FlsGetValue(key);
}

}
