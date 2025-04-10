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
#include <boost/circular_buffer.hpp>
#include <iostream>

static void boost_circular_buffer_basic() noexcept {
  std::cout << "--------- [circular_buffer.basic] ---------" << std::endl;
  using circular_buffer = boost::circular_buffer<int>;

  circular_buffer cb{3};
  std::cout << "[demo.circular_buffer] " << cb.capacity() << std::endl;
  std::cout << "[demo.circular_buffer] " << cb.size() << std::endl;

  cb.push_back(0);
  cb.push_back(1);
  cb.push_back(2);
  std::cout << "[demo.circular_buffer] " << cb.size() << std::endl;

  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  std::cout << "[demo.circular_buffer] " << cb.size() << std::endl;

  for (int i : cb)
    std::cout << "[demo.circular_buffer] " << i << std::endl;
}

static void boost_circular_buffer_various_member_functions() noexcept {
  std::cout << "--------- [circular_buffer.various_member_functions] ---------" << std::endl;
  using circular_buffer = boost::circular_buffer<int>;

  circular_buffer cb{3};
  cb.push_back(0);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  std::cout << "[demo.circular_buffer] " << std::boolalpha << cb.is_linearized() << std::endl;

  circular_buffer::array_range ar1, ar2;
  ar1 = cb.array_one();
  ar2 = cb.array_two();
  std::cout << "[demo.circular_buffer] " << ar1.second << ";" << ar2.second << std::endl;

  for (int i : cb)
    std::cout << "[demo.circular_buffer] " << i << std::endl;

  cb.linearize();
  ar1 = cb.array_one();
  ar2 = cb.array_two();
  std::cout << "[demo.circular_buffer] " << ar1.second << ";" << ar2.second << std::endl;
}

void boost_circular_buffer() noexcept {
  std::cout << "========= [circular_buffer] =========" << std::endl;

  boost_circular_buffer_basic();
  boost_circular_buffer_various_member_functions();
}
