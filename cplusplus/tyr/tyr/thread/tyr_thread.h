// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_THREAD_HEADER_H__
#define __TYR_THREAD_HEADER_H__

// ATOMIC
#if defined(TYR_CPP0X)
# include "cpp11/tyr_cpp11_atomic_counter.h"
#else
# if defined(TYR_OS_WIN)
#   include "win/tyr_win_atomic_counter.h"
# elif defined(TYR_OS_LINUX)
#   include "posix/tyr_posix_atomic_counter.h"
# elif defined(TYR_OS_MAC)
#   include "mac/tyr_mac_atomic_counter.h"
# else
#   include "tyr_atomic_counter.h"
# endif
#endif
#include "tyr_locker.h"

// SEMAPHORE
#if defined(TYR_OS_WIN)
# include "win/tyr_win_sem.h"
#elif defined(TYR_OS_LINUX)
# include "posix/tyr_posix_sem.h"
#elif defined(TYR_OS_MAC)
# include "mac/tyr_mac_sem.h"
#endif

// CONDITION VARIABLE
#if defined(TYR_CPP0X)
# include "cpp11/tyr_cpp11_condition.h"
#else
# if defined(TYR_OS_WIN)
#  include "win/tyr_win_condition.h"
# elif defined(TYR_OS_LINUX)
#  include "posix/tyr_posix_condition.h"
# elif defined(TYR_OS_MAC)
#  include "mac/tyr_mac_condition.h"
#endif

namespace tyr {

typedef std::function<void (void*)> RoutinerType;

}

#if defined(TYR_CPP0X)
# include "cpp11/tyr_cpp11_thread.h"
#else
# if defined(TYR_OS_WIN)
#   include "win/tyr_win_thread.h"
# else
#   include "posix/tyr_posix_thread.h"
# endif
#endif

#endif  // __TYR_THREAD_HEADER_H__
