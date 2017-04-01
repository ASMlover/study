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
#include <ctime>
#include <iostream>
#include "mempool_v1.h"
#include "mempool_v2.h"

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  static int COUNT = 100000;
  auto beg = std::clock();
  for (auto i = 0; i < COUNT; ++i) {
    int* p = new int;
    delete p;
  }
  std::cout << "[system allocator] use clock: " << std::clock() - beg << std::endl;

  {
    v1::MemoryPool pool(sizeof(int), 8, 16);

    beg = std::clock();
    for (auto i = 0; i < COUNT; ++i) {
      int* p = (int*)pool.alloc();
      pool.dealloc(p);
    }
    std::cout << "[mempool allocator (v1)] use clock: " << std::clock() - beg << std::endl;
  }

  {
    auto& pool = v2::MemoryPool::get_instance();

    beg = std::clock();
    for (auto i = 0; i < COUNT; ++i) {
      int* p = (int*)pool.alloc(sizeof(int));
      pool.dealloc(p, sizeof(int));
    }
    std::cout << "[mempool allocator (v2)] use clock: " << std::clock() - beg << std::endl;
  }

  return 0;
}
