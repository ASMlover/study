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
#ifndef __EUTIL_HEADER_H__
#define __EUTIL_HEADER_H__

#if !defined(EUTIL_WIN) && !defined(EUTIL_LINUX)
# if defined(_WINDOWS_) || defined(_MSC_VER) || defined(__MINGW32)
#   define EUTIL_WIN
# elif defined(WIN32_LEAN_AND_MEAN)
#   define EUTIL_WIN
# elif defined(__linux__) || defined(__GNUC__)
#   define EUTIL_LINUX
# else
#   error "DOES NOT SUPPORT THIS PLATFORM !!!"
# endif
#endif

#if defined(_MSC_VER)
# define COMPILER_MSVC
#elif defined(__GNUC__)
# define COMPILER_GCC
#endif

#if defined(COMPILER_GCC)
# define GCC_VER  (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#endif

#if (defined(COMPILER_MSVC) && (_MSC_VER < 1700))\
  || (defined(COMPILER_GCC) && (GCC_VER < 470))
# error "PLEASE USE A HIGHER VERSION OF THE COMPILER !!!"
#endif

// SYSTEM INTERFACES HEADERS
#if defined(EUTIL_WIN)
# include <windows.h>
# include <mmsystem.h>
# include <process.h>
#elif defined(EUTIL_LINUX)
# include <sys/types.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>
# include <pthread.h>
# include <limits.h>

# define MAX_PATH PATH_MAX
#endif

// ANSI C HEADERS
#include <sys/timeb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI C++ HEADERS
#include <functional>
#include <memory>
#include <string>
#include <stdexcept>

// STL HEADERS
#include <algorithm>
#include <map>
#include <unordered_map>
#include <queue>
#include <vector>

// Have our own assert, so we are sure it does not get 
// optomized away in a release build.
#if !defined(EL_ASSERT)
# include "el_io.h"
# define EL_ASSERT(expr) do {\
    if (!(expr)) {\
      el::ColorFprintf(stderr, \
          el::ColorType::COLORTYPE_RED, \
          "Assertion failed in %s on %d : %s\n", \
          __FILE__, \
          __LINE__, \
          #expr);\
      fflush(stderr);\
      abort();\
    }\
  } while (0)
#endif

#if !defined(MIN)
# define MIN(x, y)  ((x) < (y) ? (x) : (y))
#endif

#if !defined(MAX)
# define MAX(x, y)  ((x) > (y) ? (x) : (y))
#endif

#include "el_uncopyable.h"
#include "el_locker.h"
#include "el_condition.h"
#include "el_singleton.h"
#include "el_object_pool.h"
#include "el_object_mgr.h"
#include "el_thread.h"

#endif  // __EUTIL_HEADER_H__
