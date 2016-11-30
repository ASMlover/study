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
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Unittest/TestHarness.h>
#include <iostream>
#include <thread>

CHAOS_TEST(Mutex, Chaos::FakeTester) {
  Chaos::Mutex mtx;
  int counter = 0;

  CHAOS_CHECK_EQ(counter, 0);

  std::thread t([&] {
      Chaos::ScopedLock<Chaos::Mutex> guard(mtx);

      for (int i = 0; i < 10; ++i)
        std::cout << "Chaos::Mutex unittest, @i: " << ++counter << std::endl;
  });
  t.join();

  CHAOS_CHECK_EQ(counter, 10);
}

CHAOS_TEST(FastMutex, Chaos::FakeTester) {
  Chaos::FastMutex mtx;
  int counter = 0;

  CHAOS_CHECK_EQ(counter, 0);

  std::thread t([&] {
      Chaos::ScopedLock<Chaos::FastMutex> guard(mtx);

      for (int i = 0; i < 10; ++i)
        std::cout << "Chaos::FastMutex unittest, @i: " << ++counter << std::endl;
  });
  t.join();

  CHAOS_CHECK_EQ(counter, 10);
}

CHAOS_TEST(ScopedLock, Chaos::FakeTester) {
  Chaos::Mutex mtx;
  Chaos::ScopedLock<Chaos::Mutex> guard(mtx);

  CHAOS_CHECK_TRUE(guard.owned_lock());
  CHAOS_CHECK_TRUE(static_cast<bool>(guard));
  CHAOS_CHECK_EQ(guard.get_mutex(), &mtx);

  guard.unlock();
  guard.release();

  CHAOS_CHECK_TRUE(!guard.owned_lock());
  CHAOS_CHECK_TRUE(!static_cast<bool>(guard));
  CHAOS_CHECK_TRUE(guard.get_mutex() == nullptr);
}
