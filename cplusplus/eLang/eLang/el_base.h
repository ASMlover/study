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
#ifndef __EL_BASE_HEADER_H__
#define __EL_BASE_HEADER_H__

#include <stdlib.h>
#include <iostream>
#include <string>

#ifdef EL_DEBUG
# define EL_ASSERT(condition, message) do {\
    if (!(condition)) {\
      std::cout << "ASSERTION FAILED " << __FILE__ \
                << ":" << __LINE__ << " - " \
                << (message) << std::endl;\
      abort();\
    }\
  } while (0)

# define EL_ASSERT_NIL(value)\
  EL_ASSERT((value) != nullptr, "Exception " #value " cannot be null.")

# define EL_ASSERT_RANGE(value, range) do {\
    if (((value) < 0) || ((value) >= (range))) {\
      std::cout << "ASSERTION FAILED " << __FILE__ \
                << ":" << __LINE__ << " - " \
                << "Value " << (value) \
                << " was out of range [0", << (range) \
                << ")." << std::endl;\
      abort();\
    }\
  } while (0)

# define EL_ASSERT_STRING_EMPTY(value)\
  EL_ASSERT((value).Length() > 0, "String " #value " cannot be empty.")
#else
# define EL_ASSERT(condition, message)  ((void)0)
# define EL_ASSERT_NIL(value)           ((void)0)
# define EL_ASSERT_RANGE(value, range)  ((void)0)
# define EL_ASSERT_STRING_EMPTY(value)  ((void)0)
#endif

typedef int StringId;
#define EL_NOSTRING   (-1)

#include "el_uncopyable.h"

#endif  // __EL_BASE_HEADER_H__
