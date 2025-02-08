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
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>

static void boost_basic_lexical_cast() noexcept {
  std::cout << "--------- [lexical_cast.basic_lexical_cast] ---------" << std::endl;

  std::string s = boost::lexical_cast<std::string>(123);
  std::cout << "[demo.lexical_cast] " << s << std::endl;
  double d = boost::lexical_cast<double>(s);
  std::cout << "[demo.lexical_cast] " << d << std::endl;
}

static void boost_bad_lexical_cast() noexcept {
  std::cout << "--------- [lexical_cast.bad_lexical_cast] ---------" << std::endl;

  try {
    int i = boost::lexical_cast<int>("abc");
    std::cout << "[demo.lexical_cast] " << i << std::endl;
  } catch (const boost::bad_lexical_cast& e) {
    std::cerr << "[demo.lexical_cast] " << e.what() << std::endl;
  }
}

void boost_lexical_cast() noexcept {
  std::cout << "========= [lexical_cast] =========" << std::endl;

  boost_basic_lexical_cast();
  boost_bad_lexical_cast();
}
