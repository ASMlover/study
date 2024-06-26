// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#pragma once

#include <functional>
#include <iostream>
#include <Common/Common.hh>
#include <Common/Colorful.hh>

namespace Tadpole::Common::Harness {

class Tester final : private UnCopyable {
  bool is_success_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept : fname_(fname), lineno_(lineno) {}

  ~Tester() noexcept {
    if (!is_success_) {
      std::cerr
        << Colorful::fg::red << fname_ << "(" << lineno_ << "): " << ss_.str()
        << Colorful::reset << std::endl;
      std::exit(-1);
    }
  }

  inline Tester& is_true(bool cond, strv_t message) noexcept {
    if (!cond) {
      ss_ << " Assertion failure: " << message;
      is_success_ = false;
    }
    return *this;
  }

#define HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y> inline Tester& Name(const X& x, const Y& y) noexcept {\
    if (!(x Op y)) {\
      ss_ << " failed: " << x << (" " #Op " ") << y;\
      is_success_ = false;\
    }\
    return *this;\
  }

  HARNESS_BINOP(is_eq, ==)
  HARNESS_BINOP(is_ne, !=)
  HARNESS_BINOP(is_gt, >)
  HARNESS_BINOP(is_ge, >=)
  HARNESS_BINOP(is_lt, <)
  HARNESS_BINOP(is_le, <=)
#undef HARNESS_BINOP

  template <typename T> inline Tester& operator<<(const T& x) noexcept {
    if (!is_success_)
      ss_ << " " << x;
    return *this;
  }
};

using ClosureFn = std::function<void ()>;

bool register_harness(strv_t name, ClosureFn&& fn);
int run_all_harness();
int run_harness_with(strv_t name);

}

#define TADPOLE_CHECK_TRUE(c)   Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define TADPOLE_CHECK_EQ(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define TADPOLE_CHECK_NE(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define TADPOLE_CHECK_GT(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define TADPOLE_CHECK_GE(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define TADPOLE_CHECK_LT(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_lt((a), (b))
#define TADPOLE_CHECK_LE(a, b)  Tadpole::Common::Harness::Tester(__FILE__, __LINE__).is_le((a), (b))

#if defined(_TADPOLE_RUN_HARNESS)
# define _TADPOLE_IGNORED_REG(Name, Fn)\
  bool _Ignored_TadpoleCommonHarness_##Name = Tadpole::Common::Harness::register_harness(#Name, Fn);
#else
# define _TADPOLE_IGNORED_REG(Name, Fn)
#endif

#define TADPOLE_TEST(Name)\
class TadpoleCommonHarness_##Name final : private Tadpole::Common::UnCopyable {\
  void _run();\
public:\
  static void run_harness() {\
    static TadpoleCommonHarness_##Name ins;\
    ins._run();\
  }\
};\
_TADPOLE_IGNORED_REG(Name, &TadpoleCommonHarness_##Name::run_harness)\
void TadpoleCommonHarness_##Name::_run()
