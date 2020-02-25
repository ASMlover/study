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
#include "common.hh"

namespace wrencc {

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
      std::exit(1);
    }
  }

  Tester& is_true(bool b, str_t msg) noexcept {
    if (!b) {
      ss_ << " Assection failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define WRENCC_HARNESS_BINOP(name, op)\
  template <typename X, typename Y>\
  Tester& name(const X& x, const Y& y) noexcept {\
    if (!(x op y)) {\
      ss_ << " failed: " << x << (" " #op " ") << y << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  WRENCC_HARNESS_BINOP(is_eq, ==)
  WRENCC_HARNESS_BINOP(is_ne, !=)
  WRENCC_HARNESS_BINOP(is_ge, >=)
  WRENCC_HARNESS_BINOP(is_gt, >)
  WRENCC_HARNESS_BINOP(is_le, <=)
  WRENCC_HARNESS_BINOP(is_lt, <)
#undef WRENCC_HARNESS_BINOP

  template <typename V>
  Tester& operator<<(const V& value) noexcept {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

using ClosureFn = std::function<void ()>;

bool register_harness(strv_t base, strv_t name, ClosureFn&& closure);
int run_all_harness();

}

#define WRENCC_CHECK_TRUE(c)  wrencc::Tester(__FILE__, __LINE__).is_true((c), #c)
#define WRENCC_CHECK_EQ(a, b) wrencc::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define WRENCC_CHECK_NE(a, b) wrencc::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define WRENCC_CHECK_GE(a, b) wrencc::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define WRENCC_CHECK_GT(a, b) wrencc::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define WRENCC_CHECK_LE(a, b) wrencc::Tester(__FILE__, __LINE__).is_le((a), (b))
#define WRENCC_CHECK_LT(a, b) wrencc::Tester(__FILE__, __LINE__).is_lt((a), (b))

#define WRENCC_TEST(Name, Base)\
class _WrenccHarness_##Name : public Base {\
public:\
  void _run();\
  static void _run_harness() {\
    _WrenccHarness_##Name t;\
    t._run();\
  }\
};\
bool _WrenccHarness_ignored_##Name =\
  wrencc::register_harness(#Base, #Name, &_WrenccHarness_##Name::_run_harness);\
void _WrenccHarness_##Name::_run()
