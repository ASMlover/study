/*
 * Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_NET_HEADER_H__
#define __EL_NET_HEADER_H__

#if !defined(ELNET_WIN) && !defined(ELNET_LINUX) && !defined(ELNET_MAC)
# if defined(_WINDOWS_) || defined(_MSC_VER) || defined(__MINGW32)
#   define ELNET_WIN
# elif defined(WIN32_LEAN_AND_MEAN)
#   define ELNET_WIN
# elif defined(__linux__)
#   define ELNET_LINUX
# elif defined(__APPLE__) || defined(__MACH__)
#   define ELNET_MAC
# else
#   error "DOES NOT SUPPORT THIS PLATFORM !!!"
# endif
#endif

/* SYSTEM INTERFACES HEADERS */
#if defined(ELNET_WIN)
# include <windows.h>
# include <direct.h>
# include <io.h>
# include <mmsystem.h>
# include <process.h>

# undef __func__
# define __func__ __FUNCSIG__
#else
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <fcntl.h>
# include <limits.h>
# include <pthread.h>
# include <unistd.h>

# define MAX_PATH PATH_MAX
# if defined(ELNET_MAC)
#   include <libkern/OSAtomic.h>
#   include <mach/mach.h>
#   include <mach/mach_time.h>
# endif
#endif

/* ANSI C HEADERS */
#include <assert.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Have our own assert, so we are sure it does not get
 * optomized away in a release build.
 */
#if !defined(EL_ASSERT)
# define EL_ASSERT(condition) do {\
  if (!(condition)) {\
    fprintf(stderr, \
        "[%s:%d] Assertion failed in %s(): %s\n", \
        __FILE__, \
        __LINE__, \
        __func__, \
        #condition);\
    fflush(stderr);\
    abort();\
  }\
} while (0)
#endif

#define EL_ASSERTX(condition, message) do {\
  if (!(condition)) {\
    fprintf(stderr, \
        "[%s:%d] Assertion failed in %s(): %s\n", \
        __FILE__, \
        __LINE__, \
        __func__, \
        (message));\
    fflush(stderr);\
    abort();\
  }\
} while (0)

/* Assertion to indicate that the given point in 
 * the code should never be reached.
 */
#define EL_UNREACHABLE()\
  EL_ASSERTX(0, "This line should not be reached.")

#if defined(ELNET_WIN)
# ifndef snprintf
#   define snprintf _snprintf
# endif

# define EL_ARRAY(type, name, size)\
  type* name = (type*)_alloca((size) * sizeof(type))
#else
# define EL_ARRAY(type, name, size) type name[size]
#endif

#endif /* __EL_NET_HEADER_H__ */
