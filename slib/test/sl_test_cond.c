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
#include "sl_test_header.h"
#include "../src/sl_thread.h"
#include "../src/sl_mutex.h"
#include "../src/sl_cond.h"


#if defined(_WINDOWS_) || defined(_MSC_VER)
  #include <windows.h>

  #define sl_sleep(x)   Sleep(x)
#elif defined(__linux__)
  #include <unistd.h>

  #define sl_sleep(x)   usleep(1000 * (x))
#endif


typedef struct {
  sl_mutex_t mutex;
  sl_cond_t cond;
  int posted;
  int use_broadcast;
} worker_condig;



static void 
worker(void* arg)
{
  worker_condig* wc = (worker_condig*)arg;

  sl_sleep(100);
  fprintf(stdout, "\t%s -> before lock\n", __func__);
  sl_mutex_lock(&wc->mutex);
  fprintf(stdout, "\t%s -> before signal/broadcast\n", __func__);
  ASSERT(0 == wc->posted);
  wc->posted = 1;
  if (wc->use_broadcast)
    sl_cond_broadcast(&wc->cond);
  else 
    sl_cond_signal(&wc->cond);
  fprintf(stdout, "\t%s -> before unlock\n", __func__);
  sl_mutex_unlock(&wc->mutex);
  fprintf(stdout, "\t%s -> after unlock\n", __func__);
}

void 
sl_test_condition(void)
{
  worker_condig wc;
  sl_thread_t* thread;

  fprintf(stdout, "begin testing condition module : <%s>\n", __func__);
  memset(&wc, 0, sizeof(wc));
  sl_mutex_init(&wc.mutex);
  sl_cond_init(&wc.cond, &wc.mutex);

  thread = sl_thread_create(worker, (void*)&wc);
  sl_thread_start(thread);
  fprintf(stdout, "\t%s -> before lock\n", __func__);
  sl_mutex_lock(&wc.mutex);
  fprintf(stdout, "\t%s -> before wait\n", __func__);
  sl_cond_wait(&wc.cond);
  ASSERT(1 == wc.posted);
  fprintf(stdout, "\t%s -> before unlock\n", __func__);
  sl_mutex_unlock(&wc.mutex);
  fprintf(stdout, "\t%s -> before join\n", __func__);
  sl_thread_join(thread);
  fprintf(stdout, "\t%s -> after join\n\n", __func__);

  wc.posted = 0;
  wc.use_broadcast = 1;
  thread = sl_thread_create(worker, (void*)&wc);
  sl_thread_start(thread);
  fprintf(stdout, "\t%s -> before lock\n", __func__);
  sl_mutex_lock(&wc.mutex);
  fprintf(stdout, "\t%s -> before wait\n", __func__);
  sl_cond_wait(&wc.cond);
  ASSERT(1 == wc.posted);
  fprintf(stdout, "\t%s -> before unlock\n", __func__);
  sl_mutex_unlock(&wc.mutex);
  fprintf(stdout, "\t%s -> before join\n", __func__);
  sl_thread_join(thread);
  fprintf(stdout, "\t%s -> after join\n", __func__);

  sl_thread_release(thread);

  sl_cond_destroy(&wc.cond);
  sl_mutex_destroy(&wc.mutex);
  
  fprintf(stdout, "end testing condition module : all passed!!!\n");
}
