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
#pragma once

#include "common.h"

namespace gc {

struct MemoryHeader {
  enum {INVALID, INT, PAIR};
  std::uint8_t type{INVALID};
  bool marked{};
  std::size_t size{};
};

class Object : public MemoryHeader, private UnCopyable {
public:
  const char* name(void) { return "Object"; }
};

class Int : public Object {
  int value_{};
public:
  Int(void) { type = MemoryHeader::INT; }
  void value(int value = 0) { value_ = value; }
  int value(void) const { return value_; }
  const char* name(void) const { return "Int"; }
};

class Pair : public Object {
  Object* first_{};
  Object* second_{};
public:
  Pair(void) { type = MemoryHeader::PAIR; }
  void first(Object* first) { first_ = first; }
  Object* first(void) const { return first_; }
  void second(Object* second) { second_ = second; }
  Object* second(void) const { return second_; }
  const char* name(void) const { return "Pair"; }
};

constexpr std::size_t kAlignment = sizeof(void*);
constexpr std::size_t roundup(std::size_t n) {
  return (n + kAlignment - 1) & ~(kAlignment - 1);
}
constexpr std::size_t kMinObjSize = roundup(sizeof(Int));

}
