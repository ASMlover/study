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
#ifndef __TYR_BASIC_PLATFORM_HEADER_H__
#define __TYR_BASIC_PLATFORM_HEADER_H__

#include "TConfig.h"
#include "TTypes.h"

#if defined(TYR_WINDOWS)
# include "windows/TPlatformWindows.h"
#else
# include "posix/TPlatformPosix.h"
#endif

namespace tyr { namespace basic {

pid_t kern_getpid(void);
pid_t kern_gettid(void);

int kern_mutex_init(kern_mutex_t* mtx);
int kern_mutex_destroy(kern_mutex_t* mtx);
int kern_mutex_lock(kern_mutex_t* mtx);
int kern_mutex_unlock(kern_mutex_t* mtx);

int kern_this_thread_setname(const char* name);

int kern_cond_init(kern_cond_t* cond);
int kern_cond_destroy(kern_cond_t* cond);
int kern_cond_signal(kern_cond_t* cond);
int kern_cond_broadcast(kern_cond_t* cond);
int kern_cond_wait(kern_cond_t* cond, kern_mutex_t* mtx);
int kern_cond_timedwait(kern_cond_t* cond, kern_mutex_t* mtx, uint64_t nanosec);

int kern_thread_create(kern_thread_t* thread, kern_start_routine_t start_routine, void* arg);
int kern_thread_created_signal(kern_thread_t* thread);
int kern_thread_join(kern_thread_t* thread);

}}

#endif // __TYR_BASIC_PLATFORM_HEADER_H__
