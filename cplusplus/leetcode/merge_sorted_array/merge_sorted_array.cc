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
#include <iostream>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode-cn.com/problems/merge-sorted-array/
//
// 给定两个有序整数数组 nums1 和 nums2，将 nums2 合并到 nums1 中，使得 num1
// 成为一个有序数组。
//
// 说明:
//
// 初始化 nums1 和 nums2 的元素数量分别为 m 和 n。
// 你可以假设 nums1 有足够的空间（空间大小大于或等于 m + n）来保存 nums2 中的元素。
// 示例:
//
// 输入:
// nums1 = [1,2,3,0,0,0], m = 3
// nums2 = [2,5,6],       n = 3
//
// 输出: [1,2,2,3,5,6]

class MergeSortedArray final : public Singleton<MergeSortedArray> {
  void merge(int* a1, int n1, int* a2, int n2) {
    int l1 = n1 - 1;
    int l2 = n2 - 1;
    for (int i = n1 + n2 - 1; i >= 0; --i) {
      if (l1 >= 0 && l2 < 0)
        break;

      if (l1 < 0 && l2 >= 0) {
        a1[i] = a2[l2--];
        continue;
      }
      if (l1 >= 0 && l2 >= 0) {
        if (a1[l1] > a2[l2])
          a1[i] = a1[l1--];
        else
          a1[i] = a2[l2--];
      }
    }
  }
public:
  void run() {
    int a1[] = {1, 2, 3, 0, 0, 0};
    int a2[] = {2, 5, 6};

    merge(a1, 3, a2, 3);
    HARNESS_EQ(a1[0], 1);
    HARNESS_EQ(a1[1], 2);
    HARNESS_EQ(a1[2], 2);
    HARNESS_EQ(a1[3], 3);
    HARNESS_EQ(a1[4], 5);
    HARNESS_EQ(a1[5], 6);
  }
};

HARNESS_TEST(MergeSortedArray, harness::FakeTester) {
  MergeSortedArray::get_instance().run();
}
