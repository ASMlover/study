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

#include "common.hh"

namespace wrencc {

class WrenVM;

enum class ValueType : u8_t {
  UNDEFINED,

  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  OBJECT,
};

enum class ObjType : u8_t {
  STRING,
  LIST,
  RANGE,
  MAP,
  MODULE,
  FUNCTION,
  FOREIGN,
  UPVALUE,
  CLOSURE,
  FIBER,
  CLASS,
  INSTANCE,
};

class StringObject;
class ListObject;
class RangeObject;
class MapObject;
class ModuleObject;
class FunctionObject;
class ForeignObject;
class UpvalueObject;
class ClosureObject;
class FiberObject;
class ClassObject;
class InstanceObject;

struct BaseObject : private UnCopyable {
  ObjType type_;
  bool is_darken_{};
  ClassObject* cls_{};
public:
  BaseObject(ObjType type, ClassObject* cls = nullptr) noexcept
    : type_(type), cls_(cls) {
  }
  virtual ~BaseObject() noexcept {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_darken() const noexcept { return is_darken_; }
  inline void set_darken(bool darken = true) noexcept { is_darken_ = darken; }
  inline ClassObject* cls() const noexcept { return cls_; }
  inline void set_cls(ClassObject* cls) noexcept { cls_ = cls; }

  virtual bool is_equal(BaseObject* r) const noexcept { return false; }
  virtual str_t stringify() const noexcept { return "<object>"; }
  virtual void gc_blacken(WrenVM& vm) noexcept {}
  virtual void initialize(WrenVM& vm) noexcept {}
  virtual void finalize(WrenVM& vm) noexcept {}

  virtual u32_t hasher() const noexcept {
    ASSERT(false, "only immutable objects can be hashed");
    return 0;
  }
};

class ObjValue final : public Copyable {
  ValueType type_{ValueType::UNDEFINED};

  union {
    double num{};
    BaseObject* obj;
  } as_;
};

}
