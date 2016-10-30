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
#include "../basic/TStringPiece.h"
#include "../basic/TUnitTest.h"

TYR_TEST(StringPiece, tyr::basic::TestDummy) {
  const char* raw_str = "Hello, world!";
  size_t raw_len = strlen(raw_str);
  tyr::basic::StringPiece s1(raw_str);

  ASSERT_EQ(static_cast<bool>(s1), true);
  ASSERT_EQ(raw_len, s1.size());
  ASSERT_EQ(raw_str, s1.begin());
  ASSERT_EQ(raw_str + raw_len, s1.end());

  s1.remove_prefix(1);
  ASSERT_NE(raw_str, s1.begin());
  s1.remove_suffix(1);
  ASSERT_NE(raw_str + raw_len, s1.end());

  const char* raw_str2 = "hello, world!";
  size_t raw_len2 = strlen(raw_str2);
  s1.set(raw_str, raw_len);
  tyr::basic::StringPiece s2(raw_str2, raw_len2);

  ASSERT_EQ(-1, s1.compare(s2));
  ASSERT_EQ(0, s1.compare(s1));
  ASSERT_NE(s1.compare(s2), 0);
  ASSERT_GE(s2.compare(s1), 0);
  ASSERT_GT(s2.compare(s1), 0);
  ASSERT_LE(s1.compare(s2), 0);
  ASSERT_LT(s1.compare(s2), 0);

  ASSERT_TRUE(s1 == s1);
  ASSERT_TRUE(s2 == s2);
  ASSERT_TRUE(s1 != s2);
  ASSERT_TRUE(s2 != s1);
  ASSERT_TRUE(s1 < s2);
  ASSERT_TRUE(s1 <= s2);
  ASSERT_TRUE(s2 > s1);
  ASSERT_TRUE(s2 >= s1);
}
