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
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/regex.hpp>
#include <array>
#include <map>
#include <string>
#include <utility>
#include <iterator>
#include <iostream>

static void boost_adaptors_filtering_a_range() noexcept {
  std::cout << "--------- [adaptors.filtering_a_range] ---------" << std::endl;

  std::array<int, 6> a{{0, 5, 2, 1, 3, 4}};
  boost::copy(boost::adaptors::filter(a, [](int i) { return i > 2; }),
      std::ostream_iterator<int>{std::cout, ","});
  std::cout << std::endl;
}

static void boost_adaptors_using_keys_values_indirect() noexcept {
  std::cout << "-------- [adaptors.using_keys_values_indirect] ---------" << std::endl;

  std::array<int, 3> a{{0, 1, 2}};
  std::map<std::string, int*> m;
  m.insert(std::make_pair("a", &a[0]));
  m.insert(std::make_pair("b", &a[1]));
  m.insert(std::make_pair("c", &a[2]));

  boost::copy(boost::adaptors::keys(m),
      std::ostream_iterator<std::string>{std::cout, ","});
  boost::copy(boost::adaptors::indirect(boost::adaptors::values(m)),
      std::ostream_iterator<int>(std::cout, ","));
  std::cout << std::endl;
}

static void boost_adaptors_tokenize() noexcept {
  std::cout << "--------- [adaptors.tokenize] ---------" << std::endl;

  std::string s = "The Boost C++ Libraries";
  boost::regex expr{"[\\w+]+"};
  boost::copy(boost::adaptors::tokenize(s, expr, 0, boost::regex_constants::match_default),
      std::ostream_iterator<std::string>{std::cout, ","});
  std::cout << std::endl;
}

void boost_adaptors() noexcept {
  std::cout << "========= [adaptors] =========" << std::endl;

  boost_adaptors_filtering_a_range();
  boost_adaptors_using_keys_values_indirect();
  boost_adaptors_tokenize();
}
