//! Copyright (c) 2014 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __GLOBAL_HEADER_H__
#define __GLOBAL_HEADER_H__

#if !defined(EGAME_WIN) && !defined(EGAME_LINUX)
# if defined(_WINDOWS_) || defined(_MSC_VER)
#   define EGAME_WIN
# elif defined(WIN32_LEAN_AND_MEAN)
#   define EGAME_WIN
# elif defined(__linux__) || defined(__GNUC__)
#   define EGAME_LINUX
# else
#   error "unsupport this platform!"
# endif
#endif

#if defined(EGAME_WIN)
# if (_MSC_VER < 1600)
#   if (_MSC_VER < 1300)
      typedef signed char       int8_t;
      typedef unsigned char     uint8_t;
      typedef signed short      int16_t;
      typedef unsigned short    uint16_t;
      typedef signed int        int32_t;
      typedef unsigned int      uint32_t;
#   else
      typedef signed __int8     int8_t;
      typedef unsigned __int8   uint8_t;
      typedef signed __int16    int16_t;
      typedef unsigned __int16  uint16_t;
      typedef signed __int32    int32_t;
      typedef unsigned __int32  uint32_t;
#   endif
    typedef signed __int64      int64_t;
    typedef unsigned __int64    uint64_t;
# else
#   include <stdint.h>
# endif
#else
# include <stdint.h>
#endif

// system interface header
#if defined(EGAME_WIN)
# include <windows.h>
# include <process.h>
# include <io.h>
# include <direct.h>
#elif defined(EGAME_LINUX)
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
# include <fcntl.h>
# include <pthread.h>
# include <limits.h>

# define MAX_PATH PATH_MAX
#endif

// ANSI C header
#include <sys/timeb.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI C++ header
#include <memory>
#include <queue>
#include <map>
#include <set>
#include <vector>

// user library header
#include <libnet.h>

// user common utils header
#include "uncopyable.h"
#if defined(EGAME_LINUX)
# include "posix_global.h"
#endif
#include "locker.h"
#include "singleton.h"

#endif  //! __GLOBAL_HEADER_H__
