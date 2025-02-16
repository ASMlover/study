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

static void boost_format_with_reference_in_reguar() noexcept {
  std::cout << "--------- [regex.format_with_reference_in_reguar] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::regex expr{"(\\w+)\\s(\\w+)"};
  std::string fmt{"\\2 \\1"};
  std::cout << "[demo.regex] " << boost::regex_replace(s, expr, fmt) << std::endl;
}

void boost_regex() noexcept {
  std::cout << "========= [regex] =========" << std::endl;

  boost_compare_string_with_regex();
  boost_search_with_regex();
  boost_replace_with_regex();
  boost_format_with_reference_in_reguar();
}
