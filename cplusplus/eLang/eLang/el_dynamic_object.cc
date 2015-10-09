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
#include "el_dynamic_object.h"

namespace el {

void DynamicObject::Trace(std::ostream& stream) const {
  stream << name_;
}

Value DynamicObject::FindMethod(StringId message_id) {
  Value method;
  if (methods_.Find(message_id, method))
    return method;

  return Value();
}

PrimitiveFun DynamicObject::FindPrimitive(StringId message_id) {
  PrimitiveFun primitive;
  if (primitives_.Find(message_id, primitive))
    return primitive;

  return nullptr;
}

Value DynamicObject::GetField(StringId name) {
  DynamicObject* object = this;
  while (true) {
    Value field;
    if (object->fields_.Find(name, field))
      return field;

    if (object->Parent().IsNil())
      break;

    object = object->Parent().AsDynamic();
    if (nullptr == object)
      break;
  }

  return Value();
}

void DynamicObject::SetField(StringId name, const Value& value) {
  fields_.Insert(name, value);
}

void DynamicObject::AddMethod(StringId message_id, const Value& method) {
  methods_.Insert(message_id, method);
}

void DynamicObject::AddPrimitive(StringId message_id, PrimitiveFun method) {
  primitives_.Insert(message_id, method);
}

}
