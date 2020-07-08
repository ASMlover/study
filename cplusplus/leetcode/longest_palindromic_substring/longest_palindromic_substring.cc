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
#include <tuple>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode-cn.com/problems/longest-palindromic-substring/
//
// 说明：
// 给定一个字符串 s，找到 s 中最长的回文子串。你可以假设 s 的最大长度为 1000。
//
// 示例 1：
//
// 输入: "babad"
// 输出: "bab"
// 注意: "aba" 也是一个有效答案。
// 示例 2：
//
// 输入: "cbbd"
// 输出: "bb"

class LongestPalindromicSubstring final : public Singleton<LongestPalindromicSubstring> {
  std::tuple<int, int> find_palindrome(const std::string& s, int left, int right, int beg, int len) {
    int n = static_cast<int>(s.size());
    while (left >= 0 && right < n && s[left] == s[right]) {
      --left;
      ++right;
    }
    if (right - left - 1 > len) {
      len = right - left - 1;
      beg = left + 1;
    }
    return std::make_tuple(beg, len);
  }

  std::string get_result(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n <= 1)
      return s;

    int beg = 0;
    int len = 0;
    for (int i = 0; i < n - 1; ++i) {
      auto [start, count] = find_palindrome(s, i, i, beg, len);
      auto [start1, count1] = find_palindrome(s, i, i + 1, start, count);
      beg = start1;
      len = count1;
    }
    return s.substr(beg, len);
  }
public:
  void run() {
    HARNESS_EQ(get_result("babad"), "bab");
    HARNESS_EQ(get_result("cbbd"), "bb");
  }
};

HARNESS_TEST(LongestPalindromicSubstring, harness::FakeTester) {
  LongestPalindromicSubstring::get_instance().run();
}
