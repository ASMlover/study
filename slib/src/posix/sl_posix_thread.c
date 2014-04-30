/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include <pthread.h>
#include <assert.h>
#include "../sl_allocator.h"
#include "../sl_thread.h"


struct sl_thread_t {
  pthread_t thread_id;
  void (*routine)(void*);
  void* argument;
};




static inline void* 
sl_thread_routine(void* arg)
{
  sl_thread_t* thread = (sl_thread_t*)arg;
  assert(NULL != thread);

  if (NULL != thread->routine)
    thread->routine(thread->argument);

  return NULL;
}




sl_thread_t* 
sl_thread_create(void (*routine)(void*), void* arg)
{
  sl_thread_t* thread = (sl_thread_t*)sl_malloc(sizeof(sl_thread_t));
  assert(NULL != thread);

  thread->thread_id = 0;
  thread->routine = routine;
  thread->argument = arg;

  return thread;
}

void 
sl_thread_release(sl_thread_t* thread)
{
  sl_thread_join(thread);
  sl_free(thread);
}

void 
sl_thread_start(sl_thread_t* thread)
{
  int ret = pthread_create(&thread->thread_id, 
      NULL, sl_thread_routine, thread);
  assert(0 == ret);
}

void 
sl_thread_join(sl_thread_t* thread)
{
  if (0 != thread->thread_id) {
    int ret = pthread_join(thread->thread_id, NULL);
    assert(0 == ret);

    thread->thread_id = 0;
  }
}
