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
#include <Chaos/Container/StringPiece.h>
#include <Chaos/Unittest/TestHarness.h>

CHAOS_TEST(StringArg, Chaos::FakeTester) {
  const char* raw_str = "Chaos::StringArg tester with raw string";
  std::string cpp_str(raw_str);
  Chaos::StringArg s0(raw_str);
  Chaos::StringArg s1(cpp_str);
  Chaos::StringArg s2 = raw_str;

  CHAOS_CHECK_EQ(s0.c_str(), raw_str);
  CHAOS_CHECK_EQ(strlen(s0.c_str()), strlen(raw_str));
  CHAOS_CHECK_EQ(s1.c_str(), cpp_str.data());
  CHAOS_CHECK_EQ(strlen(s1.c_str()), cpp_str.size());
  CHAOS_CHECK_EQ(s2.c_str(), s0.c_str());
}

CHAOS_TEST(StringPiece, Chaos::FakeTester) {
  const char* raw_str = "Chaos::StringPiece tester with raw string";
  std::string cpp_str("Chaos::StringPiece tester with C++ std::string");

  Chaos::StringPiece s0;
  CHAOS_CHECK_TRUE(!static_cast<bool>(s0));
  CHAOS_CHECK_TRUE(s0.data() == nullptr);
  CHAOS_CHECK_TRUE(s0.size() == 0);
  CHAOS_CHECK_TRUE(s0.empty());
  CHAOS_CHECK_TRUE(s0.begin() == nullptr);
  CHAOS_CHECK_TRUE(s0.end() == nullptr);

  Chaos::StringPiece s1(raw_str);
  CHAOS_CHECK_TRUE(static_cast<bool>(s1));
  CHAOS_CHECK_EQ(s1.data(), raw_str);
  CHAOS_CHECK_EQ(s1.size(), strlen(raw_str));
  CHAOS_CHECK_TRUE(!s1.empty());
  CHAOS_CHECK_TRUE(s1.begin() == raw_str);
  CHAOS_CHECK_TRUE(s1.end() == raw_str + s1.size());
  size_t size = s1.size();
  s1.remove_prefix(2);
  size -= 2;
  CHAOS_CHECK_EQ(s1.size(), size);
  CHAOS_CHECK_EQ(s1.begin(), raw_str + 2);
  s1.remove_suffix(2);
  size -= 2;
  CHAOS_CHECK_EQ(s1.size(), size);
  CHAOS_CHECK_EQ(s1.end(), raw_str + strlen(raw_str) - 2);
  s1.clear();
  CHAOS_CHECK_TRUE(s1.empty());

  Chaos::StringPiece s2(cpp_str);
  CHAOS_CHECK_TRUE(static_cast<bool>(s2));
  CHAOS_CHECK_EQ(s2.data(), cpp_str.data());
  CHAOS_CHECK_EQ(s2.size(), cpp_str.size());
  CHAOS_CHECK_TRUE(!s2.empty());
  CHAOS_CHECK_TRUE(s2.begin() == cpp_str.c_str());
  CHAOS_CHECK_TRUE(s2.end() == cpp_str.c_str() + cpp_str.size());
  s2.remove_prefix(2);
  CHAOS_CHECK_EQ(s2.size(), cpp_str.size() - 2);
  CHAOS_CHECK_EQ(s2.begin(), cpp_str.data() + 2);
  s2.remove_suffix(2);
  CHAOS_CHECK_EQ(s2.size(), cpp_str.size() - 4);
  CHAOS_CHECK_EQ(s2.end(), cpp_str.data() + cpp_str.size() - 2);
  s2.clear();
  CHAOS_CHECK_TRUE(s2.empty());

  Chaos::StringPiece s;
  CHAOS_CHECK_TRUE(s.empty());
  s.set(raw_str);
  CHAOS_CHECK_TRUE(!s.empty());
  CHAOS_CHECK_TRUE(s.size() == strlen(raw_str));
  s.set(cpp_str.c_str(), cpp_str.size());
  CHAOS_CHECK_TRUE(s.size() == cpp_str.size());
  CHAOS_CHECK_TRUE(s.starts_with("Chaos"));

  Chaos::StringPiece a("AAA");
  Chaos::StringPiece b("BBB");
  CHAOS_CHECK_TRUE(a < b);
  CHAOS_CHECK_TRUE(a <= a);
  CHAOS_CHECK_TRUE(a <= b);
  CHAOS_CHECK_TRUE(b > a);
  CHAOS_CHECK_TRUE(b >= b);
  CHAOS_CHECK_TRUE(b >= a);
}
