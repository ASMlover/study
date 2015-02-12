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
#ifndef __EL_POLL_HEADER_H__
#define __EL_POLL_HEADER_H__

#if !defined(EL_WIN) || !defined(EL_LINUX) || !defined(EL_MAC)
# undef EL_WIN
# undef EL_LINUX
# undef EL_MAC

# if defined(_WINDOWS_) || defined(_MSC_VER) || defined(__MINGW32)\
  || defined(WIN32_LEAN_AND_MEAN)
#   define EL_WIN
# elif defined(__linux__)
#   define EL_LINUX
# elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)\
  || defined(__NetBSD__)
#   define EL_MAC
# else
#   error "does not support this platform ..."
# endif
#endif

#if defined(EL_WIN) && defined(_MSC_VER)
# define COMPILER_MSVC
#elif defined(EL_LINUX) && defined(__GNUC__)
# define COMPILER_GCC
#elif defined(EL_MAC) && defined(__clang__)
# define COMPILER_CLANG
#endif

#if defined(COMPILER_MSVC)
# define COMPILER_VER (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#elif defined(COMPILER_CLANG)
# define COMPILER_VER (__clang_major__ * 100 + __clang_minor__ * 10)
#endif

#if (defined(COMPILER_MSVC) && (_MSC_VER < 1700))\
  || (defined(COMPILER_GCC) && (COMPILER_VER < 470))\
  || (defined(COMPILER_CLANG) && (COMPILER_VER < 330))
# error "please use a higher version of the compiler ..."
#endif

#if defined(EL_WIN)
# include <winsock2.h>
# include <direct.h>
# include <io.h>
# include <mmsystem.h>
# include <process.h>

# undef __func__
# define __func__ __FUNCSIG__

  typedef int socklen_t;
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <fcntl.h>
# include <limits.h>
# include <netdb.h>
# include <pthread.h>
# include <unistd.h>

# define MAX_PATH PATH_MAX
# if defined(EL_MAC)
#   include <libkern/OSAtomic.h>
#   include <mach/mach.h>
#   include <mach/mach_time.h>
# endif
#endif

#include <assert.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#define EL_ASSERT(condition, message) do {\
  if (!(condition)) {\
    fprintf(stderr, \
        "[%s:%d] Assertion failed in %s: %s\n", \
        __FILE__, \
        __LINE__, \
        __func__, \
        (message));\
    fflush(stderr);\
    abort();\
  }\
} while (0)

#define EL_UNREACHABLE()\
  EL_ASSERT(false, "This line should not be reached.")

#define EL_MIN(x, y)  ((x) < (y) ? (x) : (y))
#define EL_MAX(x, y)  ((x) > (y) ? (x) : (y))

#if defined(EL_WIN)
# define snprintf _snprintf

# define bzero(s, n)  memset((s), 0, (n))
#endif

#define EL_NETINVAL (-1)
#define EL_NETERR   (-1)

#include "el_uncopyable.h"
#include "el_locker.h"
#include "el_thread.h"

namespace el {

struct Entity {
  virtual ~Entity(void) {}
  virtual int GetEntity(void) const = 0;
  virtual int Read(uint32_t bytes, char* buffer) = 0;
  virtual int Write(const char* buffer, uint32_t bytes) = 0;
};

// interfaces for user
struct EventHandler {
  virtual ~EventHandler(void) {}
  virtual bool AcceptEvent(Entity& entity) { return true; }
  virtual void CloseEvent(Entity& entity) {}
  virtual bool ReadEvent(Entity& entity) { return true; }
};

}

#endif  // __EL_POLL_HEADER_H__
