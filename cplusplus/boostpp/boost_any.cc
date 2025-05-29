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
#include <boost/any.hpp>
#include <string>
#include <iostream>

static void boost_any_using_any() noexcept {
  std::cout << "--------- [any.using_any] ---------" << std::endl;

  boost::any a = 1;
  a = 3.14;
  a = true;
}

static void boost_any_storing_a_string() noexcept {
  std::cout << "--------- [any.storing_a_string] ---------" << std::endl;

  boost::any a = std::string{"Boost"};
}

static void boost_any_accessing_values() noexcept {
  std::cout << "--------- [any.accessing_values] ---------" << std::endl;

  boost::any a = 1;
  std::cout << "[demo.any] " << boost::any_cast<int>(a) << std::endl;

  a = 3.14;
  std::cout << "[demo.any] " << boost::any_cast<double>(a) << std::endl;

  a = true;
  std::cout << "[demo.any] " << std::boolalpha << boost::any_cast<bool>(a) << std::endl;
}

static void boost_any_bad_any_cast() noexcept {
  std::cout << "--------- [any.bad_any_cast] ---------" << std::endl;

  try {
    boost::any a = 1;
    std::cout << "[demo.any] " << boost::any_cast<float>(a) << std::endl;
  }
  catch (boost::bad_any_cast& e) {
    std::cerr << "[demo.any] ERROR: " << e.what() << std::endl;
  }
}

void boost_any() noexcept {
  std::cout << "========= [any] =========" << std::endl;

  boost_any_using_any();
  boost_any_storing_a_string();
  boost_any_accessing_values();
  boost_any_bad_any_cast();
}
