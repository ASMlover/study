// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_UNITTEST_TESTHARNESS_H
#define CHAOS_UNITTEST_TESTHARNESS_H

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <Chaos/IO/ColorIO.h>

namespace Chaos {

class FakeTester {};

class Tester {
  bool ok_;
  const char* fname_;
  int lineno_;
  std::stringstream ss_;
public:
  Tester(const char* fname, int lineno)
    : ok_(true)
    , fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester(void) {
    if (!ok_) {
      ColorIO::fprintf(stderr, ColorIO::ColorType::COLORTYPE_RED,
          "%s:%d - %s\n", fname_, lineno_, ss_.str().c_str());
      exit(1);
    }
  }

  Tester& is_true(bool b, const char* msg) {
    if (!b) {
      ss_ << " Assection failure: " << msg;
      ok_ = false;
    }
    return *this;
  }

#define CHAOS_BINARY_OPERATOR(name, op)\
  template <typename X, typename Y>\
  Tester& name(const X& x, const Y& y) {\
    if (!(x op y)) {\
      ss_ << " failed: " << x << (" " #op " " ) << y;\
      ok_ = false;\
    }\
    return *this;\
  }

  CHAOS_BINARY_OPERATOR(is_eq, ==)
  CHAOS_BINARY_OPERATOR(is_ne, !=)
  CHAOS_BINARY_OPERATOR(is_ge, >=)
  CHAOS_BINARY_OPERATOR(is_gt, >)
  CHAOS_BINARY_OPERATOR(is_le, <=)
  CHAOS_BINARY_OPERATOR(is_lt, <)
#undef CHAOS_BINARY_OPERATOR

  template <typename V>
  Tester& operator<<(const V& value) {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

#define CHAOS_CHECK_TRUE(c)   Chaos::Tester(__FILE__, __LINE__).is_true((c), #c)
#define CHAOS_CHECK_EQ(a, b)  Chaos::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define CHAOS_CHECK_NE(a, b)  Chaos::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define CHAOS_CHECK_GE(a, b)  Chaos::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define CHAOS_CHECK_GT(a, b)  Chaos::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define CHAOS_CHECK_LE(a, b)  Chaos::Tester(__FILE__, __LINE__).is_le((a), (b))
#define CHAOS_CHECK_LT(a, b)  Chaos::Tester(__FILE__, __LINE__).is_lt((a), (b))

bool register_testharness(const char* base, const char* name, void (*closure)(void));
int run_all_testharness(void);

}

#define CHAOS_TEST(Name, Base)\
class _ChaosTestHarness_##Name : public Base {\
public:\
  void _run(void);\
  static void _run_harness(void) {\
    _ChaosTestHarness_##Name t;\
    t._run();\
  }\
};\
bool _ChaosTestHarness_ignored_##Name =\
  Chaos::register_testharness(#Base, #Name, &_ChaosTestHarness_##Name::_run_harness);\
void _ChaosTestHarness_##Name::_run(void)

#endif // CHAOS_UNITTEST_TESTHARNESS_H
