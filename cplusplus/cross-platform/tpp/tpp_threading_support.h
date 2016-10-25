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
#ifndef TPP_THREADINGSUPPORT_H_
#define TPP_THREADINGSUPPORT_H_

#include <stdint.h>
#include "tpp_config.h"
#if defined(TPP_WINDOWS)
# include "windows/tpp_windows_threading_support.h"
#else
#endif

namespace tpp {

int __libtpp_mutex_init(__libtpp_mutex_t* m);
int __libtpp_mutex_destrpy(__libtpp_mutex_t* m);
int __libtpp_mutex_lock(__libtpp_mutex_t* m);
int __libtpp_mutex_trylock(__libtpp_mutex_t* m);
int __libtpp_mutex_unlock(__libtpp_mutex_t* m);

int __libtpp_condvar_init(__libtpp_condvar_t* cv);
int __libtpp_condvar_destroy(__libtpp_condvar_t* cv);
int __libtpp_condvar_signal(__libtpp_condvar_t* cv);
int __libtpp_condvar_broadcast(__libtpp_condvar_t* cv);
int __libtpp_condvar_wait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m);
int __libtpp_condvar_timedwait(__libtpp_condvar_t* cv, __libtpp_mutex_t* m, uint64_t nanosec);

int __libtpp_thread_create(__libtpp_thread_t* t, void (*closure)(void*), void* arg);
int __libtpp_thread_join(__libtpp_thread_t* t);
int __libtpp_thread_detach(__libtpp_thread_t* t);

int __libtpp_tls_create(__libtpp_tls_key* key, void (*closure)(void*));
int __libtpp_tls_delete(__libtpp_tls_key key);
int __libtpp_tls_set(__libtpp_tls_key key, void* p);
void* __libtpp_tls_get(__libtpp_tls_key key);

}

#endif // TPP_THREADINGSUPPORT_H_
