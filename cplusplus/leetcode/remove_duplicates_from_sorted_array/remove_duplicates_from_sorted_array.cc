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
#include <vector>
#include "../common.hh"
#include "../harness.hh"

// [ADDR] https://leetcode.com/problems/remove-duplicates-from-sorted-array/description/
//
// Given a sorted array nums, remove the duplicates in-place such that each
// element appear only once and return the new length.
//
// Do not allocate extra space for another array, you must do this by modifying
// the input array in-place with O(1) extra memory.
//
// Example 1:
// Given nums = [1,1,2],
// Your function should return length = 2, with the first two elements of nums
// being 1 and 2 respectively.
//
// It doesn't matter what you leave beyond the returned length. Example 2:
// Given nums = [0,0,1,1,1,2,2,3,3,4],
// Your function should return length = 5, with the first five elements of nums
// being modified to 0, 1, 2, 3, and 4 respectively.
//
// It doesn't matter what values are set beyond the returned length. Clarification:
// Confused why the returned value is an integer but your answer is an array?
//
// Note that the input array is passed in by reference, which means modification
// to the input array will be known to the caller as well.

class RemoveDuplicatesFromSortedArray final
  : public Singleton<RemoveDuplicatesFromSortedArray> {
  std::size_t remove_duplicates(std::vector<int> array) {
    std::size_t size = array.size();

    if (size <= 1)
      return size;

    std::size_t pos{};
    for (std::size_t i = 0; i < size - 1; ++i) {
      if (array[i] != array[i + 1])
        array[++pos] = array[i + 1];
    }

    return pos + 1;
  }
public:
  void run() {
    HARNESS_EQ(remove_duplicates({1,1,2}), 2);
    HARNESS_EQ(remove_duplicates({0,0,1,1,1,2,2,3,3,3,4}), 5);
  }
};

HARNESS_TEST(RemoveDuplicatesFromSortedArray, harness::FakeTester) {
  RemoveDuplicatesFromSortedArray::get_instance().run();
}
