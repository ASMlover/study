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

#include <cstddef>
#include <Chaos/Types.h>
#include "memory_header.h"

namespace gc {

class BaseObject : public MemoryHeader, private Chaos::UnCopyable {
public:
  virtual ~BaseObject(void) {}
  virtual const char* get_name(void) const { return "BaseObject"; }
  virtual std::size_t get_size(void) const { return sizeof(*this); }
};

class Int : public BaseObject {
  int value_{};
public:
  Int(void) { set_type(MemoryHeader::INT); }
  Int(Int&& o)
    : value_(o.value_) {
    set_type(o.type()); set_forward(o.forward());
  }
  virtual const char* get_name(void) const override { return "Int"; }
  virtual std::size_t get_size(void) const override { return sizeof(*this); }
  void set_value(int value) { value_ = value; }
  int value(void) const { return value_; }
};

class Pair : public BaseObject {
  BaseObject* first_{};
  BaseObject* second_{};
public:
  Pair(void) { set_type(MemoryHeader::PAIR); }
  Pair(Pair&& o)
    : first_(o.first_), second_(o.second_) {
    set_type(o.type()); set_forward(o.forward());
  }
  virtual const char* get_name(void) const override { return "Pair"; }
  virtual std::size_t get_size(void) const override { return sizeof(*this); }
  void set_first(BaseObject* obj) { first_ = obj; }
  BaseObject* first(void) const { return first_; }
  void set_second(BaseObject* obj) { second_ = obj; }
  BaseObject* second(void) const { return second_; }
};

inline BaseObject* as_object(void* p) {
  return reinterpret_cast<BaseObject*>(p);
}

inline byte_t* as_pointer(BaseObject* obj) {
  return reinterpret_cast<byte_t*>(obj);
}

}
