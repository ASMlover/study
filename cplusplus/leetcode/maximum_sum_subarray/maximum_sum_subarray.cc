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
#include <algorithm>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode.com/problems/maximum-subarray/
//
// Given an integer array nums, find the contiguous subarray (containing at
// least one number) which has the largest sum and return its sum.
//
// Example:
//
// Input: [-2,1,-3,4,-1,2,1,-5,4],
// Output: 6
// Explanation: [4,-1,2,1] has the largest sum = 6.
// Follow up:
//
// If you have figured out the O(n) solution, try coding another solution using
// the divide and conquer approach, which is more subtle.

class MaximumSumSubarrary final : public Singleton<MaximumSumSubarrary> {
  int calc_max(const std::vector<int>& nums) {
    if (nums.empty())
      return 0;

    std::size_t size = nums.size();
    int curr_sum = nums[0];
    int sum = nums[0];
    for (std::size_t i = 1; i < size; ++i) {
      curr_sum = std::max(curr_sum + nums[i], nums[i]);
      sum = std::max(curr_sum, sum);
    }
    return sum;
  }
public:
  void run() {
    HARNESS_EQ(calc_max({-2,1,-3,4,-1,2,1,-5,4}), 6);
  }
};

HARNESS_TEST(MaximumSumSubarrary, harness::FakeTester) {
  MaximumSumSubarrary::get_instance().run();
}
