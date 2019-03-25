// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <vector>
#include "common.hh"

namespace nyx {

using byte_t = std::uint8_t;

enum class ObjType {
  FORWARD,
  NUMERIC,
  PAIR
};

class Object : private UnCopyable {
  ObjType type_;
public:
  Object(ObjType t) : type_(t) {}
  virtual ~Object(void) {}

  inline ObjType get_type(void) const { return type_; }

  virtual std::size_t size(void) const = 0;
  virtual std::string stringify(void) const = 0;
};
using Value = Object*;

class Froward : public Object {
  Object* to_{};
public:
  Froward(void) : Object(ObjType::FORWARD) {}

  inline void set_to(Object* v) { to_ = v; }
  inline Object* to(void) const { return to_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
};

class Numeric : public Object {
  double value_{};
public:
  Numeric(void) : Object(ObjType::NUMERIC) {}

  inline void set_value(double v) { value_ = v; }
  inline double value(void) const { return value_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
};

class Pair : public Object {
  Value first_{};
  Value second_{};
public:
  Pair(void) : Object(ObjType::PAIR) {}

  inline void set_first(Value v) { first_ = v; }
  inline Value first(void) const { return first_; }
  inline void set_second(Value v) { second_ = v; }
  inline Value second(void) const { return second_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
};

class VM : private UnCopyable {
  byte_t* from_beg_{};
  byte_t* from_end_{};
  byte_t* to_beg_{};
  byte_t* to_end_{};
  std::vector<Value> stack_;

  static constexpr std::size_t kMaxHeap = 10 * 1024 * 1024;

  void initialize(void);
public:
  VM(void);

  void collect(void);

  void push(double value);
  void push(void);
  void pop(void);

  void print_stack(void);
};

}
