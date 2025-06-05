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
#include <boost/variant.hpp>
#include <string>
#include <iostream>

static void boost_variant_basic() noexcept {
  std::cout << "--------- [variant.basic] ---------" << std::endl;

  boost::variant<double, char, std::string> v;
  v = 3.14;
  v = 'A';
  v = "Boost";
}

static void boost_variant_accessing_values() noexcept {
  std::cout << "--------- [variant.accessing_values] ---------" << std::endl;

  boost::variant<double, char, std::string> v;
  v = 3.14;
  std::cout << "[demo.variant] " << boost::get<double>(v) << std::endl;

  v = 'A';
  std::cout << "[demo.variant] " << boost::get<char>(v) << std::endl;

  v = "Boost";
  std::cout << "[demo.variant] " << boost::get<std::string>(v) << std::endl;
}

static void boost_variant_direct_output_on_stream() noexcept {
  std::cout << "--------- [variant.direct_output_on_stream] ---------" << std::endl;

  boost::variant<double, char, std::string> v;
  v = 3.14;
  std::cout << "[demo.variant] " << v << std::endl;

  v = 'A';
  std::cout << "[demo.variant] " << v << std::endl;

  v = "Boost";
  std::cout << "[demo.variant] " << v << std::endl;
}

void boost_variant() noexcept {
  std::cout << "========= [variant] =========" << std::endl;

  boost_variant_basic();
  boost_variant_accessing_values();
  boost_variant_direct_output_on_stream();
}
