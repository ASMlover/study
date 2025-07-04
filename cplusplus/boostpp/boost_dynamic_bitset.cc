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
#include <boost/dynamic_bitset.hpp>
#include <iostream>

static void boost_dynamic_bitset_basic_using() noexcept {
  std::cout << "--------- [dynamic_bitset.basic_using] ---------" << std::endl;

  boost::dynamic_bitset<> db{3, 4};
  db.push_back(true);

  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.dynamic_bitset] db.size() = " << db.size() << std::endl;
  std::cout << "[demo.dynamic_bitset] db.count() = " << db.count() << std::endl;
  std::cout << "[demo.dynamic_bitset] db.any() = " << db.any() << std::endl;
  std::cout << "[demo.dynamic_bitset] db.none() = " << db.none() << std::endl;
  std::cout << "[demo.dynamic_bitset] db[0].flip() = " << db[0].flip() << std::endl;
  std::cout << "[demo.dynamic_bitset] ~db[3] = " << ~db[3] << std::endl;
  std::cout << "[demo.dynamic_bitset] db = " << db << std::endl;
}

void boost_dynamic_bitset() noexcept {
  std::cout << "========= [dynamic_bitset] =========" << std::endl;

  boost_dynamic_bitset_basic_using();
}
