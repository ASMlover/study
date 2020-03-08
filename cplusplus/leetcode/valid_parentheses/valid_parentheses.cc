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
#include <stack>
#include <string_view>
#include "common.hh"
#include "harness.hh"

// [ADDR] https://leetcode.com/problems/valid-parentheses/description
//
// Given a string containing just the characters '(', ')', '{', '}', '[' and ']',
// determine if the input string is valid.
//
// An input string is valid if:
//
// Open brackets must be closed by the same type of brackets.
// Open brackets must be closed in the correct order.
// Note that an empty string is also considered valid.
//
// Example 1:
//
// Input: "()"
// Output: true
// Example 2:
//
// Input: "()[]{}"
// Output: true
// Example 3:
//
// Input: "(]"
// Output: false
// Example 4:
//
// Input: "([)]"
// Output: false
// Example 5:
//
// Input: "{[]}"
// Output: true

class ValidParentheses final : public Singleton<ValidParentheses> {
  inline bool is_lparen(char c) const noexcept {
    return c == '(' || c == '[' || c == '{';
  }

  inline bool is_rparen(char c) const noexcept {
    return c == ')' || c == ']' || c == '}';
  }

  bool is_match(char c, char expected) const noexcept {
    switch (c) {
    case ')': return expected == '(';
    case ']': return expected == '[';
    case '}': return expected == '{';
    }
    return false;
  }

  bool is_valid(std::string_view s) const {
    std::stack<char> stack;
    for (char c : s) {
      if (is_lparen(c)) {
        stack.push(c);
      }
      else if (is_rparen(c)) {
        char top = stack.top();
        stack.pop();
        if (!is_match(c, top))
          return false;
      }
      else {
        return false;
      }
    }

    return stack.empty();
  }
public:
  void run() {
    HARNESS_TRUE(is_valid("()"));
    HARNESS_TRUE(is_valid("()[]{}"));
    HARNESS_TRUE(!is_valid("(]"));
    HARNESS_TRUE(!is_valid("([)]"));
    HARNESS_TRUE(is_valid("{[]}"));
  }
};

HARNESS_TEST(ValidParentheses, harness::FakeTester) {
  ValidParentheses::get_instance().run();
}
