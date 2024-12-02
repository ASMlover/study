// Copyright (c) 2024 ASMlover. All rights reserved.
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
#define BOOST_SP_USE_QUICK_ALLOCATOR
#include <iostream>
#include <ctime>
#include <boost/shared_ptr.hpp>
#include <Windows.h>

void boost_shared_ptr() noexcept {
  std::cout << "========= [shared_ptr] =========" << std::endl;

  boost::shared_ptr<int> p1{new int{1}};
  std::cout << *p1 << std::endl;

  boost::shared_ptr<int> p2{p1};
  p1.reset(new int{2});
  std::cout << *p1.get() << std::endl;
  p1.reset();

  std::cout << std::boolalpha << static_cast<bool>(p2) << std::endl;

  std::cout << "--------- [shared_ptr.OpenProcess] ---------" << std::endl;
  boost::shared_ptr<void> handle(OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId()), CloseHandle);

  std::cout << "--------- [shared_ptr.USE_QUICK_ALLOCATOR] ---------" << std::endl;
  {
    boost::shared_ptr<int> p;
    std::time_t then = std::time(nullptr);
    for (int i = 0; i < 10000000; ++i)
      p.reset(new int{i});
    std::time_t now = std::time(nullptr);
    std::cout << now - then << std::endl;
  }
}
