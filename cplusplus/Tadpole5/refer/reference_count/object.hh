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

#include <iostream>
#include <vector>
#include <tadpole/common/common.hh>

namespace tadpole::gc {

enum class ObjType : u8_t {
  INT,
  PAIR,
};

class BaseObject;
using ObjectRef = BaseObject*;

class BaseObject : private UnCopyable {
  ObjType type_;
  u32_t rc_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline u32_t rc() const noexcept { return rc_; }
  inline u32_t inc_ref() noexcept { return ++rc_; }
  inline u32_t dec_ref() noexcept { return --rc_; }

  virtual sz_t get_size() const noexcept = 0;
  virtual const char* get_name() const noexcept = 0;
  virtual std::vector<ObjectRef*> pointers() noexcept { return std::vector<ObjectRef*>(); }
};

inline void object_incref(BaseObject* o) noexcept {
  if (o != nullptr)
    o->inc_ref();
}

inline void object_decref(BaseObject* o) noexcept {
  if (o != nullptr && o->dec_ref() == 0) {
    for (ObjectRef* field : o->pointers())
      object_decref(*field);

    delete o;
  }
}

inline void object_write(ObjectRef* target, BaseObject* source) noexcept {
  object_incref(source);
  object_decref(*target);
  *target = source;
}

class IntObject final : public BaseObject {
  int value_{};
public:
  IntObject(int value = 0) noexcept : BaseObject(ObjType::INT), value_(value) {}
  virtual ~IntObject() {
    std::cout << "[" << this << "] reclaim object: `" << get_name() << "`" << std::endl;
  }

  inline int value() const noexcept { return value_; }
  inline void set_value(int value = 0) noexcept { value_ = value; }

  virtual sz_t get_size() const noexcept override { return sizeof(IntObject); }
  virtual const char* get_name() const noexcept override { return "<int>"; }

  static IntObject* create(int value = 0) noexcept;
};

class PairObject final : public BaseObject {
  BaseObject* first_{};
  BaseObject* second_{};
public:
  PairObject(BaseObject* first = nullptr, BaseObject* second = nullptr) noexcept
    : BaseObject(ObjType::PAIR), first_(first), second_(second) {
    object_incref(first_);
    object_incref(second_);
  }
  virtual ~PairObject() {
    std::cout << "[" << this << "] reclaim object: `" << get_name() << "`" << std::endl;
  }

  inline BaseObject* first() const noexcept { return first_; }
  inline void set_first(BaseObject* first = nullptr) noexcept {
    object_write(&first_, first);
  }
  inline BaseObject* second() const noexcept { return second_; }
  inline void set_second(BaseObject* second = nullptr) noexcept {
    object_write(&second_, second);
  }

  virtual sz_t get_size() const noexcept override { return sizeof(PairObject); }
  virtual const char* get_name() const noexcept override { return "<pair>"; }

  virtual std::vector<ObjectRef*> pointers() noexcept override;

  static PairObject* create(BaseObject* first = nullptr, BaseObject* second = nullptr) noexcept;
};

}
