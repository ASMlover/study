// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "el_base.h"
#include "el_object.h"
#include "el_array_object.h"
#include "el_block_object.h"
#include "el_dynamic_object.h"
#include "el_fiber_object.h"
#include "el_number_object.h"
#include "el_string_object.h"
#include "el_interpreter.h"
#include "el_fiber.h"

namespace el {

Value::Value(const Value& other)
  : obj_(other.obj_) {
  if (nullptr != obj_)
    ++obj_->ref_count_;
}

Value Value::GetField(int name) const {
  DynamicObject* dynamic = AsDynamic();
  if (nullptr == dynamic)
    return Value();

  return dynamic->GetField(name);
}

void Value::SetField(int name, const Value& value) const {
  DynamicObject* dynamic = AsDynamic();
  if (nullptr != dynamic)
    dynamic->SetField(name, value);
}

void Value::SendMessage(
    Fiber& fiber, StringId message_id, const ArgReader& args) const {
}

Value& Value::operator=(const Value& other) {
  if (&other != this) {
    Clear();
    obj_ = other.obj_;
    if (nullptr != obj_)
      ++obj_->ref_count_;
  }

  return *this;
}

void Value::Clear(void) {
  if (nullptr != obj_) {
    --obj_->ref_count_;
    if (0 == obj_->ref_count_)
      delete obj_;
    obj_ = nullptr;
  }
}

const Value& Value::Parent(void) const {
  return obj_->Parent();
}

void Value::Trace(std::ostream& cout) const {
  if (IsNil())
    cout << "(nil)";
  else
    obj_->Trace(cout);
}

double Value::AsNumber(void) const {
  return obj_->AsNumber();
}

String Value::AsString(void) const {
  return obj_->AsString();
}

ArrayObject* Value::AsArray(void) const {
  return obj_->AsArray();
}

BlockObject* Value::AsBlock(void) const {
  return obj_->AsBlock();
}

DynamicObject* Value::AsDynamic(void) const {
  return obj_->AsDynamic();
}

FiberObject* Value::AsFiber(void) const {
  return obj_->AsFiber();
}

}
