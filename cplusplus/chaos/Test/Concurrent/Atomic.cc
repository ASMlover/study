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
#include <Chaos/Concurrent/Atomic.h>
#include <Chaos/Unittest/TestHarness.h>

CHAOS_TEST(AtomicI16, Chaos::FakeTester) {
  Chaos::AtomicI16 a0;
  CHAOS_CHECK_EQ(a0.get(), 0);
  CHAOS_CHECK_EQ(a0.fetch_add(1), 0);
  CHAOS_CHECK_EQ(a0.get(), 1);
  CHAOS_CHECK_EQ(a0++, 1);
  CHAOS_CHECK_EQ(a0.get(), 2);
  CHAOS_CHECK_EQ(++a0, 3);
  CHAOS_CHECK_EQ(a0.set(5), 3);
  CHAOS_CHECK_EQ(a0.fetch_sub(1), 5);
  CHAOS_CHECK_EQ(a0.get(), 4);
  CHAOS_CHECK_EQ(a0--, 4);
  CHAOS_CHECK_EQ(a0.get(), 3);
  CHAOS_CHECK_EQ(--a0, 2);
}

CHAOS_TEST(AtomicI32, Chaos::FakeTester) {
  Chaos::AtomicI32 a0;
  CHAOS_CHECK_TRUE(a0.get() == 0);
  CHAOS_CHECK_TRUE(a0.fetch_add(1) == 0);
  CHAOS_CHECK_TRUE(a0.get() == 1);
  CHAOS_CHECK_TRUE(a0++ == 1);
  CHAOS_CHECK_TRUE(a0.get() == 2);
  CHAOS_CHECK_TRUE(++a0 == 3);
  CHAOS_CHECK_TRUE(a0.set(5) == 3);
  CHAOS_CHECK_TRUE(a0.fetch_sub(1) == 5);
  CHAOS_CHECK_TRUE(a0.get() == 4);
  CHAOS_CHECK_TRUE(a0-- == 4);
  CHAOS_CHECK_TRUE(a0.get() == 3);
  CHAOS_CHECK_TRUE(--a0 == 2);
}

CHAOS_TEST(AtomicI64, Chaos::FakeTester) {
  Chaos::AtomicI64 a0;
  CHAOS_CHECK_EQ(a0.get(), 0);
  CHAOS_CHECK_EQ(a0.fetch_add(1), 0);
  CHAOS_CHECK_EQ(a0.get(), 1);
  CHAOS_CHECK_EQ(a0++, 1);
  CHAOS_CHECK_EQ(a0.get(), 2);
  CHAOS_CHECK_EQ(++a0, 3);
  CHAOS_CHECK_EQ(a0.set(5), 3);
  CHAOS_CHECK_EQ(a0.fetch_sub(1), 5);
  CHAOS_CHECK_EQ(a0.get(), 4);
  CHAOS_CHECK_EQ(a0--, 4);
  CHAOS_CHECK_EQ(a0.get(), 3);
  CHAOS_CHECK_EQ(--a0, 2);
}
