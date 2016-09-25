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
#ifndef __SL_CONDITION_HEADER_H__
#define __SL_CONDITION_HEADER_H__ 

#include "sl_mutex.h"

#if defined(_WINDOWS_) || defined(_MSC_VER)
  #include <windows.h>
  typedef struct cond_t {
    size_t waiters_count;
    CRITICAL_SECTION waiters_count_lock;
    HANDLE signal_event;
    HANDLE broadcast_event;
  } cond_t;
#elif defined(__linux__)
  #include <pthread.h>
  typedef pthread_cond_t cond_t;
#endif

typedef struct sl_cond_t {
  sl_mutex_t* mutex;
  cond_t cond;
} sl_cond_t;



/*
 * @attention:
 *    All interfaces of condition module, 
 *    you must ensure the validity of the 
 *    incoming parameters.
 */

extern int sl_cond_init(sl_cond_t* cond, sl_mutex_t* mutex);
extern void sl_cond_destroy(sl_cond_t* cond);
extern void sl_cond_signal(sl_cond_t* cond);
extern void sl_cond_broadcast(sl_cond_t* cond);
extern void sl_cond_wait(sl_cond_t* cond);
extern int sl_cond_timedwait(sl_cond_t* cond, unsigned int millitm);


#endif  /* __SL_CONDITION_HEADER_H__ */
