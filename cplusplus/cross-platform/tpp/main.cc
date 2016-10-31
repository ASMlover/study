// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <exception>
#include <iostream>
#include <string>
#include "tpp_types.h"
#include "tpp_thread.h"
#include "tpp_backtrace.h"
#include "tpp_corotine.h"
#include "tpp_intrusive_ptr.h"

void thread_closure(void* arg) {
  std::cout << "**************** thread_closure ************* " << arg << std::endl;
}

void corotine_closure(void* arg) {
  tpp::Corotine* co = (tpp::Corotine*)arg;
  auto c = co->running();
  for (auto i = 0; i < 5; ++i) {
    std::cout << "**************** corotine_closure ************* " << c << std::endl;
    co->yield();
  }
}

class Integer : public tpp::IntrusiveRefCounter<Integer, tpp::ThreadUnsafeCounter> {
public:
  Integer(void) {
    std::cout << "********** Integer::Integer **********" << std::endl;
  }

  ~Integer(void) {
    std::cout << "********** Integer::~Integer **********" << std::endl;
  }
};

int main(int argc, char* argv[]) {
  TPP_UNUSED(argc);
  TPP_UNUSED(argv);

  std::cout << "Hello, `tpp` !" << std::endl;

  tpp::Thread t(thread_closure, nullptr);
  t.join();

  std::string bt;
  tpp::__libtpp_backtrace(bt);
  std::cout << bt << std::endl;

  {
    tpp::IntrusivePtr<Integer> p(new Integer());
  }

  {
    tpp::Corotine co;
    auto c1 = co.create(corotine_closure, &co);
    auto c2 = co.create(corotine_closure, &co);

    while (tpp::CoStatus::DEAD != co.status(c1)
        && tpp::CoStatus::DEAD != co.status(c2)) {
      std::cout << "================ main corotine - corotines status >> " << (int)co.status(c1) << " : " << (int)co.status(c2) << std::endl;
      co.resume(c1);
      co.resume(c2);
    }
    std::cout << "================ return main thread ================" << std::endl;
  }

  return 0;
}
