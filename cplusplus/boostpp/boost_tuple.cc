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
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <string>
#include <iostream>

static void boost_tuple_replacing_pair() noexcept {
  std::cout << "--------- [tuple.replacing_pair] ---------" << std::endl;
  using animal = boost::tuple<std::string, int>;

  animal a{"cat", 4};
  std::cout << "[demo.tuple] " << a << std::endl;
}

static void boost_tuple_as_better_pair() noexcept {
  std::cout << "--------- [tuple.as_bettler_pair] ---------" << std::endl;
  using animal = boost::tuple<std::string, int, bool>;

  animal a{"cat", 4, true};
  std::cout << "[demo.tuple] " << std::boolalpha << a << std::endl;
}

static void boost_tuple_with_make_tuple() noexcept {
  std::cout << "--------- [tuple.with_make_tuple] ---------" << std::endl;

  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.tuple] " << boost::make_tuple("cat", 4, true) << std::endl;
}

void boost_tuple() noexcept {
  std::cout << "========= [tuple] =========" << std::endl;

  boost_tuple_replacing_pair();
  boost_tuple_as_better_pair();
  boost_tuple_with_make_tuple();
}
