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
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/phoenix.hpp>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
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

static void boost_spirit_with_dont_postskip() noexcept {
  std::cout << "--------- [spirit.with_dont_postskip] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), ascii::digit, ascii::space, qi::skip_flag::dont_postskip);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_with_wide_strings() noexcept {
  std::cout << "--------- [spirit.with_wide_strings] ---------" << std::endl;
  using namespace boost::spirit;

  std::wstring s;
  std::wcout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::wcin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), ascii::digit, ascii::space, qi::skip_flag::dont_postskip);
  std::wcout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::wcout << "[demo.spirit] " << std::wstring{it, s.end()} << std::endl;
}

static void boost_spirit_parser_consecutive_digits() noexcept {
  std::cout << "--------- [spirit.parser_consecutive_digits] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), ascii::digit >> ascii::digit, ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_parsing_with_lexeme() noexcept {
  std::cout << "--------- [spirit.parsing_with_lexeme] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), qi::lexeme[ascii::digit >> ascii::digit], ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_rules_similar_to_regular_expr() noexcept {
  std::cout << "--------- [spirit.rules_similar_to_regular_expr] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), +ascii::digit, ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_numeric_parsers() noexcept {
  std::cout << "--------- [spirit.numeric_parsers] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(), qi::int_, ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_linking_action_with_parsers() noexcept {
  std::cout << "--------- [spirit.linking_action_with_parsers] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  bool match = qi::phrase_parse(it, s.end(),
      qi::int_[([](int i) { std::cout << "[demo.spirit] " << i << std::endl; })], ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_with_phoenix() noexcept {
  std::cout << "--------- [spirit.with_phoenix] ---------" << std::endl;
  using namespace boost::spirit;
  using boost::phoenix::ref;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  int i;
  bool match = qi::phrase_parse(it, s.end(), qi::int_[ref(i) = qi::_1], ascii::space);
  std::cout << "[demo.spirit] " << std::boolalpha << match << std::endl;
  if (match)
    std::cout << "[demo.spirit] " << i << std::endl;
  if (it != s.end())
    std::cout << "[demo.spirit] " << std::string{it, s.end()} << std::endl;
}

static void boost_spirit_storing_value_in_attribute() noexcept {
  std::cout << "--------- [spirit.storing_value_in_attribute] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  int i;
  if (qi::phrase_parse(it, s.end(), qi::int_, ascii::space, i))
    std::cout << "[demo.spirit] " << i << std::endl;
}

static void boost_spirit_storing_several_values_in_attribute() noexcept {
  std::cout << "--------- [storing_several_values_in_attribute] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  std::vector<int> v;
  if (qi::phrase_parse(it, s.end(), qi::int_ % ',', ascii::space, v)) {
    std::ostream_iterator<int> out{std::cout, ";"};
    std::copy(v.begin(), v.end(), out);
  }
}

static void boost_spirit_defining_rules() noexcept {
  std::cout << "--------- [spirit.defining_rules] ---------" << std::endl;
  using namespace boost::spirit;

  std::string s;
  std::cout << "[demo.spirit] PLEASE INPUT: ";
  std::getline(std::cin, s);
  auto it = s.begin();
  qi::rule<std::string::iterator, std::vector<int>(), ascii::space_type> values = qi::int_ % ',';
  std::vector<int> v;
  if (qi::phrase_parse(it, s.end(), values, ascii::space, v)) {
    std::ostream_iterator<int> out{std::cout, ";"};
    std::copy(v.begin(), v.end(), out);
  }
}

void boost_spirit() noexcept {
  std::cout << "========= [spirit] =========" << std::endl;

  boost_spirit_using_parse();
  boost_spirit_using_phrase_parse();
  boost_spirit_with_dont_postskip();
  boost_spirit_with_wide_strings();
  boost_spirit_parser_consecutive_digits();
  boost_spirit_parsing_with_lexeme();
  boost_spirit_rules_similar_to_regular_expr();
  boost_spirit_numeric_parsers();
  boost_spirit_linking_action_with_parsers();
  boost_spirit_with_phoenix();
  boost_spirit_storing_value_in_attribute();
  boost_spirit_storing_several_values_in_attribute();
  boost_spirit_defining_rules();
}
