// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <Core/MEvolve.hh>

namespace _mevo {

class FakeTester {};

class Tester final : public Copyable {
  bool ok_{true};
  strv_t fname_{};
  int lineno_{};
  ss_t ss_;
public:
  Tester(strv_t fname, int lineno) noexcept
    : fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr << fname_ << ":" << lineno_ << " - " << ss_.str() << std::endl;
      std::exit(-1);
    }
  }

  Tester& is_true(bool cond, strv_t msg) noexcept {
    if (!cond) {
      ss_ << " Assertion failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define _MEVO_HARNESS_BINOP(Name, Op)\
  template <typename X, typename Y>\
  Tester& Name(const X& x, const Y& y) noexcept {\
    if (!(x Op y)) {\
      ss_ << " failed: " << x << (" " #Op " ") << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  _MEVO_HARNESS_BINOP(is_eq, ==)
  _MEVO_HARNESS_BINOP(is_ne, !=)
  _MEVO_HARNESS_BINOP(is_ge, >=)
  _MEVO_HARNESS_BINOP(is_gt, >)
  _MEVO_HARNESS_BINOP(is_le, <=)
  _MEVO_HARNESS_BINOP(is_lt, <)
#undef _MEVO_HARNESS_BINOP

  template <typename Value>
  inline Tester& operator<<(const Value& value) noexcept {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

using HarnessClosure = std::function<void ()>;
bool register_harness(strv_t base, strv_t name, HarnessClosure&& closure);
int run_all_harness();

}

#define _MEVO_CHECK_TRUE(c)   _mevo::Tester(__FILE__, __LINE__).is_true((c), #c)
#define _MEVO_CHECK_EQ(a, b)  _mevo::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define _MEVO_CHECK_NE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define _MEVO_CHECK_GE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define _MEVO_CHECK_GT(a, b)  _mevo::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define _MEVO_CHECK_LE(a, b)  _mevo::Tester(__FILE__, __LINE__).is_le((a), (b))
#define _MEVO_CHECK_LT(a, b)  _mevo::Tester(__FILE__, __LINE__).is_lt((a), (b))

#define _MEVO_TEST(Name, Base)\
class _MevoHarness_##Name final : public Base {\
  void _run();\
public:\
  static void _run_harness() {\
    _MevoHarness_##Name _ins;\
    _ins._run();\
  }\
};\
bool _Ignored_MevoHarness_##Name =\
  _mevo::register_harness(#Base, #Name, &_MevoHarness_##Name::_run_harness);\
void _MevoHarness_##Name::_run()
