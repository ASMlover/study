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
#include <iostream>

static constexpr std::size_t GC_HEAPSIZE = 512 << 10;
static constexpr std::size_t GC_ALIGNSIZE = sizeof(void*);
static inline std::size_t GC_ROUNDUP(std::size_t bytes) {
  return (bytes + GC_ALIGNSIZE - 1) & ~(GC_ALIGNSIZE - 1);
}

struct Object {
  enum {
    INT,
    PAIR,
  };
  std::uint8_t type{};
  bool marked{};
  std::uint16_t size;
  Object* next{};

  Object(void) {
    size = sizeof(*this);
  }
};

class Int : public Object {
  int value_{};
public:
  Int(void) {
    type = Object::INT;
  }

  void set_value(int value) {
    value_ = value;
  }

  int get_value(void) {
    return value_;
  }
};

class Pair : public Object {
  Object* first_{};
  Object* second_{};
public:
  Pair(void) {
    type = Object::PAIR;
  }

  void set_first(Object* obj) {
    first_ = obj;
  }

  void set_second(Object* obj) {
    second_ = obj;
  }

  Object* get_first(void) const {
    return first_;
  }

  Object* get_second(void) const {
    return second_;
  }
};

class GC {
  char* heaptr_{};
  char* allocptr_{};
public:
  GC(void) {
    heaptr_ = new char[GC_HEAPSIZE];
    allocptr_ = heaptr_;
  }

  ~GC(void) {
    delete [] heaptr_;
  }

  Object* new_int(int value) {
    Int* obj = new (allocptr_) Int;
    obj->set_value(value);

    return obj;
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  return 0;
}
