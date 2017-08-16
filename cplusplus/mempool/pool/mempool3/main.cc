// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <ctime>
#include <mutex>
#include "mempool.h"

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  static constexpr int kCount = 1000000;
  int* alloc_bytes_array = new int[kCount];
  std::srand((unsigned int)std::time(nullptr));
  for (auto i = 0; i < kCount; ++i) {
    alloc_bytes_array[i] = std::rand() % 512;
    if (alloc_bytes_array[i] == 0)
      alloc_bytes_array[i] = 1;
  }

  std::clock_t beg, end;
  void* p{};

  {
    beg = std::clock();
    for (auto i = 0; i < kCount; ++i) {
      p = std::malloc(alloc_bytes_array[i]);
      std::free(p);
    }
    end = std::clock();
    std::cout << "[system c allocator] use: " << end - beg << std::endl;
  }

  {
    beg = std::clock();
    for (auto i = 0; i < kCount; ++i) {
      p = MemPool<std::mutex>::get_instance().alloc(alloc_bytes_array[i]);
      MemPool<std::mutex>::get_instance().dealloc(p);
    }
    end = std::clock();
    std::cout << "[memory pool allocator] use: " << end - beg << std::endl;
  }

  delete [] alloc_bytes_array;

  return 0;
}
