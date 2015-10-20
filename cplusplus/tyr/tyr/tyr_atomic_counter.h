// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_ATOMIC_COUNTER_HEADER_H__
#define __TYR_ATOMIC_COUNTER_HEADER_H__

namespace tyr {

class AtomicCounter : private UnCopyable {
  std::atomic<int> counter_;
public:
  explicit AtomicCounter(int counter = 0) tyr_noexcept
    : counter_(counter) {
  }

  int Counter(void) const tyr_noexcept {
    return counter_.load();
  }

  explicit operator int(void) const tyr_noexcept {
    return counter_.load();
  }

  int operator++(void) {
    return counter_.fetch_add(1) + 1;
  }

  int operator++(int) {
    return counter_.fetch_add(1);
  }

  int operator--(void) {
    return counter_.fetch_sub(1) - 1;
  }

  int operator--(int) {
    return counter_.fetch_sub(1);
  }
};

}

#endif  // __TYR_ATOMIC_COUNTER_HEADER_H__
