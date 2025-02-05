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
// #include <boost/algorithm/string/regex.hpp>
#include <string>
#include <locale>
#include <vector>
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

static void boost_concatenate_string() noexcept {
  std::cout << "--------- [string_algorithms.concatenate_string] --------" << std::endl;

  std::vector<std::string> v{"Boost", "C++", "Libraries"};
  std::cout << "[demo.string_algorithms] " << boost::algorithm::join(v, " ") << std::endl;
}

static void boost_replace_string() noexcept {
  std::cout << "--------- [string_algorithms.replace_string] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_first_copy(s, "+", "-") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_nth_copy(s, "+", 0, "-") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_last_copy(s, "+", "-") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_all_copy(s, "+", "-") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_head_copy(s, 5, "BOOST") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::replace_tail_copy(s, 9, "LIBRARIES") << std::endl;
}

static void boost_trim_string() noexcept {
  std::cout << "--------- [string_algorithms.trim_string] ---------" << std::endl;

  std::string s = "\t Boost C++ Libraries \t";
  std::cout << "[demo.string_algorithms] _" << boost::algorithm::trim_left_copy(s) << "_" << std::endl;
  std::cout << "[demo.string_algorithms] _" << boost::algorithm::trim_right_copy(s) << "_" << std::endl;
  std::cout << "[demo.string_algorithms] _" << boost::algorithm::trim_copy(s) << "_" << std::endl;
}

static void boost_create_predicates() noexcept {
  std::cout << "--------- [string_algorithms.create_predicates] ---------" << std::endl;

  std::string s = "--Boost C++ Libraries--";
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_left_copy_if(s, boost::algorithm::is_any_of("-")) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_right_copy_if(s, boost::algorithm::is_any_of("-")) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_copy_if(s, boost::algorithm::is_any_of("-")) << std::endl;
}

static void boost_create_predicates2() noexcept {
  std::cout << "--------- [string_algorithms.create_predicates2] ---------" << std::endl;

  std::string s = "123456789Boost C++ Libraries123456789";
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_left_copy_if(s, boost::algorithm::is_digit()) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_right_copy_if(s, boost::algorithm::is_digit()) << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::trim_copy_if(s, boost::algorithm::is_digit()) << std::endl;
}

static void boost_compare_string() noexcept {
  std::cout << "--------- [string_algorithms.compare_string] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.string_algorithms] " << boost::algorithm::starts_with(s, "Boost") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::ends_with(s, "Libraries") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::contains(s, "C++") << std::endl;
  std::cout << "[demo.string_algorithms] " << boost::algorithm::lexicographical_compare(s, "Boost") << std::endl;
}

static void boost_split_string() noexcept {
  std::cout << "--------- [string_algorithms.split_string] ---------" << std::endl;

  std::string s = "Boost C++ Libraries";
  std::vector<std::string> v;
  boost::algorithm::split(v, s, boost::algorithm::is_space());
  std::cout << "[demo.string_algorithms] " << v.size() << std::endl;
}

static void boost_search_with_regex() noexcept {
  std::cout << "--------- [string_algorithms.search_with_regex] ---------" << std::endl;

  // std::string s = "Boost C++ Libraries";
  // boost::iterator_range<std::string::iterator> r = boost::algorithm::find_regex(s, boost::regex{"\\w\\+\\+"});
  // std::cout << "[demo.string_algorithms] " << r << std::endl;
}

void boost_string_algorithms() noexcept {
  std::cout << "========= [string_algorithms] =========" << std::endl;

  boost_to_upper_copy();
  boost_to_upper_copy2();
  boost_remove_from_string();
  boost_search_substring();
  boost_concatenate_string();
  boost_replace_string();
  boost_trim_string();
  boost_create_predicates();
  boost_create_predicates2();
  boost_compare_string();
  boost_split_string();
  boost_search_with_regex();
}
