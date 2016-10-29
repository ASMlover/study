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
#include <memory>
#include <vector>
#include "TUnitTest.h"

namespace tyr { namespace basic {

struct Test {
  const char* __base;
  const char* __name;
  void (*__closure)(void);

  Test(const char* base, const char* name, void (*closure)(void))
    : __base(base)
    , __name(name)
    , __closure(closure) {
  }
};
typedef std::vector<Test> TestVector;
std::unique_ptr<TestVector> g_tests;

bool register_unittest(const char* base, const char* name, void (*closure)(void)) {
  if (!g_tests)
    g_tests.reset(new TestVector);

  Test t(base, name, closure);
  g_tests->push_back(t);

  return true;
}

int run_all_tests(void) {
  int num_tests = 0;
  int num_passed = 0;
  if (g_tests && !g_tests->empty()) {
    num_tests = static_cast<int>(g_tests->size());

    for (auto& t : *g_tests) {
      t.__closure();
      ++num_passed;
    }
  }
  fprintf(stderr, "========== PASSED (%d/%d) unit tests\n", num_passed, num_tests);
  return 0;
}

}}
