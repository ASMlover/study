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
#ifndef CO_UNITTEST_H_
#define CO_UNITTEST_H_

#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace co { namespace test {

class FakeTest {};

class Tester {
  bool passed_{false};
  const char* fname_{nullptr};
  int lineno_{0};
  std::stringstream ss_;
public:
  Tester(const char* fname, int lineno)
    : passed_(true)
    , fname_(fname)
    , lineno_(lineno) {
  }

  ~Tester(void) {
    if (!passed_) {
      fprintf(stderr, "%s:%d - %s\n", fname_, lineno_, ss_.str().c_str());
      exit(1);
    }
  }

  Tester& is_true(bool cond, const char* msg) {
    if (!cond) {
      ss_ << " Assertion failure " << msg;
      passed_ = false;
    }
    return *this;
  }

#define CO_BINARY_OPERATION(fn, op)\
  template <typename T, typename U>\
  Tester& fn(const T& a, const U& b) {\
    if (!(a op b)) {\
      ss_ << " failed: " << a << " " #op " " << b;\
      passed_ = false;\
    }\
    return *this;\
  }

  CO_BINARY_OPERATION(is_eq, ==)
  CO_BINARY_OPERATION(is_ne, !=)
  CO_BINARY_OPERATION(is_ge, >=)
  CO_BINARY_OPERATION(is_gt, >)
  CO_BINARY_OPERATION(is_le, <=)
  CO_BINARY_OPERATION(is_lt, <)
#undef CO_BINARY_OPERATION

  template <typename T>
  Tester& operator<<(const T& value) {
    if (!passed_)
      ss_ << " " << value;
    return *this;
  }
};

#define ASSERT_TRUE(c)  co::test::Tester(__FILE__, __LINE__).is_true((c), #c)
#define ASSERT_EQ(a, b) co::test::Tester(__FILE__, __LINE__).is_eq((a), (b))
#define ASSERT_NE(a, b) co::test::Tester(__FILE__, __LINE__).is_ne((a), (b))
#define ASSERT_GE(a, b) co::test::Tester(__FILE__, __LINE__).is_ge((a), (b))
#define ASSERT_GT(a, b) co::test::Tester(__FILE__, __LINE__).is_gt((a), (b))
#define ASSERT_LE(a, b) co::test::Tester(__FILE__, __LINE__).is_le((a), (b))
#define ASSERT_LT(a, b) co::test::Tester(__FILE__, __LINE__).is_lt((a), (b))

bool register_unittest(const char* base, const char* name, void (*fn)(void));
int run_all_unittests(void);

}}

#define CO_TEST(name, base)\
class _CoUnitTest_##name : public base {\
public:\
  void _run(void);\
  static void _run_unit(void) {\
    _CoUnitTest_##name t;\
    t._run();\
  }\
};\
bool _CoUnitTest_##name_ignored =\
  co::test::register_unittest(#base, #name, &_CoUnitTest_##name::_run_unit);\
void _CoUnitTest_##name::_run(void)

#endif // CO_UNITTEST_H_
