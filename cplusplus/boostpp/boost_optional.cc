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
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <boost/optional.hpp>

static int get_even_random_number() noexcept {
  int i = std::rand();
  return (i % 2 == 0) ? i : -1;
}

static void boost_optional_special_values_to_denote_optional() noexcept {
  std::cout << "--------- [optional.special_values_to_denote_optional] ---------" << std::endl;

  std::srand(static_cast<unsigned int>(std::time(0)));
  int i = get_even_random_number();
  if (i != -1)
    std::cout << "[demo.optional] " << std::sqrt(static_cast<float>(i)) << std::endl;
}

static boost::optional<int> get_even_random_number2() noexcept {
  int i = std::rand();
  return (i % 2 == 0) ? i : boost::optional<int>{};
}

static void boost_optional_return_with_optional() noexcept {
  std::cout << "--------- [optional.return_with_optional] ---------" << std::endl;

  std::srand(static_cast<unsigned int>(std::time(0)));
  boost::optional<int> i = get_even_random_number2();
  if (i)
    std::cout << "[demo.optional] " << std::sqrt(static_cast<float>(*i)) << std::endl;
}

static boost::optional<int> get_even_random_number3() noexcept {
  int i = std::rand();
  return boost::optional<int>{i % 2 == 0, i};
}

static void boost_optional_useful_member_functions() noexcept {
  std::cout << "--------- [optional.useful_member_functions] ---------" << std::endl;

  std::srand(static_cast<unsigned int>(std::time(0)));
  boost::optional<int> i = get_even_random_number3();
  if (i.is_initialized())
    std::cout << "[demo.optional] " << std::sqrt(static_cast<float>(i.get())) << std::endl;
}

static boost::optional<int> get_even_random_number4() noexcept {
  int i = std::rand();
  return boost::make_optional(i % 2 == 0, i);
}

static void boost_optional_various_helper_functions() noexcept {
  std::cout << "--------- [optional.various_helper_functions] ---------" << std::endl;

  std::srand(static_cast<unsigned int>(std::time(0)));
  boost::optional<int> i = get_even_random_number4();
  double d = boost::get_optional_value_or(i, 0);
  std::cout << "[demo.optional] " << std::sqrt(d) << std::endl;
}

void boost_optional() noexcept {
  std::cout << "========= [optional] =========" << std::endl;

  boost_optional_special_values_to_denote_optional();
  boost_optional_return_with_optional();
  boost_optional_useful_member_functions();
  boost_optional_various_helper_functions();
}
