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
#include <string_view>
#include <sstream>

namespace harness {

class FakeTester {};

class Tester final {
  bool ok_{true};
  std::string_view fname_{};
  int lineno_{};
  std::stringstream ss_;
public:
  Tester(std::string_view fname, int lineno) noexcept
    : fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester() noexcept {
    if (!ok_) {
      std::cerr << fname_ << ":" << lineno_ << " - " << ss_.str() << std::endl;
      std::exit(1);
    }
  }

  Tester& is_true(bool b, std::string_view msg) noexcept {
    if (!b) {
      ss_ << " Assection failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define HARNESS_BINOP(name, op)\
  template <typename X, typename Y>\
  Tester& name(const X& x, const Y& y) noexcept {\
    if (!(x op y)) {\
      ss_ << " failed: " << x << (" " #op " ") << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  HARNESS_BINOP(is_eq, ==)
  HARNESS_BINOP(is_ne, !=)
  HARNESS_BINOP(is_ge, >=)
  HARNESS_BINOP(is_gt, >)
  HARNESS_BINOP(is_le, <=)
  HARNESS_BINOP(is_lt, <)
#undef HARNESS_BINOP

  template <typename V> Tester& operator<<(const V& value) noexcept {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

using ClosureFn = std::function<void ()>;

bool register_harness(
    std::string_view base, std::string_view name, ClosureFn&& closure);
int run_all_harness();

}

#define HARNESS_TRUE(c)   harness::Tester(__FILE__, __LINE__).is_true((c), #c)
#define HARNESS_EQ(a, b)  harness::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define HARNESS_NE(a, b)  harness::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define HARNESS_GE(a, b)  harness::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define HARNESS_GT(a, b)  harness::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define HARNESS_LE(a, b)  harness::Tester(__FILE__, __LINE__).is_le((a), (b))
#define HARNESS_LT(a, b)  harness::Tester(__FILE__, __LINE__).is_lt((a), (b))

#define HARNESS_TEST(Name, Base)\
class _Harness_##Name : public Base {\
public:\
  void _run();\
  static void _run_harness() {\
    _Harness_##Name t;\
    t._run();\
  }\
};\
bool _Harness_ignored_##Name =\
harness::register_harness(#Base, #Name, &_Harness_##Name::_run_harness);\
void _Harness_##Name::_run()

