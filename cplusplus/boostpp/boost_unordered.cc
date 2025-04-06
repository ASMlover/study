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
#include <boost/unordered_set.hpp>
#include <string>
#include <iostream>

static void boost_unordered_using_unordered_set() noexcept {
  std::cout << "--------- [unordered.using_unordered_set] ---------" << std::endl;
  using unordered_set = boost::unordered_set<std::string>;

  unordered_set set;
  set.emplace("cat");
  set.emplace("shark");
  set.emplace("spider");

  for (const std::string& s : set)
    std::cout << "[demo.unordered] " << s << std::endl;

  std::cout << "[demo.unordered] " << set.size() << std::endl;
  std::cout << "[demo.unordered] " << set.max_size() << std::endl;

  std::cout << "[demo.unordered] " << std::boolalpha << (set.find("cat") != set.end()) << std::endl;
  std::cout << "[demo.unordered] " << set.count("shark") << std::endl;
}

void boost_unordered() noexcept {
  std::cout << "========= [unordered] =========" << std::endl;

  boost_unordered_using_unordered_set();
}
