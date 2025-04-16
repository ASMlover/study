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
#include <boost/heap/priority_queue.hpp>
#include <boost/heap/binomial_heap.hpp>
#include <iostream>

static void boost_heap_priority_queue() noexcept {
  std::cout << "--------- [heap.priority_queue] ---------" << std::endl;
  using namespace boost::heap;

  priority_queue<int> pq;
  pq.push(2);
  pq.push(3);
  pq.push(1);

  for (int i : pq)
    std::cout << "[demo.heap] " << i << std::endl;

  priority_queue<int> pq2;
  pq2.push(4);
  std::cout << "[demo.heap] " << std::boolalpha << (pq > pq2) << std::endl;
}

static void boost_heap_binomial_heap() noexcept {
  std::cout << "--------- [heap.binomial_heap] ---------" << std::endl;
  using namespace boost::heap;

  binomial_heap<int> bh;
  bh.push(2);
  bh.push(3);
  bh.push(1);

  binomial_heap<int> bh2;
  bh2.push(4);
  bh.merge(bh2);

  for (auto it = bh.ordered_begin(); it != bh.ordered_end(); ++it)
    std::cout << "[demo.heap] " << *it << std::endl;
  std::cout << "[demo.heap] " << std::boolalpha << bh2.empty() << std::endl;
}

static void boost_heap_changing_elements() noexcept {
  std::cout << "--------- [heap.changing_elements] ---------" << std::endl;
  using namespace boost::heap;

  binomial_heap<int> bh;
  auto handle = bh.push(2);
  bh.push(3);
  bh.push(1);

  bh.update(handle, 4);
  std::cout << "[demo.heap] " << bh.top() << std::endl;
}

void boost_heap() noexcept {
  std::cout << "========= [heap] =========" << std::endl;

  boost_heap_priority_queue();
  boost_heap_binomial_heap();
  boost_heap_changing_elements();
}
