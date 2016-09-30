/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#ifndef DEVIL_TEST_HEADER_H
#define DEVIL_TEST_HEADER_H

#include <stddef.h>
#include "../src/devil_config.h"

#if defined(DEVIL_WINDOWS)
# define inline __inline
# define __func__ __FUNCTION__
#endif

#if defined(DEVIL_WINDOWS)
# define cmdeq(c1, c2) (0 == stricmp(c1, c2))
#else
# define cmdeq(c1, c2) (0 == strcasecmp(c1, c2))
#endif

#ifndef countof
# define countof(x) (sizeof((x)) / sizeof(*(x)))
#endif

/*
 * Have our own assert, so we are sure it dose not get
 * optomized away in a release build.
 */
#define DEVIL_ASSERT(expr) do {\
  if (!(expr)) {\
    fprintf(stderr,\
      "assertion failed in %s on line %d : %s\n",\
      __FILE__,\
      __LINE__,\
      #expr);\
      fflush(stderr);\
      abort();\
  }\
} while (0)

void devil_test_allocator(void);
void devil_test_queue(void);
void devil_test_list(void);
void devil_test_table(void);
void devil_test_array(void);
void devil_test_mutex(void);
void devil_test_spinlock(void);
void devil_test_condition(void);
void devil_test_thread(void);

#endif  /* DEVIL_TEST_HEADER_H */
