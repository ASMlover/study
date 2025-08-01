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
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <iostream>

static void boost_tribool_three_states() noexcept {
  std::cout << "--------- [tribool.three_states] ---------" << std::endl;
  using namespace boost::logic;

  tribool b{};
  std::cout << "[demo.tribool] " << __func__ << " bool(b) => " << std::boolalpha << bool(b) << std::endl;

  b = true;
  b = false;
  b = indeterminate;
  if (b) {
  }
  else if (!b) {
  }
  else {
    std::cout << "[demo.tribool] " << __func__ << " => " << "indeterminate" << std::endl;
  }
}

static void boost_tribool_logical_operators() noexcept {
  std::cout << "--------- [tribool.logical_operators] ---------" << std::endl;
  using namespace boost::logic;

  std::cout.setf(std::ios::boolalpha);

  tribool b1 = true;
  std::cout << "[demo.tribool] " << __func__ << " (b1 || indeterminate) => " << (b1 || indeterminate) << std::endl;
  std::cout << "[demo.tribool] " << __func__ << " (b1 && indeterminate) => " << (b1 && indeterminate) << std::endl;

  tribool b2 = false;
  std::cout << "[demo.tribool] " << __func__ << " (b2 || indeterminate) => " << (b2 || indeterminate) << std::endl;
  std::cout << "[demo.tribool] " << __func__ << " (b2 && indeterminate) => " << (b2 && indeterminate) << std::endl;

  tribool b3 = indeterminate;
  std::cout << "[demo.tribool] " << __func__ << " (b3 || b3) => " << (b3 || b3) << std::endl;
  std::cout << "[demo.tribool] " << __func__ << " (b3 && b3) => " << (b3 && b3) << std::endl;
}

void boost_tribool() noexcept {
  std::cout << "========= [tribool] =========" << std::endl;

  boost_tribool_three_states();
  boost_tribool_logical_operators();
}
