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
#include <windows.h>
#if !defined(_MT)
  #error "requires /MTd, /MT , /MDd or /MD complie options"
#endif
#include <process.h>
#include <assert.h>
#include "sl_allocator.h"
#include "sl_thread.h"


#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define inline __inline
#endif



struct sl_thread_t {
  HANDLE thread;
  HANDLE create_event;
  void (*routine)(void*);
  void* argument;
};






static UINT WINAPI 
sl_thread_routine(void* arg)
{
  sl_thread_t* thread = (sl_thread_t*)arg;
  assert(NULL != thread);

  if (NULL != thread->routine)
    thread->routine(thread->argument);

  return 0;
}



sl_thread_t* 
sl_thread_create(void (*routine)(void*), void* arg)
{
  sl_thread_t* thread = (sl_thread_t*)sl_malloc(sizeof(sl_thread_t));
  assert(NULL != thread);

  thread->thread = NULL;
  thread->create_event = NULL;
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
  thread->create_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  assert(NULL != thread->create_event);

  thread->thread = (HANDLE)_beginthreadex(NULL, 
      0, sl_thread_routine, thread, 0, NULL);
  assert(NULL != thread->thread);

  if (NULL != thread->thread)
    WaitForSingleObject(thread->create_event, INFINITE);
  CloseHandle(thread->create_event);
}

void 
sl_thread_join(sl_thread_t* thread)
{
  if (NULL != thread->thread) {
    WaitForSingleObject(thread->thread, INFINITE);

    CloseHandle(thread->thread);
    thread->thread = NULL;
  }
}
