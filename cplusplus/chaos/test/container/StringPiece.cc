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
#include <chaos/container/StringPiece.h>
#include <chaos/unittest/TestHarness.h>

CHAOS_TEST(StringArg, chaos::FakeTester) {
  const char* raw_str = "chaos::StringArg tester with raw string";
  std::string cpp_str(raw_str);
  chaos::StringArg s0(raw_str);
  chaos::StringArg s1(cpp_str);
  chaos::StringArg s2 = raw_str;

  CHAOS_CHECK_EQ(s0.c_str(), raw_str);
  CHAOS_CHECK_EQ(strlen(s0.c_str()), strlen(raw_str));
  CHAOS_CHECK_EQ(s1.c_str(), cpp_str.data());
  CHAOS_CHECK_EQ(strlen(s1.c_str()), cpp_str.size());
  CHAOS_CHECK_EQ(s2.c_str(), s0.c_str());
}

CHAOS_TEST(StringPiece, chaos::FakeTester) {
}
