// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <algorithm>
#include "value.hh"

namespace wrencc {

static const Value kUndefined;

StringObject* BaseObject::as_string() noexcept {
  return Xt::down<StringObject>(this);
}

const char* BaseObject::as_cstring() noexcept {
  return Xt::down<StringObject>(this)->cstr();
}

ListObject* BaseObject::as_list() noexcept {
  return Xt::down<ListObject>(this);
}

RangeObject* BaseObject::as_range() noexcept {
  return Xt::down<RangeObject>(this);
}

MapObject* BaseObject::as_map() noexcept {
  return Xt::down<MapObject>(this);
}

ModuleObject* BaseObject::as_module() noexcept {
  return Xt::down<ModuleObject>(this);
}

FunctionObject* BaseObject::as_function() noexcept {
  return Xt::down<FunctionObject>(this);
}

ForeignObject* BaseObject::as_foreign() noexcept {
  return Xt::down<ForeignObject>(this);
}

UpvalueObject* BaseObject::as_upvalue() noexcept {
  return Xt::down<UpvalueObject>(this);
}

ClosureObject* BaseObject::as_closure() noexcept {
  return Xt::down<ClosureObject>(this);
}

FiberObject* BaseObject::as_fiber() noexcept {
  return Xt::down<FiberObject>(this);
}

ClassObject* BaseObject::as_class() noexcept {
  // TODO:
  return nullptr;
}

InstanceObject* BaseObject::as_instance() noexcept {
  // TODO:
  return nullptr;
}

bool ObjValue::is_same(ObjValue r) const noexcept {
  if (type_ != r.type_)
    return false;

  if (type_ == ValueType::NUMERIC)
    return as_.num == r.as_.num;
  return as_.obj == r.as_.obj;
}

bool ObjValue::is_equal(ObjValue r) const noexcept {
  if (is_same(r))
    return true;

  if (!is_object() || !r.is_object() || objtype() != r.objtype())
    return false;
  return as_.obj->is_equal(r.as_.obj);
}

u32_t ObjValue::hasher() const noexcept {
  switch (type_) {
  case ValueType::NIL: return 1;
  case ValueType::TRUE: return 2;
  case ValueType::FALSE: return 0;
  case ValueType::NUMERIC: return Xt::hash_numeric(as_.num);
  case ValueType::OBJECT: return as_.obj->hasher();
  default: UNREACHABLE();
  }
  return 0;
}

str_t ObjValue::stringify() const noexcept {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(as_.num);
  case ValueType::OBJECT: return as_.obj->stringify();
  default: UNREACHABLE();
  }
  return "";
}

StringObject::StringObject(ClassObject* cls, char c) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(1) {
  data_ = new char[size_ + 1];
  data_[0] = c, data_[1] = 0;

  hash_string();
}

