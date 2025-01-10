// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include <boost/pool/simple_segregated_storage.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <vector>
#include <cstddef>
#include <iostream>

static void boost_object_pool() noexcept {
  std::cout << "--------- [pool.object_pool] ---------" << std::endl;
  boost::object_pool<int> pool;

  int* i = pool.malloc();
  *i = 1;

  int* j = pool.construct(2);

  std::cout << "[demo.pool.object_pool] *i = " << *i << ", *j = " << *j << std::endl;

  pool.destroy(i);
  pool.destroy(j);
}

static void boost_object_pool2() noexcept {
  std::cout << "--------- [pool.object_pool.2] ---------" << std::endl;
  boost::object_pool<int> pool{32, 0};

  pool.construct();
  std::cout << "[demo.pool.object_pool] " << pool.get_next_size() << std::endl;
  pool.set_next_size(8);
}

static void boost_pool_allocator() noexcept {
  std::cout << "--------- [pool.pool_allocator] ---------" << std::endl;

  std::vector<int, boost::pool_allocator<int>> v;
  for (int i = 0; i < 1000; ++i)
    v.push_back(i);

  v.clear();
  boost::singleton_pool<boost::pool_allocator_tag, sizeof(int)>::purge_memory();
}

void boost_pool() noexcept {
  std::cout << "========= [pool] =========" << std::endl;

  boost::simple_segregated_storage<size_t> storage;
  std::vector<char> v(1024);
  storage.add_block(&v.front(), v.size(), 256);

  int* i = static_cast<int*>(storage.malloc());
  *i = 1;

  int* j = static_cast<int*>(storage.malloc_n(1, 512));
  j[10] = 2;

  storage.free(i);
  storage.free_n(j, 1, 512);

  boost_object_pool();
  boost_object_pool2();
  boost_pool_allocator();
}
