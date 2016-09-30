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
#ifndef DEVIL_COND_HEADER_H
#define DEVIL_COND_HEADER_H

#include <stdint.h>
#include "devil_mutex.h"

#if defined(DEVIL_WINDOWS)
# include <Windows.h>
  /* kernal condition variable definition */
  typedef struct kern_cond_t {
    size_t waiters_count;
    CRITICAL_SECTION waiters_count_lock;
    HANDLE signal_event;
    HANDLE broadcast_event;
  } kern_cond_t;
#else
# include <pthread.h>
  /* kernal condition variable definition */
  typedef pthread_cond_t kern_cond_t;
#endif

/* condition variable definition */
typedef struct devil_cond_t {
  devil_mutex_t* mutex;
  kern_cond_t cond;
} devil_cond_t;

/*
 * @attention:
 *    All interfaces of condition variable,
 *    you must ensure the validity of the
 *    incoming parameters.
 */
int devil_cond_init(devil_cond_t* cond, devil_mutex_t* mutex);
void devil_cond_destroy(devil_cond_t* cond);
void devil_cond_signal(devil_cond_t* cond);
void devil_cond_broadcast(devil_cond_t* cond);
void devil_cond_wait(devil_cond_t* cond);
int devil_cond_timedwait(devil_cond_t* cond, uint32_t millitm);

#endif  /* DEVIL_COND_HEADER_H */
