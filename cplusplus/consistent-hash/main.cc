// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "hashring.h"

static void test_consistent_hash(void) {
  consistent_hash::HashRing hr;
  for (auto i = 0; i < 10; ++i) {
    std::string s("Node");
    consistent_hash::HashNode n(i + 5, s + std::to_string(i));
    hr.insert_node(n);
  }

  auto n = hr.get_node("Node3");
  std::cout << "n.name: " << n.get_name() << ", n.replicas: " << n.get_replicas() << std::endl;
}

static std::int32_t jump_consistent_hash(std::uint64_t key, std::int32_t num_buckets) {
  std::int64_t b = -1;
  std::int64_t j = 0;
  while (j < num_buckets) {
    b = j;
    key = key * 2862933555777941757ULL + 1;
    j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
  }
  return b;
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  test_consistent_hash();

  for (auto i = 0; i < 10; ++i)
    std::cout << "[" << i << "] => " << jump_consistent_hash(i, 5) << std::endl;

  std::cout << "######################################" << std::endl;
  for (auto i = 0; i < 10; ++i)
    std::cout << "[" << i << "] => " << jump_consistent_hash(i, 4) << std::endl;

  return 0;
}
