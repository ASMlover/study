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
#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>


static void boost_tokenizer_iterate_over_partial_expr() noexcept {
  std::cout << "--------- [tokenizer.iterate_over_partial_expr] ---------" << std::endl;

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  std::string s = "Boost C++ Libraries";
  tokenizer tok{s};
  for (tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
    std::cout << "[demo.tokenizer] " << *it << std::endl;
}

static void boost_tokenizer_initializing() noexcept {
  std::cout << "--------- [tokenizer.initializing] ---------" << std::endl;

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  std::string s = "Boost C++ Libraries";
  boost::char_separator<char> sep{" "};
  tokenizer tok{s, sep};
  for (const auto& t : tok)
    std::cout << "[demo.tokenizer] " << t << std::endl;
}

static void boost_tokenizer_simulating_the_default_behavior() noexcept {
  std::cout << "--------- [tokenizer.simulating_the_default_behavior] ---------" << std::endl;

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  std::string s = "Boost C++ Libraries";
  boost::char_separator<char> sep{" ", "+"};
  tokenizer tok{s, sep};
  for (const auto& t : tok)
    std::cout << "[demo.tokenizer] " << t << std::endl;
}

static void boost_tokenizer_initializing_to_display() noexcept {
  std::cout << "--------- [tokenizer.initializing_to_display] ---------" << std::endl;

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  std::string s = "Boost C++ Libraries";
  boost::char_separator<char> sep{" ", "+", boost::keep_empty_tokens};
  tokenizer tok{s, sep};
  for (const auto& t : tok)
    std::cout << "[demo.tokenizer] " << t << std::endl;
}

void boost_tokenizer() noexcept {
  std::cout << "========= [tokenizer] =========" << std::endl;

  boost_tokenizer_iterate_over_partial_expr();
  boost_tokenizer_initializing();
  boost_tokenizer_simulating_the_default_behavior();
  boost_tokenizer_initializing_to_display();
}
