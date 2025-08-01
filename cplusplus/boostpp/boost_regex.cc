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
#include <boost/regex.hpp>
#include <locale>
#include <string>
#include <iostream>

static void boost_compare_string_with_regex() noexcept {
  std::cout << "--------- [regex.compare_string_with_regex] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"\\w+\\s\\w+"};
  std::cout << "[demo.regex] " << std::boolalpha << boost::regex_match(s, expr) << std::endl;
}

static void boost_search_with_regex() noexcept {
  std::cout << "--------- [regex.search_with_regex] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"(\\w+)\\s(\\w+)"};
  boost::smatch what;
  if (boost::regex_search(s, what, expr)) {
    std::cout << "[demo.regex] " << what[0] << std::endl;
    std::cout << "[demo.regex] " << what[1] << "_" << what[2] << std::endl;
  }
}

static void boost_replace_with_regex() noexcept {
  std::cout << "--------- [regex.replace_with_regex] ---------" << std::endl;

  std::string s = " Boost Libraries ";
  boost::regex expr{"\\s"};
  std::string fmt{"_"};
  std::cout << "[demo.regex] " << boost::regex_replace(s, expr, fmt) << std::endl;
}

static void boost_format_with_reference_in_regular() noexcept {
  std::cout << "--------- [regex.format_with_reference_in_regular] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"(\\w+)\\s(\\w+)"};
  std::string fmt{"\\2 \\1"};
  std::cout << "[demo.regex] " << boost::regex_replace(s, expr, fmt) << std::endl;
}

static void boost_flag_for_formats() noexcept {
  std::cout << "--------- [regex.flag_for_formats] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"(\\w+)\\s(\\w+)"};
  std::string fmt{"\\2 \\1"};
  std::cout << "[demo.regex] "
    << boost::regex_replace(s, expr, fmt, boost::regex_constants::format_literal)
    << std::endl;
}

static void boost_iterate_over_string() noexcept {
  std::cout << "--------- [regex.iterate_over_string] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"\\w+"};
  boost::regex_token_iterator<std::string::iterator> it{s.begin(), s.end(), expr};
  boost::regex_token_iterator<std::string::iterator> end;
  while (it != end)
    std::cout << "[demo.regex] " << *it++ << std::endl;
}

static void boost_access_groups() noexcept {
  std::cout << "--------- [regex.access_groups] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"(\\w)\\w+"};
  boost::regex_token_iterator<std::string::iterator> it{s.begin(), s.end(), expr, 1};
  boost::regex_token_iterator<std::string::iterator> end;
  while (it != end)
    std::cout << "[demo.regex] " << *it++ << std::endl;
}

static void boost_link_locale_to_regular_expr() noexcept {
  std::cout << "--------- [regex.link_locale_to_regular_expr] ---------" << std::endl;

  std::string s = "Boost k\xfct\xfcphaneleri";
  boost::basic_regex<char, boost::cpp_regex_traits<char>> expr;
  expr.imbue(std::locale{"Turkish"});
  expr = "\\w+\\s\\w+";
  std::cout << "[demo.regex] " << std::boolalpha << boost::regex_match(s, expr) << std::endl;
}

void boost_regex() noexcept {
  std::cout << "========= [regex] =========" << std::endl;

  boost_compare_string_with_regex();
  boost_search_with_regex();
  boost_replace_with_regex();
  boost_format_with_reference_in_regular();
  boost_flag_for_formats();
  boost_iterate_over_string();
  boost_access_groups();
  boost_link_locale_to_regular_expr();
}
