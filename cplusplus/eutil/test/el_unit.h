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
#ifndef __EL_UNIT_HEADER_H__
#define __EL_UNIT_HEADER_H__


#include "../el_util.h"

#if defined(EUTIL_WIN)
# define __func__ __FUNCTION__
#endif

namespace el {

struct UnitCase {
  typedef std::function<void (void)> UnitType;

  const char*    unit_name;
  const UnitType unit_case;

  UnitCase(const char* name, const UnitType& unit) 
    : unit_name(name) 
    , unit_case(unit) {
  }
};


class UnitFramework : public Singleton<UnitFramework> {
  std::vector<UnitCase> unit_list_;
public:
  UnitFramework(void);
  ~UnitFramework(void);

  int Run(void);
  bool RegisterUnit(const char* name, const UnitCase::UnitType& unit);
};

extern int UnitShow(const char* format, ...);

}

#define UNIT_RUN_ALL()  el::UnitFramework::Instance().Run()
#define UNIT_IMPL(__name__)\
static void el_Unit##__name__(void);\
static bool s_boolean_##__name__ = \
  el::UnitFramework::Instance().RegisterUnit(#__name__, el_Unit##__name__);\
static void el_Unit##__name__(void)

#define UNIT_SHOW(fmt, ...) el::UnitShow((fmt), __VA_ARGS__)


#endif  // __EL_UNIT_HEADER_H__
