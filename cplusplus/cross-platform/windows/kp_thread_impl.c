/*
 * Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <Windows.h>
#include <process.h>
#include <assert.h>
#include <stdlib.h>
#include "../kp_thread.h"

struct _kp_thread_t {
  HANDLE ev;
  HANDLE h;
  void* (*start)(void*);
  void* arg;
};

struct _kp_thread_key_t {
  DWORD key;
  void (*destructor)(void*);
};

static UINT WINAPI _kp_thread_wrapper(void* arg)
{
  struct _kp_thread_t* t = (struct _kp_thread_t*)(arg);
  if (NULL == t)
    return 0;

  SetEvent(t->ev);
  if (t->start)
    t->start(t->arg);

  return 0;
}

int kp_thread_create(kp_thread_t* thread, void* (*start_routine)(void*), void* arg)
{
  struct _kp_thread_t* t = (struct _kp_thread_t*)malloc(sizeof(*t));
  if (NULL == t)
    return -1;

  *thread = t;
  t->ev = CreateEvent(NULL, TRUE, FALSE, NULL);
  assert(NULL != t->ev);

  t->start = start_routine;
  t->arg = arg;
  t->h = (HANDLE)_beginthreadex(NULL, 0, _kp_thread_wrapper, t, 0, NULL);
  assert(NULL != t->h);

  WaitForSingleObject(t->ev, INFINITE);
  CloseHandle(t->ev);
  return 0;
}

int kp_thread_join(kp_thread_t thread)
{
  if (NULL != thread) {
    if (NULL != thread->h) {
      WaitForSingleObject(thread->h, INFINITE);
      CloseHandle(thread->h);
    }
    free(thread);
  }

  return 0;
}

static void _kp_thread_key_clear(kp_thread_key_t key)
{
  void* old_val = TlsGetValue(key->key);
  if (NULL != key->destructor && NULL != old_val)
    key->destructor(old_val);
}

int kp_thread_key_create(kp_thread_key_t* key, void (*destructor)(void*))
{
  struct _kp_thread_key_t* k = (struct _kp_thread_key_t*)malloc(sizeof(*k));
  if (NULL == k)
    return -1;

  *key = k;
  k->key = TlsAlloc();
  k->destructor = destructor;

  return 0;
}

int kp_thread_key_delete(kp_thread_key_t key)
{
  if (NULL != key) {
    _kp_thread_key_clear(key);
    free(key);
  }
  return 0;
}

void* kp_thread_getspecific(kp_thread_key_t key)
{
  return NULL != key ? TlsGetValue(key->key) : NULL;
}

int kp_thread_setspecific(kp_thread_key_t key, const void* value)
{
  if (NULL != key) {
    _kp_thread_key_clear(key);
    return TlsSetValue(key->key, (LPVOID)value) ? 0 : -1;
  }
  return -1;
}
