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
#include <boost/algorithm/cxx11/one_of.hpp>
#include <boost/algorithm/cxx11/iota.hpp>
#include <boost/algorithm/cxx11/is_sorted.hpp>
#include <boost/algorithm/cxx11/copy_if.hpp>
#include <array>
#include <vector>
#include <iterator>
#include <iostream>


static void boost_algorithm_testing_for_exactly_one_value() noexcept {
  std::cout << "--------- [algorithm.testing_for_exactly_one_value] ---------" << std::endl;
  using namespace boost::algorithm;

  std::array<int, 6> a{{0, 5, 2, 1, 4, 3}};
  auto predicate = [](int i) { return i == 4; };
  std::cout.setf(std::ios::boolalpha);

  std::cout << "[demo.algorithm] " << __func__ << " one_of(a.begin(), a.end(), predicate) => " << one_of(a.begin(), a.end(), predicate) << std::endl;
  std::cout << "[demo.algorithm] " << __func__ << " one_of_equal(a.begin(), a.end(), 4) = > " << one_of_equal(a.begin(), a.end(), 4) << std::endl;
}

static void boost_algorithm_more_variants() noexcept {
  std::cout << "--------- [algorithm.more_variants] ---------" << std::endl;
  using namespace boost::algorithm;

  std::vector<int> v;
  iota_n(std::back_inserter(v), 10, 5);
  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.algorithm] " << __func__ << " is_increasing(v) => " << is_increasing(v) << std::endl;
  std::ostream_iterator<int> out{std::cout, ","};
  copy_until(v, out, [](int i) { return i > 12; });
}

void boost_algorithm() noexcept {
  std::cout << "========= [algorithm] =========" << std::endl;

  boost_algorithm_testing_for_exactly_one_value();
  boost_algorithm_more_variants();
}
