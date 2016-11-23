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
#include "co/co_unittest.h"
#include "basic/tpp_memory.h"
#include "tpp_types.h"
#include "tpp_thread.h"
#include "tpp_backtrace.h"
#include "tpp_corotine.h"
#include "tpp_intrusive_ptr.h"

#define TPP_LITTLE_ENDIAN (0x41424344UL)
#define TPP_BIG_ENDIAN    (0x44434241UL)
#define TPP_PDP_ENDIAN    (0x42414443UL)
#define TPP_BYTE_ORDER    (unsigned int)('ABCD')

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

class IntegerEx {
public:
  IntegerEx(void) {
    std::cout << "########## IntegerEx::IntegerEx ##########" << std::endl;
  }

  ~IntegerEx(void) {
    std::cout << "########## IntegerEx::~IntegerEx ##########" << std::endl;
  }
};

int main(int argc, char* argv[]) {
  TPP_UNUSED(argc);
  TPP_UNUSED(argv);

  std::cout << "Hello, `tpp` !" << std::endl;

  {
    // byte order checking
    if (TPP_BYTE_ORDER == TPP_LITTLE_ENDIAN)
      std::cout << "little endian" << std::endl;
    else if (TPP_BYTE_ORDER == TPP_BIG_ENDIAN)
      std::cout << "big endian" << std::endl;
    else if (TPP_BYTE_ORDER == TPP_PDP_ENDIAN)
      std::cout << "pdp endian" << std::endl;
    else
      std::cout << "error endian" << std::endl;
  }

  {
    // thread sample
    tpp::Thread t(thread_closure, nullptr);
    t.join();
  }
  {
    // backtrace sample
    std::string bt;
    tpp::__libtpp_backtrace(bt);
    std::cout << bt << std::endl;
  }
  {
    // intrusive pointer sample
    tpp::IntrusivePtr<Integer> p(new Integer());
  }
  {
    // shared pointer sample
    tpp::basic::SharedPtr<IntegerEx> sp1 = tpp::basic::make_shared<IntegerEx>();
    tpp::basic::WeakPtr<IntegerEx> wp1(sp1);
    tpp::basic::SharedPtr<IntegerEx> sp2 = sp1;
    std::cout << "use_count: " << wp1.lock().use_count() << std::endl;

    tpp::basic::SharedArray<IntegerEx> sa1(new IntegerEx[3]);
  }
  {
    // non-recursive mutex sample
    extern void test_NonRecursiveMutex(void);
    test_NonRecursiveMutex();
  }
  co::test::run_all_unittests();

  {
    // corotine sample
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
