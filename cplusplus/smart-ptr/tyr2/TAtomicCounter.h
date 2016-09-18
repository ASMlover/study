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
#ifndef TYR_ATOMIC_COUNTER_HEADER_H
#define TYR_ATOMIC_COUNTER_HEADER_H

namespace tyr {

class AtomicCounter : private UnCopyable {
  struct CounterT {
    mutable Mutex mtx;
    volatile uint32_t val;
  };

  CounterT counter_;
public:
  explicit AtomicCounter(uint32_t counter = 0) {
    counter_.val = counter;
  }

  uint32_t Counter(void) const {
    uint32_t r;
    {
      LockerGuard<Mutex> guard(counter_.mtx);
      r = counter_.val;
    }

    return r;
  }

  explicit operator uint32_t(void) const {
    return Counter();
  }

  uint32_t operator++(void) {
    LockerGuard<Mutex> guard(counter_.mtx);
    return ++counter_.val;
  }

  uint32_t operator++(int) {
    LockerGuard<Mutex> guard(counter_.mtx);
    return counter_.val++;
  }

  uint32_t operator--(void) {
    LockerGuard<Mutex> guard(counter_.mtx);
    return --counter_.val;
  }

  uint32_t operator--(int) {
    LockerGuard<Mutex> guard(counter_.mtx);
    return counter_.val--;
  }
};

}

#endif // TYR_ATOMIC_COUNTER_HEADER_H
