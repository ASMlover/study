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
#ifndef __SL_TEST_HEADER_H__
#define __SL_TEST_HEADER_H__

#include <stdio.h>
#include <stdlib.h>

#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define inline    __inline
  #define __func__  __FUNCTION__
#endif 

#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define cmdeq(c1, c2) (0 == stricmp(c1, c2))
#elif defined(__linux__)
  #define cmdeq(c1, c2  (0 == strcasecmp(c1, c2))
#endif 

#ifndef countof
  #define countof(x)  (sizeof((x)) / sizeof(*(x)))
#endif 


/*
 * Have our own assert, so we are sure it dose not get 
 * optomized away in a release build.
 */
#define ASSERT(expr)\
do {\
  if (!(expr)) {\
    fprintf(stderr, \
      "assertion failed in %s on line %d : %s\n", \
      __FILE__, \
      __LINE__, \
      #expr);\
      fflush(stderr);\
      abort();\
  }\
} while (0)


extern void sl_test_allocator(void);
extern void sl_test_queue(void);

#endif  /* __SL_TEST_HEADER_H__ */
