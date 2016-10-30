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
#ifndef __TYR_BASIC_UNITTEST_HEADER_H__
#define __TYR_BASIC_UNITTEST_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

namespace tyr { namespace basic {

class TestDummy {};

class Tester {
  bool ok_{false};
  const char* fname_{nullptr};
  int lineno_{0};
  std::stringstream ss_;
public:
  Tester(const char* fname, int lineno)
    : ok_(true)
    , fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester(void) {
    if (!ok_) {
      fprintf(stderr, "%s:%d - %s\n", fname_, lineno_, ss_.str().c_str());
      exit(1);
    }
  }

  Tester& is_true(bool b, const char* msg) {
    if (!b) {
      ss_ << " Assertion failure " << msg;
      ok_ = false;
    }
    return *this;
  }

#define BINARY_OPERATION(name, op)\
  template <typename X, typename Y>\
  Tester& name(const X& x, const Y& y) {\
    if (!(x op y)) {\
      ss_ << " failed: " << x << " " #op " " << y;\
      ok_ = false;\
    }\
    return *this;\
  }
  BINARY_OPERATION(is_eq, ==)
  BINARY_OPERATION(is_ne, !=)
  BINARY_OPERATION(is_ge, >=)
  BINARY_OPERATION(is_gt, >)
  BINARY_OPERATION(is_le, <=)
  BINARY_OPERATION(is_lt, <)
#undef BINARY_OPERATION

  template <typename T>
  Tester& operator<<(const T& value) {
    if (!ok_)
      ss_ << " " << value;
    return *this;
  }
};

#define ASSERT_TRUE(c)  tyr::basic::Tester(__FILE__, __LINE__).is_true((c), #c)
#define ASSERT_EQ(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define ASSERT_NE(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define ASSERT_GE(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define ASSERT_GT(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define ASSERT_LE(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_le((a), (b))
#define ASSERT_LT(a, b) tyr::basic::Tester(__FILE__, __LINE__).is_lt((a), (b))

extern bool register_unittest(const char* base, const char* name, void (*closure)(void));
extern int run_all_tests(void);

}}

#define TYR_TEST(name, base)\
class _UnitTest_##name : public base {\
public:\
  void _run(void);\
  static void _run_unit(void) {\
    _UnitTest_##name t;\
    t._run();\
  }\
};\
bool _UnitTest_ignored_##name = \
  tyr::basic::register_unittest(#base, #name, &_UnitTest_##name::_run_unit);\
void _UnitTest_##name::_run(void)

#endif // __TYR_BASIC_UNITTEST_HEADER_H__
