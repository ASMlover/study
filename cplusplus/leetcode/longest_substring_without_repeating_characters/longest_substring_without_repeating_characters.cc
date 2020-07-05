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
#include <string>
#include <unordered_map>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode-cn.com/problems/longest-substring-without-repeating-characters/
//
// 说明:
//
// 给定一个字符串，请你找出其中不含有重复字符的 最长子串 的长度。
//
// 示例 1:
// 输入: "abcabcbb"
// 输出: 3
// 解释: 因为无重复字符的最长子串是 "abc"，所以其长度为 3。
// 示例 2:
//
// 输入: "bbbbb"
// 输出: 1
// 解释: 因为无重复字符的最长子串是 "b"，所以其长度为 1。
// 示例 3:
//
// 输入: "pwwkew"
// 输出: 3
// 解释: 因为无重复字符的最长子串是 "wke"，所以其长度为 3。
//       请注意，你的答案必须是 子串 的长度，"pwke" 是一个子序列，不是子串。

class LongestSubstringWithoutRepeatingCharacters final
  : public Singleton<LongestSubstringWithoutRepeatingCharacters> {
  int get_result(const std::string& s) {
    int max_len = 0;
    int last_repeat_pos = -1;
    std::unordered_map<char, int> m;

    for (int i = 0; i < s.size(); ++i) {
      auto c = s[i];
      if (auto it = m.find(c); it != m.end() && last_repeat_pos < it->second)
        last_repeat_pos = it->second;

      if (i - last_repeat_pos > max_len)
        max_len = i - last_repeat_pos;

      m[c] = i;
    }

    return max_len;
  }
public:
  void run() {
    HARNESS_EQ(get_result("abcabcbb"), 3);
    HARNESS_EQ(get_result("bbbbbbbb"), 1);
    HARNESS_EQ(get_result("pwwkewww"), 3);
  }
};

HARNESS_TEST(LongestSubstringWithoutRepeatingCharacters, harness::FakeTester) {
  LongestSubstringWithoutRepeatingCharacters::get_instance().run();
}
