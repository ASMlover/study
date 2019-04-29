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
#include "base_object.hh"

namespace sparrow {

enum class ValueType {
  UNKNOWN,
  NIL,
  FALSE,
  TRUE,
  NUMERIC,
  OBJECT,
};

class Value : public Copyable {
  ValueType type_{ValueType::UNKNOWN};
  union {
    double numeric;
    BaseObject* object;
  } as_{};

  inline bool is_objtype(ObjType type) const {
    return is_object() && as_.object->type() == type;
  }
public:
  Value(void) noexcept
    : type_(ValueType::NIL) {
  }

  Value(nil_t) noexcept
    : type_(ValueType::NIL) {
  }

  Value(bool b) noexcept
    : type_(b ? ValueType::TRUE : ValueType::FALSE) {
  }

  Value(double d) noexcept
    : type_(ValueType::NUMERIC) {
    as_.numeric = d;
  }

  Value(BaseObject* obj) noexcept
    : type_(ValueType::OBJECT) {
    as_.object = obj;
  }

  Value(const Value& r) noexcept
    : type_(r.type_) {
    switch (type_) {
    case ValueType::NUMERIC: as_.numeric = r.as_.numeric; break;
    case ValueType::OBJECT: as_.object = r.as_.object; break;
    }
  }

  Value(Value&& r) noexcept
    : type_(std::move(r.type_)) {
    switch (type_) {
    case ValueType::NUMERIC: as_.numeric = std::move(r.as_.numeric); break;
    case ValueType::OBJECT: as_.object = std::move(r.as_.object); break;
    }
  }

  Value& operator=(const Value& r) noexcept {
    if (this != &r) {
      type_ = r.type_;
      switch (type_) {
      case ValueType::NUMERIC: as_.numeric = r.as_.numeric; break;
      case ValueType::OBJECT: as_.object = r.as_.object; break;
      }
    }
    return *this;
  }

  Value& operator=(Value&& r) noexcept {
    if (this != &r) {
      type_ = std::move(r.type_);
      switch (type_) {
      case ValueType::NUMERIC: as_.numeric = std::move(r.as_.numeric); break;
      case ValueType::OBJECT: as_.object = std::move(r.as_.object); break;
      }
    }
    return *this;
  }

  inline bool operator>(const Value& r) const {
    return as_.numeric > r.as_.numeric;
  }

  inline bool operator>=(const Value& r) const {
    return as_.numeric >= r.as_.numeric;
  }

  inline bool operator<(const Value& r) const {
    return as_.numeric < r.as_.numeric;
  }

  inline bool operator<=(const Value& r) const {
    return as_.numeric <= r.as_.numeric;
  }

  bool operator==(const Value& r) const;
  bool operator!=(const Value& r) const;
  bool is_truthy(void) const;
  str_t stringify(void) const;

  inline bool is_unknown(void) const { return type_ == ValueType::UNKNOWN; }
  inline bool is_nil(void) const { return type_ == ValueType::NIL; }
  inline bool is_true(void) const { return type_ == ValueType::TRUE; }
  inline bool is_false(void) const { return type_ == ValueType::FALSE; }
  inline bool is_numeric(void) const { return type_ == ValueType::NUMERIC; }
  inline bool is_object(void) const { return type_ == ValueType::OBJECT; }

  inline double as_numeric(void) const { return as_.numeric; }
  inline BaseObject* as_object(void) const { return as_.object; }

  inline bool is_class(void) const { return is_objtype(ObjType::CLASS); }
  inline bool is_module(void) const { return is_objtype(ObjType::MODULE); }
  inline bool is_string(void) const { return is_objtype(ObjType::STRING); }
  inline bool is_upvalue(void) const { return is_objtype(ObjType::UPVALUE); }
  inline bool is_function(void) const { return is_objtype(ObjType::FUNCTION); }
  inline bool is_closure(void) const { return is_objtype(ObjType::CLOSURE); }

  ClassObject* as_class(void) const;
  ModuleObject* as_module(void) const;
  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
  UpvalueObject* as_upvalue(void) const;
  FunctionObject* as_function(void) const;
  ClosureObject* as_closure(void) const;
};

std::ostream& operator<<(std::ostream& out, const Value& value);

}
