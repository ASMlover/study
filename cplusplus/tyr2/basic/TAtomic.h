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
#ifndef __TYR_BASIC_ATOMIC_HEADER_H__
#define __TYR_BASIC_ATOMIC_HEADER_H__

#include "TTypes.h"

namespace tyr { namespace basic {

template <typename T>
class AtomicInt : private UnCopyable {
  volatile T value_{};

  static_assert(sizeof(T) == 1
      || sizeof(T) == 2
      || sizeof(T) == 4
      || sizeof(T) == 8
      , "AtomicInt size must be `1`, `2`, `4`, `8`");
public:
  AtomicInt(void) = default;

  T get(void) {
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T set(T desired) {
    return __sync_lock_test_and_set(&value_, desired);
  }

  T fetch_add(T arg) {
    return __sync_fetch_and_add(&value_, arg);
  }

  T fetch_sub(T arg) {
    return __sync_fetch_and_sub(&value_, arg);
  }

  T operator+=(T arg) {
    return fetch_add(arg) + arg;
  }

  T operator-=(T arg) {
    return fetch_sub(arg) - arg;
  }

  T operator++(void) {
    return fetch_add(1) + 1;
  }

  T operator--(void) {
    return fetch_sub(1) - 1;
  }

  T operator++(int) {
    return fetch_add(1);
  }

  T operator--(int) {
    return fetch_sub(1);
  }
};

typedef AtomicInt<int32_t> AtomicI32;
typedef AtomicInt<int64_t> AtomicI64;
typedef AtomicInt<uint32_t> AtomicU32;
typedef AtomicInt<uint64_t> AtomicU64;

}}

#endif // __TYR_BASIC_ATOMIC_HEADER_H__
