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
#ifndef __EL_DYNAMIC_OBJECT_HEADER_H__
#define __EL_DYNAMIC_OBJECT_HEADER_H__

#include "el_object.h"

namespace el {

class Interpreter;

class DynamicObject : public Object {
  String                     name_;
  IdDictionary<Value>        fields_;
  IdDictionary<Value>        methods_;
  IdDictionary<PrimitiveFun> primitives_;
public:
  DynamicObject(const Value& parent, const String& name)
    : Object(parent)
    , name_(name) {
  }

  explicit DynamicObject(const Value& parent)
    : Object(parent)
    , name_("object") {
  }

  virtual String AsString(void) const override {
    return name_;
  }

  virtual DynamicObject* AsDynamic(void) override {
    return this;
  }

  virtual void Trace(std::ostream& stream) const override;

  Value FindMethod(StringId message_id);
  PrimitiveFun FindPrimitive(StringId message_id);
  Value GetField(StringId name);
  void SetField(StringId name, const Value& value);
  void AddMethod(StringId message_id, const Value& method);
  void AddPrimitive(StringId message_id, PrimitiveFun method);
private:
  void InitializeScope(void);
};

}

#endif  // __EL_DYNAMIC_OBJECT_HEADER_H__
