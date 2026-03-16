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
#include <cstring>
#include <format>
#include "Object.hh"

namespace ms {

// Forward declarations for GC marking (defined in Memory.cc)
void mark_object(Object* object) noexcept;
void mark_value(Value& value) noexcept;

// --- ObjString ---

ObjString::ObjString(str_t value, u32_t hash) noexcept
    : Object(ObjectType::OBJ_STRING), value_(std::move(value)), hash_(hash) {
}

str_t ObjString::stringify() const noexcept {
  return value_;
}

sz_t ObjString::size() const noexcept {
  return sizeof(ObjString) + value_.capacity();
}

u32_t ObjString::hash_string(cstr_t chars, sz_t length) noexcept {
  u32_t hash = 2166136261u;
  for (sz_t i = 0; i < length; i++) {
    hash ^= static_cast<u8_t>(chars[i]);
    hash *= 16777619u;
  }
  return hash;
}

// --- ObjFunction ---

ObjFunction::ObjFunction() noexcept
    : Object(ObjectType::OBJ_FUNCTION) {
}

str_t ObjFunction::stringify() const noexcept {
  if (name_ == nullptr) return "<script>";
  return std::format("<fn {}>", name_->value());
}

void ObjFunction::trace_references() noexcept {
  mark_object(name_);
  for (auto& constant : chunk_.constants()) {
    // We need a non-const reference for marking
    mark_value(const_cast<Value&>(constant));
  }
  // Mark inline cache entries (class pointers + cached closures)
  for (auto& ic : ic_) {
    mark_object(ic.klass);
    mark_value(ic.cached);
  }
}

sz_t ObjFunction::size() const noexcept {
  return sizeof(ObjFunction);
}

// --- ObjNative ---

ObjNative::ObjNative(NativeFn function) noexcept
    : Object(ObjectType::OBJ_NATIVE), function_(std::move(function)) {
}

str_t ObjNative::stringify() const noexcept {
  return "<native fn>";
}

sz_t ObjNative::size() const noexcept {
  return sizeof(ObjNative);
}

// --- ObjUpvalue ---

ObjUpvalue::ObjUpvalue(Value* slot) noexcept
    : Object(ObjectType::OBJ_UPVALUE), location_(slot) {
}

str_t ObjUpvalue::stringify() const noexcept {
  return "upvalue";
}

void ObjUpvalue::trace_references() noexcept {
  mark_value(closed_);
}

sz_t ObjUpvalue::size() const noexcept {
  return sizeof(ObjUpvalue);
}

// --- ObjClosure ---

ObjClosure::ObjClosure(ObjFunction* function) noexcept
    : Object(ObjectType::OBJ_CLOSURE)
    , function_(function)
    , upvalues_(static_cast<sz_t>(function->upvalue_count()), nullptr)
    , upvalue_count_(function->upvalue_count()) {
}

str_t ObjClosure::stringify() const noexcept {
  return function_->stringify();
}

void ObjClosure::trace_references() noexcept {
  mark_object(function_);
  for (auto* upvalue : upvalues_) {
    mark_object(upvalue);
  }
}

sz_t ObjClosure::size() const noexcept {
  return sizeof(ObjClosure) + upvalues_.capacity() * sizeof(ObjUpvalue*);
}

// --- ObjClass ---

ObjClass::ObjClass(ObjString* name) noexcept
    : Object(ObjectType::OBJ_CLASS), name_(name) {
}

str_t ObjClass::stringify() const noexcept {
  return name_->value();
}

void ObjClass::trace_references() noexcept {
  mark_object(name_);
  methods_.mark_table();
  static_methods_.mark_table();
  getters_.mark_table();
  setters_.mark_table();
  abstract_methods_.mark_table();
}

sz_t ObjClass::size() const noexcept {
  return sizeof(ObjClass);
}

// --- ObjInstance ---

ObjInstance::ObjInstance(ObjClass* klass) noexcept
    : Object(ObjectType::OBJ_INSTANCE), klass_(klass) {
}

str_t ObjInstance::stringify() const noexcept {
  return std::format("{} instance", klass_->name()->value());
}

void ObjInstance::trace_references() noexcept {
  mark_object(klass_);
  fields_.mark_table();
}

sz_t ObjInstance::size() const noexcept {
  return sizeof(ObjInstance);
}

// --- ObjBoundMethod ---

ObjBoundMethod::ObjBoundMethod(Value receiver, ObjClosure* method) noexcept
    : Object(ObjectType::OBJ_BOUND_METHOD)
    , receiver_(receiver)
    , method_(method) {
}

str_t ObjBoundMethod::stringify() const noexcept {
  return method_->function()->stringify();
}

void ObjBoundMethod::trace_references() noexcept {
  mark_value(receiver_);
  mark_object(method_);
}

sz_t ObjBoundMethod::size() const noexcept {
  return sizeof(ObjBoundMethod);
}

// --- ObjList ---

ObjList::ObjList() noexcept
    : Object(ObjectType::OBJ_LIST) {
}

str_t ObjList::stringify() const noexcept {
  str_t result = "[";
  for (sz_t i = 0; i < elements_.size(); i++) {
    if (i > 0) result += ", ";
    if (is_obj_type(elements_[i], ObjectType::OBJ_STRING)) {
      result += "\"" + elements_[i].stringify() + "\"";
    } else {
      result += elements_[i].stringify();
    }
  }
  result += "]";
  return result;
}

void ObjList::trace_references() noexcept {
  for (auto& element : elements_) {
    mark_value(element);
  }
}

sz_t ObjList::size() const noexcept {
  return sizeof(ObjList) + elements_.capacity() * sizeof(Value);
}

// --- ValueHash ---

sz_t ValueHash::operator()(const Value& v) const noexcept {
  if (v.is_nil()) return 0;
  if (v.is_boolean()) return std::hash<bool>{}(v.as_boolean());
  if (v.is_number()) {
    double d = v.as_number();
    sz_t h;
    std::memcpy(&h, &d, sizeof(h));
    return h;
  }
  if (v.is_object()) {
    if (v.as_object()->type() == ObjectType::OBJ_STRING) {
      return as_obj<ObjString>(v.as_object())->hash();
    }
    if (v.as_object()->type() == ObjectType::OBJ_TUPLE) {
      return as_obj<ObjTuple>(v.as_object())->hash();
    }
    return std::hash<void*>{}(v.as_object());
  }
  return 0;
}

// --- ObjMap ---

ObjMap::ObjMap() noexcept
    : Object(ObjectType::OBJ_MAP) {
}

str_t ObjMap::stringify() const noexcept {
  str_t result = "{";
  bool first = true;
  for (auto& [key, val] : entries_) {
    if (!first) result += ", ";
    first = false;
    if (is_obj_type(key, ObjectType::OBJ_STRING)) {
      result += "\"" + key.stringify() + "\"";
    } else {
      result += key.stringify();
    }
    result += ": ";
    if (is_obj_type(val, ObjectType::OBJ_STRING)) {
      result += "\"" + val.stringify() + "\"";
    } else {
      result += val.stringify();
    }
  }
  result += "}";
  return result;
}

void ObjMap::trace_references() noexcept {
  for (auto& [key, val] : entries_) {
    mark_value(const_cast<Value&>(key));
    mark_value(val);
  }
}

sz_t ObjMap::size() const noexcept {
  return sizeof(ObjMap) + entries_.size() * (sizeof(Value) * 2);
}

// --- ObjModule ---

ObjModule::ObjModule(ObjString* name) noexcept
    : Object(ObjectType::OBJ_MODULE), name_(name) {
}

str_t ObjModule::stringify() const noexcept {
  return std::format("<module {}>", name_->value());
}

void ObjModule::trace_references() noexcept {
  mark_object(name_);
  exports_.mark_table();
}

sz_t ObjModule::size() const noexcept {
  return sizeof(ObjModule);
}

// --- ObjStringBuilder ---

ObjStringBuilder::ObjStringBuilder() noexcept
    : Object(ObjectType::OBJ_STRING_BUILDER) {
}

str_t ObjStringBuilder::stringify() const noexcept {
  return std::format("<StringBuilder len={}>", buffer_.size());
}

sz_t ObjStringBuilder::size() const noexcept {
  return sizeof(ObjStringBuilder) + buffer_.capacity();
}

// --- ObjTuple ---

ObjTuple::ObjTuple(std::vector<Value> elements) noexcept
    : Object(ObjectType::OBJ_TUPLE), elements_(std::move(elements)) {
  hash_ = compute_hash();
}

str_t ObjTuple::stringify() const noexcept {
  str_t result = "(";
  for (sz_t i = 0; i < elements_.size(); i++) {
    if (i > 0) result += ", ";
    if (is_obj_type(elements_[i], ObjectType::OBJ_STRING)) {
      result += "\"" + elements_[i].stringify() + "\"";
    } else {
      result += elements_[i].stringify();
    }
  }
  if (elements_.size() == 1) result += ",";
  result += ")";
  return result;
}

void ObjTuple::trace_references() noexcept {
  for (auto& element : elements_) {
    mark_value(element);
  }
}

sz_t ObjTuple::size() const noexcept {
  return sizeof(ObjTuple) + elements_.capacity() * sizeof(Value);
}

u32_t ObjTuple::compute_hash() const noexcept {
  u32_t h = 2166136261u;
  ValueHash hasher;
  for (const auto& elem : elements_) {
    sz_t elem_hash = hasher(elem);
    h ^= static_cast<u32_t>(elem_hash);
    h *= 16777619u;
  }
  return h;
}

} // namespace ms
