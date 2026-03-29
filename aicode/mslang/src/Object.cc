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
#include <cassert>
#include <cstring>
#include <format>
#include <sstream>
#include "Object.hh"

namespace ms {

// Forward declarations for GC marking (defined in Memory.cc)
void mark_object(Object* object) noexcept;
void mark_value(Value& value) noexcept;

// --- Shape ---

Shape::~Shape() noexcept {
  transitions_.for_each_value([](Shape* child) { delete child; });
}

i32_t Shape::find_slot(ObjString* name) const noexcept {
  const u32_t* p = slots_.find(name);
  return p ? static_cast<i32_t>(*p) : -1;
}

Shape* Shape::add_transition(ObjString* name, u32_t& next_id) noexcept {
  Shape** p = transitions_.find(name);
  if (p) return *p;

  auto* child = new Shape(next_id++);
  child->slots_ = slots_;
  child->slots_.insert(name, slot_count_);
  child->slot_count_ = slot_count_ + 1;
  transitions_.insert(name, child);
  return child;
}

void Shape::mark_keys() noexcept {
  slots_.for_each_key([](ObjString* k) { mark_object(k); });
  transitions_.for_each([](ObjString* k, Shape* child) {
    mark_object(k);
    child->mark_keys();
  });
}

// --- ObjString ---

ObjString* ObjString::create(cstr_t chars, u32_t length, u32_t hash) noexcept {
  void* mem = ::operator new(sizeof(ObjString) + length + 1);
  ObjString* s = ::new(mem) ObjString();
  s->hash_   = hash;
  s->length_ = length;
  std::memcpy(s->data_, chars, length);
  s->data_[length] = '\0';
  return s;
}

str_t ObjString::stringify() const noexcept {
  return str_t(data_, length_);
}

sz_t ObjString::size() const noexcept {
  return sizeof(ObjString) + length_ + 1;
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
  // Mark inline cache entries (PIC: name, class pointers + cached closures)
  for (auto& ic : ic_) {
    if (ic.name) mark_object(ic.name);
    for (u8_t i = 0; i < ic.count; ++i) {
      if (ic.entries[i].klass)
        mark_object(ic.entries[i].klass);
      mark_value(ic.entries[i].cached);
    }
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

ObjClosure* ObjClosure::create(ObjFunction* function) noexcept {
  int n = function->upvalue_count();
  sz_t sz = sizeof(ObjClosure) + static_cast<sz_t>(n) * sizeof(ObjUpvalue*);
  void* mem = ::operator new(sz);
  ObjClosure* c = ::new(mem) ObjClosure();
  c->function_ = function;
  c->upvalue_count_ = n;
  auto** upvalues = reinterpret_cast<ObjUpvalue**>(c + 1);
  for (int i = 0; i < n; ++i) upvalues[i] = nullptr;
  return c;
}

str_t ObjClosure::stringify() const noexcept {
  return function_->stringify();
}

void ObjClosure::trace_references() noexcept {
  mark_object(function_);
  for (int i = 0; i < upvalue_count_; ++i)
    mark_object(upvalue_at(static_cast<sz_t>(i)));
}

sz_t ObjClosure::size() const noexcept {
  return sizeof(ObjClosure) + static_cast<sz_t>(upvalue_count_) * sizeof(ObjUpvalue*);
}

// --- ObjClass ---

ObjClass::ObjClass(ObjString* name) noexcept
    : Object(ObjectType::OBJ_CLASS), name_(name), root_shape_(new Shape(0)) {
}

ObjClass::~ObjClass() noexcept {
  delete root_shape_;
}

str_t ObjClass::stringify() const noexcept {
  return str_t(name_->value());
}

void ObjClass::trace_references() noexcept {
  mark_object(name_);
  methods_.mark_table();
  static_methods_.mark_table();
  if (getters_) getters_->mark_table();
  if (setters_) setters_->mark_table();
  if (abstract_methods_) abstract_methods_->mark_table();
  root_shape_->mark_keys();
}

sz_t ObjClass::size() const noexcept {
  return sizeof(ObjClass)
      + (getters_ ? getters_->capacity() * sizeof(Entry) : 0)
      + (setters_ ? setters_->capacity() * sizeof(Entry) : 0)
      + (abstract_methods_ ? abstract_methods_->capacity() * sizeof(Entry) : 0);
}

// --- ObjInstance ---

ObjInstance::ObjInstance(ObjClass* klass) noexcept
    : Object(ObjectType::OBJ_INSTANCE), klass_(klass), shape_(klass->root_shape()) {
}

ObjInstance::~ObjInstance() noexcept {
  delete[] overflow_;
}

str_t ObjInstance::stringify() const noexcept {
  return std::format("{} instance", klass_->name()->value());
}

void ObjInstance::trace_references() noexcept {
  mark_object(klass_);
  Value* flds = fields_ptr();
  for (u32_t i = 0; i < field_count_; ++i) mark_value(flds[i]);
}

sz_t ObjInstance::size() const noexcept {
  return sizeof(ObjInstance) + (overflow_ ? capacity_ * sizeof(Value) : 0);
}

bool ObjInstance::get_field(ObjString* name, Value* value) const noexcept {
  i32_t slot = shape_->find_slot(name);
  if (slot < 0) return false;
  *value = fields_ptr()[static_cast<u32_t>(slot)];
  return true;
}

void ObjInstance::set_field(ObjString* name, Value value) noexcept {
  i32_t slot = shape_->find_slot(name);
  if (slot >= 0) {
    fields_ptr()[static_cast<u32_t>(slot)] = value;
  } else {
    shape_ = shape_->add_transition(name, klass_->next_shape_id_ref());
    if (field_count_ < capacity_) {
      fields_ptr()[field_count_++] = value;
    } else {
      // Spill: double capacity and move to heap
      u32_t new_cap = capacity_ * 2;
      Value* new_buf = new Value[new_cap];
      Value* old_buf = fields_ptr();
      for (u32_t i = 0; i < field_count_; ++i) new_buf[i] = old_buf[i];
      delete[] overflow_;
      overflow_ = new_buf;
      capacity_ = new_cap;
      overflow_[field_count_++] = value;
    }
  }
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

// --- ValueTable ---

ValueEntry* ValueTable::find_entry(const Value& key) noexcept {
  sz_t cap = entries_.size();
  sz_t idx = ValueHash{}(key) % cap;
  ValueEntry* first_tombstone = nullptr;
  for (;;) {
    ValueEntry* e = &entries_[idx];
    if (e->tombstone) {
      if (!first_tombstone) first_tombstone = e;
    } else if (e->key.is_nil()) {
      return first_tombstone ? first_tombstone : e;
    } else if (e->key.is_equal(key)) {
      return e;
    }
    idx = (idx + 1) % cap;
  }
}

void ValueTable::grow() noexcept {
  sz_t new_cap = entries_.size() < 8 ? 8 : entries_.size() * 2;
  std::vector<ValueEntry> new_entries(new_cap);
  int new_count = 0;
  for (auto& e : entries_) {
    if (e.tombstone || e.key.is_nil()) continue;
    sz_t idx = ValueHash{}(e.key) % new_cap;
    while (!new_entries[idx].key.is_nil()) idx = (idx + 1) % new_cap;
    new_entries[idx] = e;
    new_count++;
  }
  entries_ = std::move(new_entries);
  count_ = new_count;
  used_ = new_count;
}

bool ValueTable::get(const Value& key, Value* out) const noexcept {
  if (entries_.empty()) return false;
  sz_t cap = entries_.size();
  sz_t idx = ValueHash{}(key) % cap;
  for (;;) {
    const ValueEntry& e = entries_[idx];
    if (!e.tombstone && e.key.is_nil()) return false;
    if (!e.tombstone && e.key.is_equal(key)) {
      if (out) *out = e.value;
      return true;
    }
    idx = (idx + 1) % cap;
  }
}

bool ValueTable::set(const Value& key, const Value& val) noexcept {
  sz_t cap = entries_.size();
  if (cap == 0 || static_cast<double>(used_ + 1) > cap * kMAX_LOAD) {
    grow();
  }
  ValueEntry* entry = find_entry(key);
  bool was_new = (!entry->tombstone && entry->key.is_nil());
  bool was_tombstone = entry->tombstone;
  entry->key = key;
  entry->value = val;
  entry->tombstone = false;
  if (was_new) { count_++; used_++; }
  else if (was_tombstone) { count_++; }
  return was_new || was_tombstone;
}

bool ValueTable::del(const Value& key) noexcept {
  if (entries_.empty()) return false;
  ValueEntry* entry = find_entry(key);
  if (entry->key.is_nil() || entry->tombstone) return false;
  entry->tombstone = true;
  entry->key = Value();
  entry->value = Value();
  count_--;
  return true;
}

void ValueTable::mark_entries() noexcept {
  for (auto& e : entries_) {
    if (!e.tombstone && !e.key.is_nil()) {
      mark_value(e.key);
      mark_value(e.value);
    }
  }
}

// --- ObjMap ---

ObjMap::ObjMap() noexcept
    : Object(ObjectType::OBJ_MAP) {
}

str_t ObjMap::stringify() const noexcept {
  str_t result = "{";
  bool first = true;
  entries_.for_each([&](const Value& key, const Value& val) {
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
  });
  result += "}";
  return result;
}

void ObjMap::trace_references() noexcept {
  entries_.mark_entries();
}

sz_t ObjMap::size() const noexcept {
  return sizeof(ObjMap) + sizeof(ValueEntry) * 8; // approximate
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

// --- ObjFile ---

ObjFile::ObjFile(str_t path, str_t mode) noexcept
    : Object(ObjectType::OBJ_FILE), path_(std::move(path)), mode_(std::move(mode)),
      stream_(std::make_unique<std::fstream>()) {
}

ObjFile::~ObjFile() noexcept {
  close();
}

str_t ObjFile::stringify() const noexcept {
  return std::format("<file '{}' {}>", path_, is_open_ ? "open" : "closed");
}

sz_t ObjFile::size() const noexcept {
  return sizeof(ObjFile);
}

bool ObjFile::open() noexcept {
  std::ios_base::openmode flags{};
  if (mode_ == "r") {
    flags = std::ios::in;
  } else if (mode_ == "w") {
    flags = std::ios::out | std::ios::trunc;
  } else if (mode_ == "a") {
    flags = std::ios::out | std::ios::app;
  } else if (mode_ == "rb") {
    flags = std::ios::in | std::ios::binary;
  } else if (mode_ == "wb") {
    flags = std::ios::out | std::ios::trunc | std::ios::binary;
  } else if (mode_ == "ab") {
    flags = std::ios::out | std::ios::app | std::ios::binary;
  } else {
    return false;
  }
  stream_->open(path_, flags);
  is_open_ = stream_->is_open();
  return is_open_;
}

void ObjFile::close() noexcept {
  if (is_open_) {
    stream_->close();
    is_open_ = false;
  }
}

str_t ObjFile::read_all() noexcept {
  if (!is_open_) return "";
  std::ostringstream oss;
  oss << stream_->rdbuf();
  // Trigger eof detection after full read
  stream_->peek();
  return oss.str();
}

str_t ObjFile::read_line() noexcept {
  if (!is_open_) return "";
  str_t line;
  if (std::getline(*stream_, line)) {
    return line;
  }
  return "";
}

void ObjFile::write(strv_t data) noexcept {
  if (is_open_) {
    stream_->write(data.data(), static_cast<std::streamsize>(data.size()));
  }
}

bool ObjFile::eof() const noexcept {
  return !is_open_ || stream_->eof();
}

// --- ObjWeakRef ---

ObjWeakRef::ObjWeakRef(Object* target) noexcept
    : Object(ObjectType::OBJ_WEAK_REF), target_(target) {
}

str_t ObjWeakRef::stringify() const noexcept {
  if (target_ != nullptr) {
    return std::format("<weak_ref {}>", object_stringify(target_));
  }
  return "<weak_ref (dead)>";
}

sz_t ObjWeakRef::size() const noexcept {
  return sizeof(ObjWeakRef);
}

// --- Devirtualized dispatch functions ---

str_t object_stringify(const Object* obj) noexcept {
  switch (obj->type()) {
    case ObjectType::OBJ_STRING:         return static_cast<const ObjString*>(obj)->stringify();
    case ObjectType::OBJ_FUNCTION:       return static_cast<const ObjFunction*>(obj)->stringify();
    case ObjectType::OBJ_NATIVE:         return static_cast<const ObjNative*>(obj)->stringify();
    case ObjectType::OBJ_UPVALUE:        return static_cast<const ObjUpvalue*>(obj)->stringify();
    case ObjectType::OBJ_CLOSURE:        return static_cast<const ObjClosure*>(obj)->stringify();
    case ObjectType::OBJ_CLASS:          return static_cast<const ObjClass*>(obj)->stringify();
    case ObjectType::OBJ_INSTANCE:       return static_cast<const ObjInstance*>(obj)->stringify();
    case ObjectType::OBJ_BOUND_METHOD:   return static_cast<const ObjBoundMethod*>(obj)->stringify();
    case ObjectType::OBJ_LIST:           return static_cast<const ObjList*>(obj)->stringify();
    case ObjectType::OBJ_MAP:            return static_cast<const ObjMap*>(obj)->stringify();
    case ObjectType::OBJ_MODULE:         return static_cast<const ObjModule*>(obj)->stringify();
    case ObjectType::OBJ_STRING_BUILDER: return static_cast<const ObjStringBuilder*>(obj)->stringify();
    case ObjectType::OBJ_TUPLE:          return static_cast<const ObjTuple*>(obj)->stringify();
    case ObjectType::OBJ_FILE:           return static_cast<const ObjFile*>(obj)->stringify();
    case ObjectType::OBJ_WEAK_REF:       return static_cast<const ObjWeakRef*>(obj)->stringify();
    case ObjectType::OBJ_COROUTINE:      return static_cast<const ObjCoroutine*>(obj)->stringify();
  }
  return "<unknown>";
}

void object_trace(Object* obj) noexcept {
  switch (obj->type()) {
    case ObjectType::OBJ_FUNCTION:       static_cast<ObjFunction*>(obj)->trace_references();    break;
    case ObjectType::OBJ_UPVALUE:        static_cast<ObjUpvalue*>(obj)->trace_references();     break;
    case ObjectType::OBJ_CLOSURE:        static_cast<ObjClosure*>(obj)->trace_references();     break;
    case ObjectType::OBJ_CLASS:          static_cast<ObjClass*>(obj)->trace_references();       break;
    case ObjectType::OBJ_INSTANCE:       static_cast<ObjInstance*>(obj)->trace_references();    break;
    case ObjectType::OBJ_BOUND_METHOD:   static_cast<ObjBoundMethod*>(obj)->trace_references(); break;
    case ObjectType::OBJ_LIST:           static_cast<ObjList*>(obj)->trace_references();        break;
    case ObjectType::OBJ_MAP:            static_cast<ObjMap*>(obj)->trace_references();         break;
    case ObjectType::OBJ_MODULE:         static_cast<ObjModule*>(obj)->trace_references();      break;
    case ObjectType::OBJ_TUPLE:          static_cast<ObjTuple*>(obj)->trace_references();       break;
    case ObjectType::OBJ_COROUTINE:      static_cast<ObjCoroutine*>(obj)->trace_references();   break;
    default: break; // OBJ_STRING, OBJ_NATIVE, OBJ_STRING_BUILDER, OBJ_FILE, OBJ_WEAK_REF: no refs
  }
}

sz_t object_size(const Object* obj) noexcept {
  switch (obj->type()) {
    case ObjectType::OBJ_STRING:         return static_cast<const ObjString*>(obj)->size();
    case ObjectType::OBJ_FUNCTION:       return static_cast<const ObjFunction*>(obj)->size();
    case ObjectType::OBJ_NATIVE:         return static_cast<const ObjNative*>(obj)->size();
    case ObjectType::OBJ_UPVALUE:        return static_cast<const ObjUpvalue*>(obj)->size();
    case ObjectType::OBJ_CLOSURE:        return static_cast<const ObjClosure*>(obj)->size();
    case ObjectType::OBJ_CLASS:          return static_cast<const ObjClass*>(obj)->size();
    case ObjectType::OBJ_INSTANCE:       return static_cast<const ObjInstance*>(obj)->size();
    case ObjectType::OBJ_BOUND_METHOD:   return static_cast<const ObjBoundMethod*>(obj)->size();
    case ObjectType::OBJ_LIST:           return static_cast<const ObjList*>(obj)->size();
    case ObjectType::OBJ_MAP:            return static_cast<const ObjMap*>(obj)->size();
    case ObjectType::OBJ_MODULE:         return static_cast<const ObjModule*>(obj)->size();
    case ObjectType::OBJ_STRING_BUILDER: return static_cast<const ObjStringBuilder*>(obj)->size();
    case ObjectType::OBJ_TUPLE:          return static_cast<const ObjTuple*>(obj)->size();
    case ObjectType::OBJ_FILE:           return static_cast<const ObjFile*>(obj)->size();
    case ObjectType::OBJ_WEAK_REF:       return static_cast<const ObjWeakRef*>(obj)->size();
    case ObjectType::OBJ_COROUTINE:      return static_cast<const ObjCoroutine*>(obj)->size();
  }
  return 0;
}

void object_destroy(Object* obj) noexcept {
  switch (obj->type()) {
    case ObjectType::OBJ_STRING:         static_cast<ObjString*>(obj)->~ObjString(); ::operator delete(obj); break;
    case ObjectType::OBJ_FUNCTION:       delete static_cast<ObjFunction*>(obj);      break;
    case ObjectType::OBJ_NATIVE:         delete static_cast<ObjNative*>(obj);        break;
    case ObjectType::OBJ_UPVALUE:        delete static_cast<ObjUpvalue*>(obj);       break;
    case ObjectType::OBJ_CLOSURE: {
      auto* c = static_cast<ObjClosure*>(obj);
      c->~ObjClosure();
      ::operator delete(c);
      break;
    }
    case ObjectType::OBJ_CLASS:          delete static_cast<ObjClass*>(obj);         break;
    case ObjectType::OBJ_INSTANCE:       delete static_cast<ObjInstance*>(obj);      break;
    case ObjectType::OBJ_BOUND_METHOD:   delete static_cast<ObjBoundMethod*>(obj);   break;
    case ObjectType::OBJ_LIST:           delete static_cast<ObjList*>(obj);          break;
    case ObjectType::OBJ_MAP:            delete static_cast<ObjMap*>(obj);           break;
    case ObjectType::OBJ_MODULE:         delete static_cast<ObjModule*>(obj);        break;
    case ObjectType::OBJ_STRING_BUILDER: delete static_cast<ObjStringBuilder*>(obj); break;
    case ObjectType::OBJ_TUPLE:          delete static_cast<ObjTuple*>(obj);         break;
    case ObjectType::OBJ_FILE:           delete static_cast<ObjFile*>(obj);          break;
    case ObjectType::OBJ_WEAK_REF:       delete static_cast<ObjWeakRef*>(obj);       break;
    case ObjectType::OBJ_COROUTINE:      delete static_cast<ObjCoroutine*>(obj);     break;
    default: assert(false && "object_destroy: unknown type");                        break;
  }
}

// --- ObjCoroutine ---

void ObjCoroutine::trace_references() noexcept {
  mark_object(closure_);
  mark_value(yielded_value_);
  mark_value(sent_value_);
  for (auto& val : saved_stack_) mark_value(val);
  for (auto& val : init_args_) mark_value(val);
  // Mark all GC roots stored in properly-copied SavedCallFrame entries
  for (auto& f : saved_frames_) {
    mark_object(f.closure);
    mark_value(const_cast<Value&>(f.pending_return));
  }
}

} // namespace ms
