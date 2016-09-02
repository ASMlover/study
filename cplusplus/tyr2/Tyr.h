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
#ifndef __TYR_HEADER_H__
#define __TYR_HEADER_H__

#if defined(_WINDOWS_) || defined(_WIN32) || defined(_WIN64)
# define TYR_WIN
#elif defined(__linux__)
# define TYR_LNX
#elif defined(__APPLE__) || defined(__MACH__)
# define TYR_MAC
#else
# error "ERROR PLATFORM !!!"
#endif

#if defined(TYR_WIN)
# include <Windows.h>
# include <process.h>

# define __func__ __FUNCTION__
#else
# include <sys/stat.h>
# include <pthread.h>
# include <unistd.h>
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#define TYR_CHECK(cond) do {\
  if (!(cond)) {\
    fprintf(stderr,\
        "[%s:%d] Assertion failed in %s() - %s\n",\
        __FILE__,\
        __LINE__,\
        __func__,\
        #cond);\
    fflush(stderr);\
    abort();\
  }\
} while (0)

#include "TUnCopyable.h"
#include "TMutex.h"

#endif // __TYR_HEADER_H__
