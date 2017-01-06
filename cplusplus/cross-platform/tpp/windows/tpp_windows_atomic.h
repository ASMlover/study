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
#ifndef TPP_WINDOWS_ATOMIC_H_
#define TPP_WINDOWS_ATOMIC_H_

#include <Windows.h>
#include <stdint.h>

namespace tpp {

template <typename T, int Bytes>
class AtomicBase {
protected:
  volatile T value_{};
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
};

template <typename T>
class AtomicBase<T, 1> {
protected:
  volatile T value_{};
public:
  T get(void) {
    return static_cast<T>(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&value_), 0, 0));
  }

  T set(T desired) {
    return static_cast<T>(InterlockedExchange8(reinterpret_cast<volatile char*>(&value_), desired));
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
};

template <typename T>
class AtomicBase<T, 2> {
protected:
  volatile T value_{};
public:
  T get(void) {
    return static_cast<T>(InterlockedCompareExchange16(reinterpret_cast<volatile short*>(&value_), 0, 0));
  }

  T set(T desired) {
    return static_cast<T>(InterlockedExchange16(reinterpret_cast<volatile short*>(&value_), desired));
  }

  T fetch_add(T arg) {
    return static_cast<T>(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&value_), arg));
  }

  T fetch_sub(T arg) {
    return static_cast<T>(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(&value_), -arg));
  }

  T operator++(void) {
    return static_cast<T>(InterlockedIncrement16(reinterpret_cast<volatile short*>(&value_)));
  }

  T operator--(void) {
    return static_cast<T>(InterlockedDecrement(reinterpret_cast<volatile LONG*>(&value_)));
  }
};

template <typename T>
class AtomicBase<T, 8> {
protected:
  volatile T value_{};
public:
  T get(void) {
    return static_cast<T>(InterlockedCompareExchange64(reinterpret_cast<volatile LONGLONG*>(&value_), 0, 0));
  }

  T set(T desired) {
    return static_cast<T>(InterlockedExchange64(reinterpret_cast<volatile LONGLONG*>(&value_), desired));
  }

  T fetch_add(T arg) {
    return static_cast<T>(InterlockedExchangeAdd64(reinterpret_cast<volatile LONGLONG*>(&value_), arg));
  }

  T fetch_sub(T arg) {
    return static_cast<T>(InterlockedExchangeAdd64(reinterpret_cast<volatile LONGLONG*>(&value_), -arg));
  }

  T operator++(void) {
    return static_cast<T>(InterlockedIncrement64(reinterpret_cast<volatile LONGLONG*>(&value_)));
  }

  T operator--(void) {
    return static_cast<T>(InterlockedDecrement64(reinterpret_cast<volatile LONGLONG*>(&value_)));
  }
};

template <typename T>
class Atomic : public AtomicBase<T, sizeof(T)> {
  static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
      "Atomic value type's size must be `1`, `2`, `4` or `8`");
public:
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

#endif // TPP_WINDOWS_ATOMIC_H_
