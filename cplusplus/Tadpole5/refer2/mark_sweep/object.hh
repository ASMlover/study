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

#include "common.hh"
#include "memory_header.hh"

namespace tadpole::gc {

enum ObjType : u8_t {
  INT,
  PAIR,
};

class BaseObject : private UnCopyable {
  ObjType type_;
public:
  BaseObject(ObjType type) noexcept : type_(type) {
    as_memory(this)->set_as_object();
  }
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_marked() const noexcept { return as_memory(this)->is_marked(); }
  inline void set_marked(bool marked = true) noexcept {
    if (marked)
      as_memory<BaseObject>(this)->set_marked();
    else
      as_memory<BaseObject>(this)->unset_marked();
  }

  virtual sz_t get_size() const { return get_objsize<BaseObject>(); }
  virtual const char* get_name() const { return "<object>"; }

  inline const char* type_asstr() const noexcept {
    switch (type_) {
    case ObjType::INT: return "<INT>";
    case ObjType::PAIR: return "<PAIR>";
    }
    return "<UNKNOWN>";
  }
};

class IntObject final : public BaseObject {
  int value_{};
public:
  IntObject(int value = 0) noexcept
    : BaseObject(ObjType::INT), value_(value) {
  }

  inline int value() const noexcept { return value_; }
  inline void set_value(int value) noexcept { value_ = value; }

  virtual sz_t get_size() const override { return get_objsize<IntObject>(); }
  virtual const char* get_name() const override { return "<int>"; }
};

class PairObject final : public BaseObject {
  BaseObject* first_{};
  BaseObject* second_{};
public:
  PairObject(BaseObject* first = nullptr, BaseObject* second = nullptr) noexcept
    : BaseObject(ObjType::PAIR), first_(first), second_(second) {
  }

  inline BaseObject* first() const noexcept { return first_; }
  inline void set_first(BaseObject* first = nullptr) noexcept { first_ = first; }
  inline BaseObject* second() const noexcept { return second_; }
  inline void set_second(BaseObject* second = nullptr) noexcept { second_ = second; }

  virtual sz_t get_size() const override { return get_objsize<PairObject>(); }
  virtual const char* get_name() const override { return "<pair>"; }
};

}
