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
#include <boost/algorithm/string.hpp>
#include <string>
#include <locale>
#include <iostream>

static void boost_to_upper_copy() noexcept {
  std::cout << "--------- [string_algorithms.to_upper_copy] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  std::cout << "[demo.string_algorithms] " << boost::algorithm::to_upper_copy(s) << std::endl;
}

static void boost_to_upper_copy2() noexcept {
  std::cout << "--------- [string_algorithms.to_upper_copy2] ---------" << std::endl;

  std::string s = "Boost C++ k\xfct\xfcphaneleri";
  std::string upper_case1 = boost::algorithm::to_upper_copy(s);
  std::string upper_case2 = boost::algorithm::to_upper_copy(s, std::locale{"Turkish"});
  std::locale::global(std::locale{"Turkish"});
  std::cout << "[demo.string_algorithms] " << upper_case1 << std::endl;
  std::cout << "[demo.string_algorithms] " << upper_case2 << std::endl;
}

static void boost_remove_from_string() noexcept {
  std::cout << "--------- [string_algorithms.remove_from_string] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_first_copy(s, "s") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_nth_copy(s, "s", 0) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_last_copy(s, "s") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_all_copy(s, "s") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_head_copy(s, 5) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::erase_tail_copy(s, 9) << std::endl;
}

static void boost_search_substring() noexcept {
  std::cout << "--------- [string_algorithms.search_substring] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  boost::iterator_range<std::string::iterator> r = boost::algorithm::find_first(s, "C++");
  std::cout << "[demo.string_algorithms] " << r << std::endl;
  r = boost::algorithm::find_first(s, "xyz");
  std::cout << "[demo.string_algorithms] " << r << std::endl;
}

void boost_string_algorithms() noexcept {
  std::cout << "========= [string_algorithms] =========" << std::endl;

  boost_to_upper_copy();
  boost_to_upper_copy2();
  boost_remove_from_string();
  boost_search_substring();
}
