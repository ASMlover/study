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
#include <stdio.h>
#include <algorithm>
#include <limits>
#include <chaos/logging/LogStream.h>

namespace chaos {

const char kDigits[] = "9876543210123456789";
const char* kZero = kDigits + 9;
static_assert(sizeof(kDigits) == 20, "wrong number of digits");

const char kDigitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(kDigitsHex) == 17, "wrong hex number of digits");

template <typename T>
size_t convert(char buf[], T value) {
  T i = value;
  char* p = buf;

  do {
    int n = static_cast<int>(i % 10);
    i /= 10;
    *p++ = kZero[n];
  } while (0 != i);

  if (value < 0)
    *p++ = '-';
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

size_t convert_hex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p = buf;

  do {
    int n = static_cast<int>(i % 16);
    i /= 16;
    *p++ = kDigitsHex[n];
  } while (0 != i);
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <int N> void FixedBuffer<N>::cookie_start(void) {
}

template <int N> void FixedBuffer<N>::cookie_end(void) {
}

template <int N>
const char* FixedBuffer<N>::debug_string(void) {
  return (current_ = '\0', data_);
}

}
