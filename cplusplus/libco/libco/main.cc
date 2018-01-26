// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <iostream>
#include "co_mutex.h"
#include "ext/lock_free_container.h"
#include "ext/ext_utils.h"

class Data {
public:
  Data(void) {
    std::cout << "Data::Data" << std::endl;
  }

  ~Data(void) {
    std::cout << "Data::~Data" << std::endl;
  }

  void show(void) {
    std::cout << "Data::show" << std::endl;
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << "hello, libco !!!" << std::endl;

  {
    co::FastMutex m;
    m.lock();
    m.unlock();
  }

  {
    ext::lock_free::Stack<Data> s;
    s.push(new Data());

    auto n = s.pop();
    n->value->show();
    delete n->value;
  }

  {
    std::cout << "HEX output is: " << ext::as_hex("Hello, world") << std::endl;
  }

  return 0;
}
