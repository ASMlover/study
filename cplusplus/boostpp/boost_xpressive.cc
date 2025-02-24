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
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <string>
#include <iterator>
#include <iostream>

static void boost_xpressive_compare_strings() noexcept {
  std::cout << "--------- [xpressive.compare_strings] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::xpressive::sregex expr = boost::xpressive::sregex::compile("\\w+\\s\\w+");
  std::cout << "[demo.xpressive] " << std::boolalpha << boost::xpressive::regex_match(s, expr) << std::endl;
}

static void boost_xpressive_cregex_with_strings() noexcept {
  std::cout << "--------- [xpressive.cregex_with_strings] ---------" << std::endl;

  const char* c = "Boost Libraries";
  boost::xpressive::cregex expr = boost::xpressive::cregex::compile("\\w+\\s\\w+");
  std::cout << "[demo.xpressive] " << std::boolalpha << boost::xpressive::regex_match(c, expr) << std::endl;
}

static void boost_xpressive_regular_expr() noexcept {
  std::cout << "--------- [xpressive.regular_expr] ---------" << std::endl;

  std::string s = "Boost Libraries";
  boost::xpressive::sregex expr = +boost::xpressive::_w >> boost::xpressive::_s >> +boost::xpressive::_w;
  std::cout << "[demo.xpressive] " << std::boolalpha << boost::xpressive::regex_match(s, expr) << std::endl;
}

static void boost_xpressive_link_action_to_expr() noexcept {
  std::cout << "--------- [xpressive.link_action_to_expr] ---------" << std::endl;

  std::string s = "Boost Libraries";
  std::ostream_iterator<std::string> it{std::cout, "\n"};
  boost::xpressive::sregex expr = (+boost::xpressive::_w)[*boost::xpressive::ref(it)
    = boost::xpressive::_] >> boost::xpressive::_s >> +boost::xpressive::_w;
  std::cout << "[demo.xpressive] " << std::boolalpha << boost::xpressive::regex_match(s, expr) << std::endl;
}

void boost_xpressive() noexcept {
  std::cout << "========= [xpressive] =========" << std::endl;

  boost_xpressive_compare_strings();
  boost_xpressive_cregex_with_strings();
  boost_xpressive_regular_expr();
  boost_xpressive_link_action_to_expr();
}
