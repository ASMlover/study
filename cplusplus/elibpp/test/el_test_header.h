//! Copyright (c) 2013 ASMlover. All rights reserved.
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
#ifndef __EL_TEST_HEADER_H__
#define __EL_TEST_HEADER_H__

#include "../elib_internal.h"

#if defined(PLATFORM_WIN)
# define inline       __inline
# define __func__     __FUNCTION__
# define el_sleep(x)  Sleep((x))
# define SelfThreadId GetCurrentThreadId
#elif defined(PLATFORM_LINUX)
# define el_sleep(x)  usleep((x) * 1000)
# define SelfThreadId pthread_self
#endif


struct UnitCase {
  const char* unit_name;
  void (*unit_case)(void);

  UnitCase(const char* name, void (*unit)(void))
    : unit_name(name)
    , unit_case(unit)
  {
  }
};

class UnitFramework 
  : public el::Singleton<UnitFramework>, private el::NonCopyable {
  std::vector<UnitCase> unit_list_;
public:
  explicit UnitFramework(void);
  ~UnitFramework(void);

  int Run(void);
  bool RegisterUnit(const char* name, void (*unit)(void));
};

#define UNIT_RUN_ALL()    UnitFramework::Instance().Run()
#define UNIT_IMPL(name)\
static void el_Unit##name(void);\
static bool _s_##name = \
  UnitFramework::Instance().RegisterUnit(#name, el_Unit##name);\
static void el_Unit##name(void)


//! Have our own assert, so we are sure it dose not get 
//! optomized away in a release build.
#define UNIT_ASSERT(expr)\
do {\
  if (!(expr)) {\
    el::ColorFprintf(stderr, \
        el::kColorTypeRed, \
        "assertion failed in %s on line %d : %s\n", \
        __FILE__, \
        __LINE__, \
        #expr);\
    fflush(stderr);\
    abort();\
  }\
} while (0)


#endif  //! __EL_TEST_HEADER_H__
