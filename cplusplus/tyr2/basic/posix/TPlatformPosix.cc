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
#include <sys/syscall.h>
#include <unistd.h>
#include "../TPlatform.h"

namespace tyr { namespace basic {

pid_t kern_getpid(void) {
  return getpid();
}

int kern_mutex_init(KernMutex* mtx) {
  return pthread_mutex_init(mtx, nullptr);
}

int kern_mutex_destroy(KernMutex* mtx) {
  return pthread_mutex_destroy(mtx);
}

int kern_mutex_lock(KernMutex* mtx) {
  return pthread_mutex_lock(mtx);
}

int kern_mutex_unlock(KernMutex* mtx) {
  return pthread_mutex_unlock(mtx);
}

int kern_cond_init(KernCond* cond) {
  return pthread_cond_init(cond, nullptr);
}

int kern_cond_destroy(KernCond* cond) {
  return pthread_cond_destroy(cond);
}

int kern_cond_signal(KernCond* cond) {
  return pthread_cond_signal(cond);
}

int kern_cond_broadcast(KernCond* cond) {
  return pthread_cond_broadcast(cond);
}

int kern_cond_wait(KernCond* cond, KernMutex* mtx) {
  return pthread_cond_wait(cond, mtx);
}

int kern_cond_timedwait(KernCond* cond, KernMutex* mtx, uint64_t nanosec) {
  struct timespec ts;
  kern_gettime(&ts);
  ts.tv_sec += nanosec / NANOSEC;
  ts.tv_nsec = nanosec % NANOSEC;
  return pthread_cond_timedwait(cond, mtx, &ts);
}

int kern_thread_create(KernThread* thread, void* (*start_routine)(void*), void* arg) {
  return pthread_create(thread, nullptr, start_routine, arg);
}

int kern_thread_join(KernThread thread) {
  return pthread_join(thread, nullptr);
}

int kern_thread_detach(KernThread thread) {
  return pthread_detach(thread);
}

int kern_thread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
  return pthread_atfork(prepare, parent, child);
}

int kern_threadkey_create(KernThreadKey* key, void (*destructor)(void*)) {
  return pthread_key_create(key, destructor);
}

int kern_threadkey_delete(KernThreadKey key) {
  return pthread_key_delete(key);
}

int kern_setspecific(KernThreadKey key, const void* value) {
  return pthread_setspecific(key, value);
}

void* kern_getspecific(KernThreadKey key) {
  return pthread_getspecific(key);
}

}}
