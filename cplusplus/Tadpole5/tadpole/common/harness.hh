// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
#include "common.hh"
#include "colorful.hh"

namespace tadpole::harness {

class Tester final : private UnCopyable {
  bool ok_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept : fname_(fname), lineno_(lineno) {}

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr
        << colorful::fg::red
        << fname_ << "(" << lineno_ << "): " << ss_.str()
        << colorful::reset << std::endl;
      std::exit(-1);
    }
  }

  inline Tester& is_true(bool cond, strv_t message) noexcept {
    if (!cond) {
      ss_ << " Assertion failure: " << message;
      ok_ = false;
    }
    return *this;
  }

#define _HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y> inline Tester& Name(const X& x, const Y& y) noexcept {\
    if (!(x Op y)) {\
      ss_ << " failed: " << x << (" " #Op " ") << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  _HARNESS_BINOP(is_eq, ==)
  _HARNESS_BINOP(is_ne, !=)
  _HARNESS_BINOP(is_gt, >)
  _HARNESS_BINOP(is_ge, >=)
  _HARNESS_BINOP(is_lt, <)
  _HARNESS_BINOP(is_le, <=)
#undef _HARNESS_BINOP

  template <typename T> inline Tester& operator<<(const T& x) noexcept {
    if (!ok_)
      ss_ << " " << x;
    return *this;
  }
};

using ClosureFn = std::function<void ()>;

bool register_harness(strv_t name, ClosureFn&& fn);
int run_all_harness();

}

#define TADPOLE_CHECK_TRUE(c)   tadpole::harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define TADPOLE_CHECK_EQ(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define TADPOLE_CHECK_NE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define TADPOLE_CHECK_GT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define TADPOLE_CHECK_GE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define TADPOLE_CHECK_LT(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_lt((a), (b))
#define TADPOLE_CHECK_LE(a, b)  tadpole::harness::Tester(__FILE__, __LINE__).is_le((a), (b))

#if defined(_TADPOLE_RUN_HARNESS)
# define _TADPOLE_IGNORED_REG(Name, Fn)\
  bool _Ignored_TadpoleHarness_##Name = tadpole::harness::register_harness(#Name, Fn);
#else
# define _TADPOLE_IGNORED_REG(Name, Fn)
#endif

#define TADPOLE_TEST(Name)\
class TadpoleHarness_##Name final : private tadpole::UnCopyable {\
  void _run();\
public:\
  static void run_harness() {\
    static TadpoleHarness_##Name _ins;\
    _ins._run();\
  }\
};\
_TADPOLE_IGNORED_REG(Name, &TadpoleHarness_##Name::run_harness)\
void TadpoleHarness_##Name::_run()
