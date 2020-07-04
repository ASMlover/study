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
#include <list>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode-cn.com/problems/add-two-numbers/?utm_source=LCUS&utm_medium=ip_redirect_q_uns&utm_campaign=transfer2china
//
// 说明:
//
// 给出两个 非空 的链表用来表示两个非负的整数。其中，它们各自的位数是按照 逆序 的方式存储的，
// 并且它们的每个节点只能存储 一位 数字。
//
// 如果，我们将这两个数相加起来，则会返回一个新的链表来表示它们的和。
// 您可以假设除了数字 0 之外，这两个数都不会以 0 开头。
//
// 示例:
//
// 输入：(2 -> 4 -> 3) + (5 -> 6 -> 4)
// 输出：7 -> 0 -> 8
// 原因：342 + 465 = 807

class AddTwoNumbers final : public Singleton<AddTwoNumbers> {
  std::list<int> get_result(const std::list<int>& a, const std::list<int>& b) {
    std::list<int> result;

    int carry = 0;
    auto a_iter = a.begin();
    auto b_iter = b.begin();
    while (a_iter != a.end() || b_iter != b.end()) {
      int x = 0, y = 0;
      if (a_iter != a.end())
        x = *a_iter++;
      if (b_iter != b.end())
        y = *b_iter++;

      auto sum = carry + x + y;
      result.push_back(sum % 10);
      carry = sum / 10;
    }

    return result;
  }
public:
  void run() {
    std::list<int> a{2, 4, 3};
    std::list<int> b{5, 6, 4};

    auto result = get_result(a, b);

    bool is_first = true;
    std::cout << "AddTwoNumbers: ";
    for (auto r : result) {
      if (!is_first)
        std::cout << " -> ";
      std::cout << "[" << r << "]";

      if (is_first)
        is_first = false;
    }
    std::cout << std::endl;
  }
};

HARNESS_TEST(AddTwoNumbers, harness::FakeTester) {
  AddTwoNumbers::get_instance().run();
}