StringObject::StringObject(
    ClassObject* cls, const char* s, sz_t n, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(n) {
  if (replace_owner) {
    data_ = const_cast<char*>(s);
  }
  else {
    if (s != nullptr) {
      data_ = new char[size_ + 1];
      std::memcpy(data_, s, n);
      data_[size_] = 0;
    }
  }

  hash_string();
}

StringObject::~StringObject() noexcept {
  if (data_)
    delete [] data_;
}

void StringObject::hash_string() noexcept {
  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/

  u32_t hash = 2166136261u;
  for (u32_t i = 0; i < size_; ++i) {
    hash ^= data_[i];
    hash *= 16777619;
  }
  hash_ = hash;
}

int StringObject::find(StringObject* sub, sz_t off) const {
  if (sub->size_ == 0)
    return Xt::as_type<int>(off);
  if (sub->size_ + off > size_ || off >= size_)
    return -1;

  char* found = std::strstr(data_ + off, sub->data_);
  return found != nullptr ? Xt::as_type<int>(found - data_) : -1;
}

bool StringObject::is_equal(BaseObject* r) const {
  return compare(r->as_string());
}

str_t StringObject::stringify() const {
  return data_ != nullptr ? data_ : "";
}

u32_t StringObject::hasher() const {
  return hash_;
}

StringObject* StringObject::create(WrenVM& vm, char c) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::create(WrenVM& vm, const char* s, sz_t n) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::create(WrenVM& vm, const str_t& s) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(
    WrenVM& vm, StringObject* s1, StringObject* s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(WrenVM& vm, const char* s1, const char* s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(
    WrenVM& vm, const str_t& s1, const str_t& s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_byte(WrenVM& vm, u8_t value) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_numeric(WrenVM& vm, double value) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_range(
    WrenVM& vm, StringObject* s, sz_t off, sz_t n, sz_t step) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::format(WrenVM& vm, const char* format, ...) {
  // TODO:
  return nullptr;
}

ListObject::ListObject(ClassObject* cls, sz_t n, Value v) noexcept
  : BaseObject(ObjType::LIST, cls) {
  if (n > 0)
    elements_.resize(n, v);
}

str_t ListObject::stringify() const {
  ss_t ss;

  ss << "[";
  for (auto i = 0u; i < elements_.size(); ++i) {
    if (i > 0)
      ss << ", ";

    auto& v = elements_[i];
    if (v.is_string())
      ss << "\"" << v << "\"";
    else
      ss << v;
  }
  ss << "]";

  return ss.str();
}

void ListObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

ListObject* ListObject::create(WrenVM& vm, sz_t n, Value v) {
  // TODO:
  return nullptr;
}

bool RangeObject::is_equal(BaseObject* r) const {
  RangeObject* o = r->as_range();
  return from_ == o->from_ && to_ == o->to_ && is_inclusive_ == o->is_inclusive_;
}

str_t RangeObject::stringify() const {
  return get_stringify("range");
}

u32_t RangeObject::hasher() const {
  return Xt::hash_numeric(from_) ^ Xt::hash_numeric(to_);
}

RangeObject* RangeObject::create(
    WrenVM& vm, double from, double to, bool is_inclusive) {
  // TODO:
  return nullptr;
}

std::tuple<bool, int> MapObject::find_entry(Value key) const {
  if (capacity_ == 0)
    return std::make_tuple(false, -1);

  sz_t start_index = key.hasher() % capacity_;
  sz_t index = start_index;
  do {
    const MapEntry& entry = entries_[index];
    if (entry.first.is_undefined()) {
      if (!entry.second.as_boolean())
        return std::make_tuple(false, Xt::as_type<int>(index));
    }
    else if (entry.first == key) {
      return std::make_tuple(true, Xt::as_type<int>(index));
    }

    index = (index + 1) % capacity_;
  } while (index != start_index);

  ASSERT(false, "map should have tombstones or empty entries");
  return std::make_tuple(false, -1);
}

bool MapObject::insert_entry(Value k, Value v) {
  auto [r, index] = find_entry(k);
  if (r)
    entries_[index].second = v;
  else
    entries_[index].swap(std::make_pair(k, v));
  return !r;
}

void MapObject::resize(sz_t new_capacity) {
  std::vector<MapEntry> new_entries(
      new_capacity, std::make_pair(kUndefined, false));

  if (capacity_ > 0) {
    for (auto& entry : entries_) {
      if (!entry.first.is_undefined())
        insert_entry(entry.first, entry.second);
    }
  }

  entries_.clear();
  capacity_ = new_capacity;
  entries_.swap(new_entries);
}

void MapObject::clear() {
  entries_.clear();
  capacity_ = 0;
  size_ = 0;
}

bool MapObject::contains(Value key) const {
  auto [r, _] = find_entry(key);
  return r;
}

std::optional<Value> MapObject::get(Value key) const {
  auto [r, index] = find_entry(key);
  if (r)
    return {entries_[index].second};
  return {};
}

void MapObject::set(Value key, Value val) {
  if (size_ + 1 > capacity_ * kLoadPercent / 100) {
    sz_t new_capacity = std::max(capacity_ * kGrowFactor, kMinCapacity);
    resize(new_capacity);
  }

  if (insert_entry(key, val))
    ++size_;
}

Value MapObject::remove(Value key) {
  auto [r, index] = find_entry(key);
  if (!r)
    return nullptr;

  Value value = entries_[index].second;
  entries_[index].first = kUndefined;
  entries_[index].second = true;

  --size_;
  if (size_ == 0) {
    clear();
  }
  else if (capacity_ > kMinCapacity &&
      size_ < capacity_ / kGrowFactor * kLoadPercent / 100) {
    sz_t new_capacity = std::max(capacity_ / kGrowFactor, kMinCapacity);
    resize(new_capacity);
  }

  return value;
}

str_t MapObject::stringify() const {
  return get_stringify("map");
}

void MapObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

MapObject* MapObject::create(WrenVM& vm) {
  // TODO:
  return nullptr;
}

std::optional<sz_t> ModuleObject::find_variable(const str_t& name) const {
  for (sz_t i = 0; i < variable_names_.size(); ++i) {
    if (variable_names_[i]->compare(name))
      return {i};
  }
  return {};
}

sz_t ModuleObject::declare_variable(WrenVM& vm, const str_t& name, int line) {
  // TODO:
  return variable_names_.size() - 1;
}

std::tuple<int, int> ModuleObject::define_variable(
    WrenVM& vm, const str_t& name, Value value) {
  // TODO:
  return std::make_tuple(0, 0);
}

void ModuleObject::iter_variables(
    std::function<void (sz_t, StringObject*, Value)>&& fn, int off) {
  for (sz_t i = off; i < variables_.size(); ++i)
    fn(i, variable_names_[i], variables_[i]);
}

str_t ModuleObject::stringify() const {
  return get_stringify("module");
}

void ModuleObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

ModuleObject* ModuleObject::create(WrenVM& vm, StringObject* name) {
  // TODO:
  return nullptr;
}

FunctionObject::FunctionObject(
    ClassObject* cls, ModuleObject* module, int max_slots) noexcept
  : BaseObject(ObjType::FUNCTION, cls)
  , module_(module)
  , max_slots_(max_slots) {
}

FunctionObject::FunctionObject(ClassObject* cls,
    u8_t* codes, int codes_count,
    Value* constants, int constants_count,
    ModuleObject* module, int max_slots, int num_upvalues, int arity,
    const str_t& debug_name, int* source_lines, int lines_count) noexcept
  : BaseObject(ObjType::FUNCTION, cls)
  , codes_(codes, codes + codes_count)
  , constants_(constants, constants + constants_count)
  , module_(module)
  , max_slots_(max_slots)
  , num_upvalues_(num_upvalues)
  , arity_(arity)
  , debug_(debug_name, source_lines, lines_count) {
}

int FunctionObject::indexof_constant(Value v) const {
  for (sz_t i = 0; i < constants_.size(); ++i) {
    if (constants_[i] == v)
      return Xt::as_type<int>(i);
  }
  return -1;
}

str_t FunctionObject::stringify() const {
  return get_stringify("fn");
}

void FunctionObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

u32_t FunctionObject::hasher() const {
  return Xt::hash_numeric(arity_) ^ Xt::hash_u64(codes_count());
}

int FunctionObject::get_argc(
    const u8_t* bytecode, const Value* constants, int ip) {
  // TODO:
  return 0;
}

FunctionObject* FunctionObject::create(
    WrenVM& vm, ModuleObject* module, int max_slots) {
  // TODO:
  return nullptr;
}

FunctionObject* FunctionObject::create(WrenVM& vm,
    u8_t* codes, int codes_count,
    Value* constants, int constants_count,
    ModuleObject* module, int max_slots, int num_upvalues, int arity,
    const str_t& name, int* source_lines, int lines_count) {
  // TODO:
  return nullptr;
}

str_t ForeignObject::stringify() const {
  return get_stringify("foreign");
}

void ForeignObject::finalize(WrenVM& vm) {
  // TODO:
}

ForeignObject* ForeignObject::create(WrenVM& vm, ClassObject* cls, sz_t count) {
  // TODO:
  return nullptr;
}

UpvalueObject::UpvalueObject(Value* value, UpvalueObject* next) noexcept
  : BaseObject(ObjType::UPVALUE, nullptr)
  , value_(value)
  , next_(next) {
}

str_t UpvalueObject::stringify() const {
  return get_stringify("upvalue");
}

void UpvalueObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

UpvalueObject* UpvalueObject::create(
    WrenVM& vm, Value* value, UpvalueObject* next) {
  // TODO:
  return nullptr;
}

ClosureObject::ClosureObject(ClassObject* cls, FunctionObject* fn) noexcept
  : BaseObject(ObjType::CLOSURE, cls)
  , fn_(fn) {
  if (int num_upvalues = fn_->num_upvalues(); num_upvalues > 0) {
    upvalues_ = new UpvalueObject*[num_upvalues];
    for (int i = 0; i < num_upvalues; ++i)
      upvalues_[i] = nullptr;
  }
}

ClosureObject::~ClosureObject() noexcept {
  if (upvalues_ != nullptr)
    delete [] upvalues_;
}

str_t ClosureObject::stringify() const {
  return get_stringify("closure");
}

void ClosureObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

ClosureObject* ClosureObject::create(WrenVM& vm, FunctionObject* fn) {
  // TODO:
  return nullptr;
}

}
