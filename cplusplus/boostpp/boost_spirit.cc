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
#include <boost/spirit/include/qi.hpp>
#include <string>
#include <iostream>

static void boost_spirit_using_parse() noexcept {
  std::cout << "--------- [spirit.using_parse] ---------" << std::endl;

  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::string s;
  std::getline(std::cin, s);

  auto it = s.begin();
  bool match = boost::spirit::qi::parse(it, s.end(), boost::spirit::ascii::digit);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_using_phrase_parse() noexcept {
  std::cout << "--------- [spirit.using_phrase_parse] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.begin(), ascii::digit, ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

void boost_spirit() noexcept {
  std::cout << "========= [spirit] =========" << std::endl;

  boost_spirit_using_parse();
  boost_spirit_using_phrase_parse();
}
