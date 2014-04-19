// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __TINYCLT_HEADER_H__
#define __TINYCLT_HEADER_H__

#if !defined(USE_WINDOWS) || !defined(USE_POSIX)
# if defined(_WINDOWS_) || defined(_MSC_VER)
#   define USE_WINDOWS
# elif defined(__linux__) || defined(__GNUC__)
#   define USE_POSIX
# else
#   error "Unsupport this platform !"
# endif
#endif

#if defined(USE_WINDOWS)
# if !defined(_WINDOWS_)
#   include <winsock2.h>
# endif
# include <process.h>
#elif defined(USE_POSIX)
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <pthread.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <functional>
#include <memory>

#include <queue>

#define TC_ASSERT(expr) do {\
  if (!(expr)) {\
    fprintf(stderr, \
        "assertion failed in %s at %d : %s", \
        __FILE__, \
        __LINE__, \
        #expr);\
    fflush(stderr);\
    abort();\
  }\
} while (0)


enum NetType {
  NETTYPE_INVAL = -1, 
  NETTYPE_ERR   = -1, 
};


#if defined(USE_WINDOWS)
# define GetCurrentMS timeGetTime
#elif defined(USE_POSIX)
inline uint32_t GetCurrentMS(void) {
  struct timeval tv;
  if (0 == gettimeofday(&tv, 0))
    return (tv.tv_sec - 1000000000) * 1000 + (tv.tv_usec / 1000);
  return 0;
}
#endif


#include "tc_uncopyable.h"
#include "tc_locker.h"
#include "tc_thread.h"

#endif  // __TINYCLT_HEADER_H__
