// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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
#include <tadpole/common/common.hh>
#include "gc.hh"

namespace tadpole::gc {

enum class ObjType : u8_t {
  INT,
  PAIR,
};

class BaseObject;
using ObjectRef = BaseObject*;

class BaseObject : private UnCopyable {
  ObjType type_;
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() noexcept {}

  inline ObjType type() const noexcept { return type_; }

  virtual sz_t get_size() const noexcept = 0;
  virtual const char* get_name() const noexcept = 0;
  virtual std::vector<ObjectRef*> pointers() noexcept { return std::vector<ObjectRef*>(); }
};

class IntObject final : public BaseObject {
  int value_{};
public:
  IntObject(int value = 0) noexcept : BaseObject(ObjType::INT), value_(value) {}
  virtual ~IntObject() noexcept {}

  inline int value() const noexcept { return value_; }
  inline void set_value(int value = 0) noexcept { value_ = value; }

  virtual sz_t get_size() const noexcept override { return sizeof(IntObject); }
  virtual const char* get_name() const noexcept override { return "<int>"; }

  static IntObject* create(int value = 0);
};

class PairObject final : public BaseObject {
  BaseObject* first_{};
  BaseObject* second_{};
public:
  PairObject(BaseObject* first = nullptr, BaseObject* second = nullptr) noexcept
    : BaseObject(ObjType::PAIR) {
    global_gc().set_object(&first_, first);
    global_gc().set_object(&second_, second);
  }
  virtual ~PairObject() noexcept {}

  inline BaseObject* first() const noexcept { return first_; }
  inline void set_first(BaseObject* first = nullptr) noexcept {
    global_gc().set_object(&first_, first);
  }

  inline BaseObject* second() const noexcept { return second_; }
  inline void set_second(BaseObject* second = nullptr) noexcept {
    global_gc().set_object(&second_, second);
  }

  virtual sz_t get_size() const noexcept override { return sizeof(PairObject); }
  virtual const char* get_name() const noexcept override { return "<pair>"; }
  virtual std::vector<ObjectRef*> pointers() noexcept override { return {&first_, &second_}; }

  static PairObject* create(BaseObject* first = nullptr, BaseObject* second = nullptr);
};

}
