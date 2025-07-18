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
#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <array>
#include <vector>
#include <iterator>
#include <string>
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
  std::cout << std::endl;
}

static void boost_algorithm_using_cxx14() noexcept {
  std::cout << "--------- [algorithm.using_cxx14] ---------" << std::endl;
  using namespace boost::algorithm;

  std::vector<int> v{1, 2};
  std::vector<int> w{1, 2, 3};
  std::cout.setf(std::ios::boolalpha);

  auto x = boost::algorithm::equal(v.begin(), v.end(), w.begin(), w.end());
  std::cout << "[demo.algorithm] " << __func__ << " equal(v.begin(), v.end(), w.begin(), w.end()) => " << x << std::endl;

  auto pair = boost::algorithm::mismatch(v.begin(), v.end(), w.begin(), w.end());
  if (pair.first != v.end())
    std::cout << "[demo.algorithm] " << __func__ << " *pair.first => " << *pair.first << std::endl;
  if (pair.second != w.end())
    std::cout << "[demo.algorithm] " << __func__ << " *pair.second => " << *pair.second << std::endl;
}

static void boost_algorithm_using_hex_and_unhex() noexcept {
  std::cout << "--------- [algorithm.using_hex_and_unhex] ---------" << std::endl;
  using namespace boost::algorithm;

  std::vector<char> v{'C', '+', '+'};
  hex(v, std::ostream_iterator<char>{std::cout, ""});
  std::cout << std::endl;

  std::string s = "C++";
  std::cout << "[demo.algorithm] " << __func__ << " hex(s) => " << hex(s) << std::endl;

  std::vector<char> w{'4', '3', '2', 'b', '2', 'b'};
  unhex(w, std::ostream_iterator<char>{std::cout, ""});
  std::cout << std::endl;

  std::string t = "432b2b";
  std::cout << "[demo.algorithm] " << __func__ << " unhex(t) => " << unhex(t) << std::endl;
}

static void boost_algorithm_counting() noexcept {
  std::cout << "--------- [algorithm.counting] ---------" << std::endl;

  std::array<int, 6> a{{0, 1, 0, 1, 0, 1}};
  std::cout << "[demo.algorithm] " << __func__ << " count(a, 0) => " << boost::count(a, 0) << std::endl;
}

static void boost_algorithm_related_from_stdlib() noexcept {
  std::cout << "--------- [algorithm.related_from_stdlib] ---------" << std::endl;

  std::array<int, 6> a{{0, 1, 2, 3, 4, 5}};
  boost:: random_shuffle(a);
  boost::copy(a, std::ostream_iterator<int>{std::cout, ","});
  std::cout << std::endl;
  std::cout << "[demo.algorithm] " << __func__ << " *boost::max_element(a) => " << *boost::max_element(a) << std::endl;
  std::cout << "[demo.algorithm] " << __func__ << " boost::accumulate(a, 0) => " << boost::accumulate(a, 0) << std::endl;
}

void boost_algorithm() noexcept {
  std::cout << "========= [algorithm] =========" << std::endl;

  boost_algorithm_testing_for_exactly_one_value();
  boost_algorithm_more_variants();
  boost_algorithm_using_cxx14();
  boost_algorithm_using_hex_and_unhex();
  boost_algorithm_counting();
  boost_algorithm_related_from_stdlib();
}
