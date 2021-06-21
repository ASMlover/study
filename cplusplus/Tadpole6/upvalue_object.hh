// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
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

#include "object.hh"
#include "value.hh"

namespace tadpole {

class UpvalueObject final : public BaseObject {
  Value* value_{};
  Value closed_{};
  UpvalueObject* next_{};
 public:
  UpvalueObject(Value* value, UpvalueObject* next = nullptr) noexcept
    : BaseObject(ObjType::UPVALUE), value_(value), next_(next) {
  }

  inline Value* value() const noexcept { return value_; }
  inline Value* value_asptr() const noexcept { return value_; }
  inline const Value& value_asref() const noexcept { return *value_; }
  inline void set_value(Value* value) noexcept { value_ = value; }
  inline void set_value(const Value& value) noexcept { *value_ = value; }
  inline const Value& closed() const noexcept { return closed_; }
  inline Value* closed_asptr() const noexcept { return const_cast<Value*>(&closed_); }
  inline const Value& closed_asref() const noexcept { return closed_; }
  inline void set_closed(const Value& closed) noexcept { closed_ = closed; }
  inline void set_closed(Value* closed) noexcept { closed_ = *closed; }
  inline UpvalueObject* next() const noexcept { return next_; }
  inline void set_next(UpvalueObject* next = nullptr) noexcept { next_ = next;}

  virtual str_t stringify() const override;
  virtual void iter_children(ObjectVisitor&& visitor) override;

  static UpvalueObject* create(Value* value, UpvalueObject* next = nullptr);
};

}
