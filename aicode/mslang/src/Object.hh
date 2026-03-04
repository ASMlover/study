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
public:
  ObjectType type_;
  bool is_marked_{false};
  Object* next_{nullptr};

  explicit Object(ObjectType type) noexcept : type_(type) {}
  virtual ~Object() = default;

  virtual str_t stringify() const noexcept = 0;
  virtual void trace_references() noexcept {}
  virtual sz_t size() const noexcept = 0;

  ObjectType type() const noexcept { return type_; }
  bool is_marked() const noexcept { return is_marked_; }
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
public:
  str_t value_;
  u32_t hash_{0};

  ObjString(str_t value, u32_t hash) noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;

  static u32_t hash_string(cstr_t chars, sz_t length) noexcept;
};

// --- ObjFunction ---
class ObjFunction final : public Object {
public:
  int arity_{0};
  int upvalue_count_{0};
  Chunk chunk_;
  ObjString* name_{nullptr};

  ObjFunction() noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjNative ---
using NativeFn = std::function<Value(int arg_count, Value* args)>;

class ObjNative final : public Object {
public:
  NativeFn function_;

  explicit ObjNative(NativeFn function) noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjUpvalue ---
class ObjUpvalue final : public Object {
public:
  Value* location_{nullptr};
  Value closed_{};
  ObjUpvalue* next_upvalue_{nullptr};  // next open upvalue in VM's linked list

  explicit ObjUpvalue(Value* slot) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjClosure ---
class ObjClosure final : public Object {
public:
  ObjFunction* function_{nullptr};
  std::vector<ObjUpvalue*> upvalues_;
  int upvalue_count_{0};

  explicit ObjClosure(ObjFunction* function) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjClass ---
class ObjClass final : public Object {
public:
  ObjString* name_{nullptr};
  std::unordered_map<ObjString*, Value> methods_;

  explicit ObjClass(ObjString* name) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjInstance ---
class ObjInstance final : public Object {
public:
  ObjClass* klass_{nullptr};
  std::unordered_map<ObjString*, Value> fields_;

  explicit ObjInstance(ObjClass* klass) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjBoundMethod ---
class ObjBoundMethod final : public Object {
public:
  Value receiver_;
  ObjClosure* method_{nullptr};

  ObjBoundMethod(Value receiver, ObjClosure* method) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
};

// --- ObjModule ---
class ObjModule final : public Object {
public:
  ObjString* name_{nullptr};
  std::unordered_map<ObjString*, Value> exports_;

  explicit ObjModule(ObjString* name) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;
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
  return as_string(v)->value_;
}

} // namespace ms
