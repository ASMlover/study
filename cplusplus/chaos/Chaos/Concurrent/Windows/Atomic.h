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
#ifndef CHAOS_CONCURRENT_WINDOWS_ATOMIC_H
#define CHAOS_CONCURRENT_WINDOWS_ATOMIC_H

#include <Windows.h>
#include <Chaos/UnCopyable.h>

namespace Chaos {

template <typename T>
class Atomic : private UnCopyable {
  volatile T value_{};

  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
      "Chaos::Atomic value type's size must be `1`, `2`, `4` or `8`");
public:
  T get(void) {
    return static_cast<T>(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&value_), 0, 0));
  }

  T set(T desired) {
    return static_cast<T>(InterlockedExchange(reinterpret_cast<volatile LONG*>(&value_), desired));
  }

  T fetch_add(T arg) {
    return static_cast<T>(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&value_), arg));
  }

  T fetch_sub(T arg) {
    return static_cast<T>(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&value_), -arg));
  }

  T operator++(void) {
    return static_cast<T>(InterlockedIncrement(reinterpret_cast<volatile LONG*>(&value_)));
  }

  T operator--(void) {
    return static_cast<T>(InterlockedDecrement(reinterpret_cast<volatile LONG*>(&value_)));
  }

  T operator++(int) {
    return fetch_add(1);
  }

  T operator--(int) {
    return fetch_sub(1);
  }

  T operator+=(T arg) {
    return fetch_add(arg) + arg;
  }

  T operator-=(T arg) {
    return fetch_sub(arg) - arg;
  }
};

}

#endif // CHAOS_CONCURRENT_WINDOWS_ATOMIC_H
