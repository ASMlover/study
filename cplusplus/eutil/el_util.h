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
#ifndef __EL_UTIL_HEADER_H__
#define __EL_UTIL_HEADER_H__


#include "el_config.h"

// System interfaces headers
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

// ANSI C headers
#include <sys/timeb.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI C++ headers
#include <functional>
#include <memory>
#include <string>
#include <stdexcept>

// STL headers
#include <algorithm>
#include <map>
#include <queue>
#include <vector>

// Have our own assert, so we are sure it does not get 
// optomized away in a release build.
#define EL_ASSERT(expr) do {\
  if (!(expr)) {\
    fprintf(stderr, \
        "Assertion failed in %s on %d : %s\n", \
        __FILE__, \
        __LINE__, \
        #expr);\
    fflush(stderr);\
    abort();\
  }\
} while (0)


#include "el_uncopyable.h"
#include "el_static_assert.h"
#include "el_locker.h"
#include "el_singleton.h"
#include "el_auto_ptr.h"
#include "el_auto_array.h"
#include "el_ref_counter.h"
#include "el_smart_ptr.h"
#include "el_smart_array.h"
#include "el_object_pool.h"
#include "el_object_mgr.h"
#include "el_thread.h"


#endif  // __EL_UTIL_HEADER_H__
