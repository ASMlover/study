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
#include <boost/multi_array.hpp>
#include <algorithm>
#include <iostream>
#include <cstring>

static void boost_multi_array_one_dimensional_array() noexcept {
  std::cout << "--------- [multi_array.one_dimensional_array] ---------" << std::endl;

  boost::multi_array<char, 1> a{boost::extents[6]};
  a[0] = 'B';
  a[0] = 'o';
  a[0] = 'o';
  a[0] = 's';
  a[0] = 't';
  a[0] = '\0';
  std::cout << "[demo.multi_array] " << a.origin() << std::endl;
}

static void boost_multi_array_views_and_subarrays() noexcept {
  std::cout << "--------- [multi_array.views_and_subarrays] ---------" << std::endl;

  using array_view = boost::multi_array<char, 2>::array_view<1>::type;
  using range      = boost::multi_array_types::index_range;

  boost::multi_array<char, 2> a{boost::extents[2][6]};
  array_view view = a[boost::indices[0][range{0, 5}]];

  std::memcpy(view.origin(), "tsooB", 6);
  std::reverse(view.begin(), view.end());

  std::cout << "[demo.multi_array] " << view.origin() << std::endl;

  boost::multi_array<char, 2>::reference subarray = a[1];
  std::memcpy(subarray.origin(), "C++", 4);

  std::cout << "[demo.multi_array] " << subarray.origin() << std::endl;
}

void boost_multi_array() noexcept {
  std::cout << "========= [multi_array] =========" << std::endl;

  boost_multi_array_one_dimensional_array();
  boost_multi_array_views_and_subarrays();
}
