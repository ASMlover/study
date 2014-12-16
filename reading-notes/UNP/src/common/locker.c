/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "common.h"

#if defined(PLATFORM_WIN)
void mutex_init(mutex_t* mutex) {
  InitializeCriticalSection(mutex);
}

void mutex_destroy(mutex_t* mutex) {
  DeleteCriticalSection(mutex);
}

void mutex_lock(mutex_t* mutex) {
  EnterCriticalSection(mutex);
}

void mutex_unlock(mutex_t* mutex) {
  LeaveCriticalSection(mutex);
}
#else
void mutex_init(mutex_t* mutex) {
  int n = pthread_mutex_init(mutex, NULL);
  if (0 == n)
    return;

  errno = n;
  error_print("mutex_init error\n");
}

void mutex_destroy(mutex_t* mutex) {
  int n = pthread_mutex_destroy(mutex);
  if (0 == n)
    return;

  errno = n;
  error_print("mutex_destroy error\n");
}

void mutex_lock(mutex_t* mutex) {
  int n = pthread_mutex_lock(mutex);
  if (0 == n)
    return;

  errno = n;
  error_print("mutex_lock error\n");
}

void mutex_unlock(mutex_t* mutex) {
  int n = pthread_mutex_unlock(mutex);
  if (0 == n)
    return;

  errno = n;
  error_print("mutex_unlock error\n");
}
#endif
