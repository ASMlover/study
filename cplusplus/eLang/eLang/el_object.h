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
#ifndef __EL_OBJECT_HEADER_H__
#define __EL_OBJECT_HEADER_H__

#include "el_arg_reader.h"

#define EL_PRIMITIVE(name)\
  Value name(Fiber& fiber, const Value& self, const ArgReader& args)

namespace el {

class Expr;
class ArrayObject;
class Block;
class BlockObject;
class DynamicObject;
class Environment;
class Fiber;
class FiberObject;
class Interpreter;
class Object;

typedef Value (*PrimitiveFun)(Fiber& fiber,
    const Value& value, const ArgReader& args);

class Value {
  Object* obj_;
public:
  Value(void)
    : obj_(nullptr) {
  }

  explicit Value(Object* obj)
    : obj_(obj) {
  }

  Value(const Value& other);
  ~Value(void) {
    Clear();
  }

  Value GetField(int name) const;
  void SetField(int name, const Value& value) const;
  Value SendMessage(
      Fiber& fiber, StringId message_id, const ArgReader& args) const;

  bool operator==(const Value& other) const {
    return obj_ == other.obj_;
  }

  bool operator!=(const Value& other) const {
    return obj_ != other.obj_;
  }

  Value& operator=(const Value& other);
  
  inline bool IsNil(void) const {
    return nullptr == obj_;
  }

  void Clear(void);
  const Value& Parent(void) const;
  void Trace(std::ostream& cout) const;

  double AsNumber(void) const;
  String AsString(void) const;
  ArrayObject* AsArray(void) const;
  BlockObject* AsBlock(void) const;
  DynamicObject* AsDynamic(void) const;
  FiberObject* AsFiber(void) const;
};

std::ostream& operator<<(std::ostream& cout, const Value& value);

class Object {
  friend class Value;
  Value parent_;
  int   ref_count_;
protected:
  Object(const Value& parent)
    : parent_(parent)
    , ref_count_(1) {
  }
public:
  virtual ~Object(void) {}

  virtual double AsNumber(void) const {
    return 0;
  }

  virtual String AsString(void) const {
    return "";
  }

  virtual ArrayObject* AsArray(void) {
    return nullptr;
  }

  virtual BlockObject* AsBlock(void) {
    return nullptr;
  }

  virtual DynamicObject* AsDynamic(void) {
    return nullptr;
  }

  virtual FiberObject* AsFiber(void) {
    return nullptr;
  }

  inline const Value& Parent(void) const {
    return parent_;
  }

  virtual void Trace(std::ostream& stream) const = 0;
};

}

#endif  // __EL_OBJECT_HEADER_H__
