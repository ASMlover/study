// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <functional>
#include <unordered_map>
#include <vector>
#include "Common.hh"
#include "Chunk.hh"
#include "Value.hh"

namespace ms {

enum class ObjectType : int {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_CLOSURE,
  OBJ_UPVALUE,
  OBJ_CLASS,
  OBJ_INSTANCE,
  OBJ_BOUND_METHOD,
  OBJ_MODULE,
};

class Object {
  ObjectType type_;
  bool is_marked_{false};
  Object* next_{nullptr};

public:
  explicit Object(ObjectType type) noexcept : type_(type) {}
  virtual ~Object() = default;

  virtual str_t stringify() const noexcept = 0;
  virtual void trace_references() noexcept {}
  virtual sz_t size() const noexcept = 0;

  ObjectType type() const noexcept { return type_; }
  bool is_marked() const noexcept { return is_marked_; }
  void set_marked(bool marked) noexcept { is_marked_ = marked; }
  Object* next() const noexcept { return next_; }
  void set_next(Object* next) noexcept { next_ = next; }
};

template <typename T>
inline T* as_obj(Object* obj) noexcept {
  return static_cast<T*>(obj);
}

template <typename T>
inline const T* as_obj(const Object* obj) noexcept {
  return static_cast<const T*>(obj);
}

inline bool is_obj_type(const Value& value, ObjectType type) noexcept {
  return value.is_object() && value.as_object()->type() == type;
}

// --- ObjString ---
class ObjString final : public Object {
  str_t value_;
  u32_t hash_{0};

public:
  ObjString(str_t value, u32_t hash) noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;

  const str_t& value() const noexcept { return value_; }
  u32_t hash() const noexcept { return hash_; }

  static u32_t hash_string(cstr_t chars, sz_t length) noexcept;
};

// --- ObjFunction ---
class ObjFunction final : public Object {
  int arity_{0};
  int upvalue_count_{0};
  Chunk chunk_;
  ObjString* name_{nullptr};

public:
  ObjFunction() noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  int arity() const noexcept { return arity_; }
  void set_arity(int arity) noexcept { arity_ = arity; }
  void increment_arity() noexcept { arity_++; }
  int upvalue_count() const noexcept { return upvalue_count_; }
  int increment_upvalue_count() noexcept { return upvalue_count_++; }
  Chunk& chunk() noexcept { return chunk_; }
  const Chunk& chunk() const noexcept { return chunk_; }
  ObjString* name() const noexcept { return name_; }
  void set_name(ObjString* name) noexcept { name_ = name; }
};

// --- ObjNative ---
using NativeFn = std::function<Value(int arg_count, Value* args)>;

class ObjNative final : public Object {
  NativeFn function_;

public:
  explicit ObjNative(NativeFn function) noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;

  NativeFn& function() noexcept { return function_; }
};

// --- ObjUpvalue ---
class ObjUpvalue final : public Object {
  Value* location_{nullptr};
  Value closed_{};
  ObjUpvalue* next_upvalue_{nullptr};

public:
  explicit ObjUpvalue(Value* slot) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  Value* location() const noexcept { return location_; }
  void set_location(Value* loc) noexcept { location_ = loc; }
  Value& closed() noexcept { return closed_; }
  ObjUpvalue* next_upvalue() const noexcept { return next_upvalue_; }
  void set_next_upvalue(ObjUpvalue* next) noexcept { next_upvalue_ = next; }
};

// --- ObjClosure ---
class ObjClosure final : public Object {
  ObjFunction* function_{nullptr};
  std::vector<ObjUpvalue*> upvalues_;
  int upvalue_count_{0};

public:
  explicit ObjClosure(ObjFunction* function) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjFunction* function() const noexcept { return function_; }
  int upvalue_count() const noexcept { return upvalue_count_; }
  ObjUpvalue* upvalue_at(sz_t index) const noexcept { return upvalues_[index]; }
  void set_upvalue_at(sz_t index, ObjUpvalue* uv) noexcept { upvalues_[index] = uv; }
};

// --- ObjClass ---
class ObjClass final : public Object {
  ObjString* name_{nullptr};
  std::unordered_map<ObjString*, Value> methods_;

public:
  explicit ObjClass(ObjString* name) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjString* name() const noexcept { return name_; }
  std::unordered_map<ObjString*, Value>& methods() noexcept { return methods_; }
  const std::unordered_map<ObjString*, Value>& methods() const noexcept { return methods_; }
};

// --- ObjInstance ---
class ObjInstance final : public Object {
  ObjClass* klass_{nullptr};
  std::unordered_map<ObjString*, Value> fields_;

public:
  explicit ObjInstance(ObjClass* klass) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjClass* klass() const noexcept { return klass_; }
  std::unordered_map<ObjString*, Value>& fields() noexcept { return fields_; }
  const std::unordered_map<ObjString*, Value>& fields() const noexcept { return fields_; }
};

// --- ObjBoundMethod ---
class ObjBoundMethod final : public Object {
  Value receiver_;
  ObjClosure* method_{nullptr};

public:
  ObjBoundMethod(Value receiver, ObjClosure* method) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  const Value& receiver() const noexcept { return receiver_; }
  ObjClosure* method() const noexcept { return method_; }
};

// --- ObjModule ---
class ObjModule final : public Object {
  ObjString* name_{nullptr};
  std::unordered_map<ObjString*, Value> exports_;

public:
  explicit ObjModule(ObjString* name) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjString* name() const noexcept { return name_; }
  std::unordered_map<ObjString*, Value>& exports() noexcept { return exports_; }
  const std::unordered_map<ObjString*, Value>& exports() const noexcept { return exports_; }
};

// --- Convenience helpers for Value ---
inline ObjString* as_string(const Value& v) noexcept {
  return as_obj<ObjString>(v.as_object());
}

inline ObjFunction* as_function(const Value& v) noexcept {
  return as_obj<ObjFunction>(v.as_object());
}

inline ObjClosure* as_closure(const Value& v) noexcept {
  return as_obj<ObjClosure>(v.as_object());
}

inline ObjClass* as_class(const Value& v) noexcept {
  return as_obj<ObjClass>(v.as_object());
}

inline ObjInstance* as_instance(const Value& v) noexcept {
  return as_obj<ObjInstance>(v.as_object());
}

inline ObjNative* as_native(const Value& v) noexcept {
  return as_obj<ObjNative>(v.as_object());
}

inline ObjBoundMethod* as_bound_method(const Value& v) noexcept {
  return as_obj<ObjBoundMethod>(v.as_object());
}

inline ObjModule* as_module(const Value& v) noexcept {
  return as_obj<ObjModule>(v.as_object());
}

inline strv_t as_cppstring(const Value& v) noexcept {
  return as_string(v)->value();
}

} // namespace ms
