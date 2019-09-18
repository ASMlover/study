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
#include <cstdarg>
#include <sstream>
#include "vm.hh"
#include "value.hh"

namespace wrencc {

static const Value kUndefined;

union DoubleBits {
  u64_t b64;
  u32_t b32[2];
  double num;
};

inline u32_t hash_numeric(double num) {
  // generates a hash code for [num]

  DoubleBits data;
  data.num = num;
  return data.b32[0] ^ data.b32[1];
}

std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

StringObject* TagValue::as_string(void) const {
  return Xt::down<StringObject>(as_object());
}

const char* TagValue::as_cstring(void) const {
  return Xt::down<StringObject>(as_object())->cstr();
}

ListObject* TagValue::as_list(void) const {
  return Xt::down<ListObject>(as_object());
}

RangeObject* TagValue::as_range(void) const {
  return Xt::down<RangeObject>(as_object());
}

MapObject* TagValue::as_map(void) const {
  return Xt::down<MapObject>(as_object());
}

ModuleObject* TagValue::as_module(void) const {
  return Xt::down<ModuleObject>(as_object());
}

FunctionObject* TagValue::as_function(void) const {
  return Xt::down<FunctionObject>(as_object());
}

UpvalueObject* TagValue::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(as_object());
}

ClosureObject* TagValue::as_closure(void) const {
  return Xt::down<ClosureObject>(as_object());
}

FiberObject* TagValue::as_fiber(void) const {
  return Xt::down<FiberObject>(as_object());
}

ClassObject* TagValue::as_class(void) const {
  return Xt::down<ClassObject>(as_object());
}

InstanceObject* TagValue::as_instance(void) const {
  return Xt::down<InstanceObject>(as_object());
}

bool TagValue::is_equal(const TagValue& r) const noexcept {
  if (is_same(r))
    return true;

  if (!is_object() || !r.is_object())
    return false;
  if (objtype() != r.objtype())
    return false;

  return as_object()->is_equal(r.as_object());
}

u32_t TagValue::hash(void) const {
  if (is_object())
    return as_object()->hash();

  // hash the raw bits of the unboxed value
  DoubleBits bits;
  bits.b64 = bits_;
  return bits.b32[0] ^ bits.b32[1];
}

str_t TagValue::stringify(void) const {
  if (is_numeric()) {
    return Xt::to_string(as_numeric());
  }
  else if (is_object()) {
    return as_object()->stringify();
  }
  else {
    switch (tag()) {
    case Tag::NaN: return "NaN";
    case Tag::NIL: return "nil";
    case Tag::TRUE: return "true";
    case Tag::FALSE: return "false";
    case Tag::UNDEFINED: UNREACHABLE();
    }
  }
  return "";
}

StringObject* ObjValue::as_string(void) const {
  return Xt::down<StringObject>(obj_);
}

const char* ObjValue::as_cstring(void) const {
  return Xt::down<StringObject>(obj_)->cstr();
}

ListObject* ObjValue::as_list(void) const {
  return Xt::down<ListObject>(obj_);
}

RangeObject* ObjValue::as_range(void) const {
  return Xt::down<RangeObject>(obj_);
}

MapObject* ObjValue::as_map(void) const {
  return Xt::down<MapObject>(obj_);
}

ModuleObject* ObjValue::as_module(void) const {
  return Xt::down<ModuleObject>(obj_);
}

FunctionObject* ObjValue::as_function(void) const {
  return Xt::down<FunctionObject>(obj_);
}

UpvalueObject* ObjValue::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(obj_);
}

ClosureObject* ObjValue::as_closure(void) const {
  return Xt::down<ClosureObject>(obj_);
}

FiberObject* ObjValue::as_fiber(void) const {
  return Xt::down<FiberObject>(obj_);
}

ClassObject* ObjValue::as_class(void) const {
  return Xt::down<ClassObject>(obj_);
}

InstanceObject* ObjValue::as_instance(void) const {
  return Xt::down<InstanceObject>(obj_);
}

bool ObjValue::is_same(const ObjValue& r) const noexcept {
  if (type_ != r.type_)
    return false;

  if (type_ == ValueType::NUMERIC)
    return num_ == r.num_;
  return obj_ == r.obj_;
}

bool ObjValue::is_equal(const ObjValue& r) const noexcept {
  if (is_same(r))
    return true;

  if (!is_object() || !r.is_object())
    return false;
  if (objtype() != r.objtype())
    return false;

  return obj_->is_equal(r.obj_);
}

u32_t ObjValue::hash(void) const {
  switch (type_) {
  case ValueType::NIL: return 1;
  case ValueType::TRUE: return 2;
  case ValueType::FALSE: return 0;
  case ValueType::NUMERIC: return hash_numeric(num_);
  case ValueType::OBJECT: return obj_->hash();
  default: UNREACHABLE(); return 0;
  }
  return 0;
}

str_t ObjValue::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(num_);
  case ValueType::OBJECT: return obj_->stringify();
  case ValueType::UNDEFINED: UNREACHABLE();
  }
  return "";
}

StringObject::StringObject(ClassObject* cls, char c) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(1) {
  value_ = new char[2];
  value_[0] = c;
  value_[1] = 0;

  hash_string();
}

StringObject::StringObject(
    ClassObject* cls, const char* s, int n, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(n) {
  if (replace_owner) {
    value_ = const_cast<char*>(s);
  }
  else {
    value_ = new char[Xt::as_type<sz_t>(size_ + 1)];
    if (s != nullptr) {
      memcpy(value_, s, n);
      value_[size_] = 0;
    }
  }

  hash_string();
}

StringObject::~StringObject(void) {
  delete [] value_;
}

void StringObject::hash_string(void) {
  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/
  u32_t hash = 2166136261u;

  // this is O(n) on the length of the string, but we only call this when a
  // new string is created, since the creation is also O(n) (to copy/initialize
  // all the bytes), we allow this here
  for (int i = 0; i < size_; ++i) {
    hash ^= value_[i];
    hash *= 16777619;
  }

  hash_ = hash;
}

int StringObject::find(StringObject* sub) const {
  if (sub->size_ == 0)
    return 0;
  if (sub->size_ > size_)
    return -1;

  char* first_occur = std::strstr(value_, sub->value_);
  return first_occur != nullptr ? Xt::as_type<int>(first_occur - value_) : -1;
}

bool StringObject::is_equal(BaseObject* r) const {
  auto* o = Xt::down<StringObject>(r);
  return size_ == o->size_ &&
    hash_ == o->hash_ && std::memcmp(value_, o->value_, size_) == 0;
}

str_t StringObject::stringify(void) const {
  return value_;
}

u32_t StringObject::hash(void) const {
  return hash_;
}

StringObject* StringObject::make_string(WrenVM& vm, char c) {
  auto* o = new StringObject(vm.str_cls(), c);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const char* s, int n) {
  ASSERT(s != nullptr, "unexpected null string");

  auto* o = new StringObject(vm.str_cls(), s, n);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const str_t& s) {
  return make_string(vm, s.data(), Xt::as_type<int>(s.size()));
}

StringObject* StringObject::make_string(
    WrenVM& vm, StringObject* s1, StringObject* s2) {
  int n = s1->size() + s2->size();
  char* s = new char [Xt::as_type<sz_t>(n + 1)];
  memcpy(s, s1->cstr(), s1->size());
  memcpy(s + s1->size(), s2->cstr(), s2->size());
  s[n] = 0;

  auto* o = new StringObject(vm.str_cls(), s, n, true);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::concat_string(
    WrenVM& vm, const char* s1, const char* s2) {
  sz_t n1 = strlen(s1);
  sz_t n2 = strlen(s2);
  sz_t n = n1 + n2;
  char* s = new char [n + 1];
  memcpy(s, s1, n1);
  memcpy(s + n1, s2, n2);
  s[n] = 0;

  auto* o = new StringObject(vm.str_cls(), s, Xt::as_type<int>(n), true);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::concat_string(
    WrenVM& vm, const str_t& s1, const str_t& s2) {
  return concat_string(vm, s1.data(), s2.data());
}

StringObject* StringObject::from_numeric(WrenVM& vm, double value) {
  // produces a string representation of [value]

  if (value != value)
    return make_string(vm, "nan");
  if (value == INFINITY)
    return make_string(vm, "infinity");
  if (value == -INFINITY)
    return make_string(vm, "-infinity");

  return make_string(vm, Xt::to_string(value));
}

StringObject* StringObject::format(WrenVM& vm, const char* format, ...) {
  // creates a new formatted string from [format] and any additional arguments
  // used in the format string
  //
  // this is a very restricted flavor of formatting, intended only for internal
  // use by the VM, two formatting characters are supported, each of which
  // reads the next argument as a certain type:
  //
  // $ - a C++ string
  // @ - a wren string object

  va_list ap;

  str_t text;
  va_start(ap, format);
  for (const char* c = format; *c != '\0'; ++c) {
    switch (*c) {
    case '$': text += va_arg(ap, str_t); break;
    case '@': text += va_arg(ap, Value).as_cstring(); break;
    default: text.push_back(*c); break; // any other charactor is interpreted literally
    }
  }
  va_end(ap);

  return make_string(vm, text);
}

ListObject::ListObject(ClassObject* cls, int num_elements) noexcept
  : BaseObject(ObjType::LIST, cls) {
  if (num_elements > 0)
    elements_.resize(num_elements);
}

str_t ListObject::stringify(void) const {
  std::stringstream ss;
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

void ListObject::gc_mark(WrenVM& vm) {
  for (auto& e : elements_)
    vm.mark_value(e);
}

ListObject* ListObject::make_list(WrenVM& vm, int num_elements) {
  auto* o = new ListObject(vm.list_cls(), num_elements);
  vm.append_object(o);
  return o;
}

bool RangeObject::is_equal(BaseObject* r) const {
  auto* o = Xt::down<RangeObject>(r);
  return from_ == o->from_ && to_ == o->to_ && is_inclusive_ == o->is_inclusive_;
}

str_t RangeObject::stringify(void) const {
  std::stringstream ss;
  ss << "[range `" << this << "`]";
  return ss.str();
}

u32_t RangeObject::hash(void) const {
  return hash_numeric(from_) ^ hash_numeric(to_);
}

RangeObject* RangeObject::make_range(
    WrenVM& vm, double from, double to, bool is_inclusive) {
  auto* o = new RangeObject(vm.range_cls(), from, to, is_inclusive);
  vm.append_object(o);
  return o;
}

bool MapObject::add_entry(std::vector<MapEntry>& entries,
    int capacity, const Value& k, const Value& v) {
  int index = k.hash() % capacity;

  while (true) {
    auto& entry = entries[index];

    // if we found an open slot, the key is not in the table
    if (entry.first.is_undefined()) {
      // do not stop at a tombstone, though, because the key may be found
      // after it
      if (!entry.second.as_boolean()) {
        entry.first = k;
        entry.second = v;
        return true;
      }
    }
    else if (entry.first == k) {
      // if the key already exists, just replace the value
      entry.second = v;
      return false;
    }

    // try the next slot
    index = (index + 1) % capacity;
  }
  return false;
}

void MapObject::grow(void) {
  if (count_ + 1 > capacity_ * kLoadPercent / 100) {
    int new_capacity = capacity_ * kGrowFactor;
    if (new_capacity < kMinCapacity)
      new_capacity = kMinCapacity;

    resize(new_capacity);
  }
}

void MapObject::resize(int new_capacity) {
  std::vector<MapEntry> new_entries(
      new_capacity, std::make_pair(kUndefined, false));
  if (capacity_ > 0) {
    for (auto& entry : entries_) {
      if (!entry.first.is_undefined())
        add_entry(new_entries, new_capacity, entry.first, entry.second);
    }
  }

  capacity_ = new_capacity;
  entries_.swap(new_entries);
}

int MapObject::find(const Value& key) const {
  // if there is no entry array (an empty map) we definitely won't find it
  if (capacity_ == 0)
    return -1;

  int index = key.hash() % capacity_;
  while (true) {
    auto& entry = entries_[index];

    if (entry.first.is_undefined()) {
      // if we found an empty slot, the key is not in the table, if we found
      // a slot that contains a deleted key, we have to keep looking
      if (!entry.second.as_boolean())
        break;
    }
    else if (entry.first == key) {
      return index;
    }

    index = (index + 1) % capacity_;
  }
  return -1;
}

void MapObject::clear(void) {
  entries_.clear();
  capacity_ = 0;
  count_ = 0;
}

bool MapObject::contains(const Value& key) const {
  if (auto val = get(key); val)
    return true;
  return false;
}

std::optional<Value> MapObject::get(const Value& key) const {
  int index = find(key);
  if (index == -1)
    return {};
  return {entries_[index].second};
}

void MapObject::set(const Value& key, const Value& val) {
  grow();
  if (add_entry(entries_, capacity_, key, val))
    ++count_;
}

Value MapObject::remove(const Value& key) {
  int index = find(key);
  if (index == -1)
    return nullptr;

  // remove the entry from the map
  Value value = entries_[index].second;
  entries_[index].first = kUndefined;
  entries_[index].second = true;

  --count_;
  if (count_ == 0) {
    clear();
  }
  else if (capacity_ > kMinCapacity &&
      count_ < capacity_ / kGrowFactor * kLoadPercent / 100) {
    int new_capacity = capacity_ / kGrowFactor;
    if (new_capacity < kMinCapacity)
      new_capacity = kMinCapacity;

    resize(new_capacity);
  }

  return value;
}

str_t MapObject::stringify(void) const {
  std::stringstream ss;
  ss << "[map `" << this << "`]";
  return ss.str();
}

void MapObject::gc_mark(WrenVM& vm) {
  for (auto& entry : entries_) {
    if (!entry.first.is_undefined()) {
      vm.mark_value(entry.first);
      vm.mark_value(entry.second);
    }
  }
}

MapObject* MapObject::make_map(WrenVM& vm) {
  auto* o = new MapObject(vm.map_cls());
  vm.append_object(o);
  return o;
}

DebugObject::DebugObject(const str_t& name,
    const str_t& source_path, int* source_lines, int lines_count) noexcept
  : name_(name)
  , source_path_(source_path)
  , source_lines_(source_lines, source_lines + lines_count) {
}

int ModuleObject::declare_variable(const str_t& name) {
  // adds a new implicitly declared top-level variable named [name] to [module]
  //
  // does not check to see if a variable with that name is already decalred or
  // defined, returns the symbol for the new variable or -2 if there are too
  // many variables defined

  if (variables_.size() == MAX_MODULE_VARS)
    return -2;

  variables_.push_back(Value());
  return variable_names_.add(name);
}

int ModuleObject::define_variable(const str_t& name, const Value& value) {
  // adds a new top-level variable named [name] to [module]
  //
  // returns the symbol for the new variable, -1 if a variable with the given
  // name is already defined or -2 if there are too many variables defined

  if (variables_.size() == MAX_MODULE_VARS)
    return -2;

  // see if the variable is already explicitly or implicitly declared
  int symbol = variable_names_.get(name);
  if (symbol == -1) {
    // brand new variable
    symbol = variable_names_.add(name);
    variables_.push_back(value);
  }
  else if (variables_[symbol].is_undefined()) {
    variables_[symbol] = value;
  }
  else {
    symbol = -1;
  }

  return symbol;
}

void ModuleObject::iter_variables(
    std::function<void (int, const Value&, const str_t&)>&& fn) {
  for (int i = 0; i < variables_.size(); ++i)
    fn(i, variables_[i], variable_names_.get_name(i));
}

str_t ModuleObject::stringify(void) const {
  std::stringstream ss;
  ss << "[module `" << this << "`]";
  return ss.str();
}

void ModuleObject::gc_mark(WrenVM& vm) {
  for (auto& v : variables_)
    vm.mark_value(v);
}

ModuleObject* ModuleObject::make_module(WrenVM& vm) {
  // modules are never used as first-class objects, so do not need a class
  auto* o = new ModuleObject();
  vm.append_object(o);
  return o;
}

FunctionObject::FunctionObject(
    ClassObject* cls,
    ModuleObject* module,
    int num_upvalues, int arity,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& source_path, const str_t& debug_name,
    int* source_lines, int lines_count) noexcept
  : BaseObject(ObjType::FUNCTION, cls)
  , module_(module)
  , num_upvalues_(num_upvalues)
  , codes_(codes, codes + codes_count)
  , constants_(constants, constants + constants_count)
  , arity_(arity)
  , debug_(debug_name, source_path, source_lines, lines_count) {
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

void FunctionObject::gc_mark(WrenVM& vm) {
  for (auto& c : constants_)
    vm.mark_value(c);
}

int FunctionObject::get_argc(
    const u8_t* bytecode, const Value* constants, int ip) {
  // returns the number of arguments to the instruction at [ip] in bytecode

  switch (Code instruction = Xt::as_type<Code>(bytecode[ip])) {
  case Code::NIL:
  case Code::FALSE:
  case Code::TRUE:
  case Code::POP:
  case Code::DUP:
  case Code::IS:
  case Code::CLOSE_UPVALUE:
  case Code::RETURN:
  case Code::END:
  case Code::LOAD_LOCAL_0:
  case Code::LOAD_LOCAL_1:
  case Code::LOAD_LOCAL_2:
  case Code::LOAD_LOCAL_3:
  case Code::LOAD_LOCAL_4:
  case Code::LOAD_LOCAL_5:
  case Code::LOAD_LOCAL_6:
  case Code::LOAD_LOCAL_7:
  case Code::LOAD_LOCAL_8:
    return 0;

  case Code::LOAD_LOCAL:
  case Code::STORE_LOCAL:
  case Code::LOAD_UPVALUE:
  case Code::STORE_UPVALUE:
  case Code::LOAD_FIELD_THIS:
  case Code::STORE_FIELD_THIS:
  case Code::LOAD_FIELD:
  case Code::STORE_FIELD:
  case Code::CLASS:
    return 1;

    // instructions with two arguments
  case Code::CONSTANT:
  case Code::LOAD_MODULE_VAR:
  case Code::STORE_MODULE_VAR:
  case Code::CALL_0:
  case Code::CALL_1:
  case Code::CALL_2:
  case Code::CALL_3:
  case Code::CALL_4:
  case Code::CALL_5:
  case Code::CALL_6:
  case Code::CALL_7:
  case Code::CALL_8:
  case Code::CALL_9:
  case Code::CALL_10:
  case Code::CALL_11:
  case Code::CALL_12:
  case Code::CALL_13:
  case Code::CALL_14:
  case Code::CALL_15:
  case Code::CALL_16:
  case Code::SUPER_0:
  case Code::SUPER_1:
  case Code::SUPER_2:
  case Code::SUPER_3:
  case Code::SUPER_4:
  case Code::SUPER_5:
  case Code::SUPER_6:
  case Code::SUPER_7:
  case Code::SUPER_8:
  case Code::SUPER_9:
  case Code::SUPER_10:
  case Code::SUPER_11:
  case Code::SUPER_12:
  case Code::SUPER_13:
  case Code::SUPER_14:
  case Code::SUPER_15:
  case Code::SUPER_16:
  case Code::JUMP:
  case Code::LOOP:
  case Code::JUMP_IF:
  case Code::AND:
  case Code::OR:
  case Code::METHOD_INSTANCE:
  case Code::METHOD_STATIC:
  case Code::LOAD_MODULE:
    return 2;

  case Code::IMPORT_VARIABLE:
    return 4;

  case Code::CLOSURE:
    {
      int constant = (bytecode[ip + 1] << 8) | (bytecode[ip + 2]);
      FunctionObject* loaded_fn = constants[constant].as_function();

      // there are two bytes for the constant, then two for each upvalue
      return 2 + (loaded_fn->num_upvalues() * 2);
    }
  default: UNREACHABLE();
  }
  return 0;
}

FunctionObject* FunctionObject::make_function(
    WrenVM& vm, ModuleObject* module,
    int num_upvalues, int arity,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& source_path, const str_t& debug_name,
    int* source_lines, int lines_count) {
  auto* o = new FunctionObject(
      vm.fn_cls(), module, num_upvalues, arity,
      codes, codes_count, constants, constants_count,
      source_path, debug_name, source_lines, lines_count);
  vm.append_object(o);
  return o;
}

UpvalueObject::UpvalueObject(Value* value, UpvalueObject* next) noexcept
  : BaseObject(ObjType::UPVALUE, nullptr)
  , value_(value)
  , next_(next) {
}

str_t UpvalueObject::stringify(void) const {
  std::stringstream ss;
  ss << "[upvalue `" << this << "`]";
  return ss.str();
}

void UpvalueObject::gc_mark(WrenVM& vm) {
  vm.mark_value(closed_);
}

UpvalueObject* UpvalueObject::make_upvalue(
    WrenVM& vm, Value* value, UpvalueObject* next) {
  auto* o = new UpvalueObject(value, next);
  vm.append_object(o);
  return o;
}

ClosureObject::ClosureObject(ClassObject* cls, FunctionObject* fn) noexcept
  : BaseObject(ObjType::CLOSURE, cls)
  , fn_(fn) {
  int num_upvalues = fn_->num_upvalues();
  if (num_upvalues > 0) {
    upvalues_ = new UpvalueObject*[num_upvalues];
    for (int i = 0; i < num_upvalues; ++i)
      upvalues_[i] = nullptr;
  }
}

ClosureObject::~ClosureObject(void) {
  if (upvalues_ != nullptr)
    delete [] upvalues_;
}

str_t ClosureObject::stringify(void) const {
  std::stringstream ss;
  ss << "[closure `" << this << "`]";
  return ss.str();
}

void ClosureObject::gc_mark(WrenVM& vm) {
  vm.mark_object(fn_);
  for (int i = 0; i < fn_->num_upvalues(); ++i)
    vm.mark_object(upvalues_[i]);
}

ClosureObject* ClosureObject::make_closure(WrenVM& vm, FunctionObject* fn) {
  auto* o = new ClosureObject(vm.fn_cls(), fn);
  vm.append_object(o);
  return o;
}

FiberObject::FiberObject(ClassObject* cls, BaseObject* fn) noexcept
  : BaseObject(ObjType::FIBER, cls) {
  stack_.reserve(kDefaultCap);
  const u8_t* ip;
  if (fn->type() == ObjType::FUNCTION)
    ip = Xt::down<FunctionObject>(fn)->codes();
  else
    ip = Xt::down<ClosureObject>(fn)->fn()->codes();
  frames_.push_back(CallFrame(ip, fn, 0));
}

void FiberObject::call_function(BaseObject* fn, int argc) {
  const u8_t* ip;
  if (fn->type() == ObjType::FUNCTION)
    ip = Xt::down<FunctionObject>(fn)->codes();
  else
    ip = Xt::down<ClosureObject>(fn)->fn()->codes();
  frames_.push_back(CallFrame(ip, fn, stack_size() - argc));
}

UpvalueObject* FiberObject::capture_upvalue(WrenVM& vm, int slot) {
  // capture the local variable in [slot] into an [upvalue], if that local is
  // already in an upvalue, the existing one will be used. otherwise it will
  // create a new open upvalue and add it into the fiber's list of upvalues

  Value* local = &stack_[slot];
  // if there are no open upvalues at all, we must need a new one
  if (open_upvlaues_ == nullptr) {
    open_upvlaues_ = UpvalueObject::make_upvalue(vm, local);
    return open_upvlaues_;
  }

  UpvalueObject* prev_upvalue = nullptr;
  UpvalueObject* upvalue = open_upvlaues_;

  // walk towards the bottom of the stack until we find a previously
  // existing upvalue or pass where it should be
  while (upvalue != nullptr && upvalue->value() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }
  // found an existing upvalue for this local
  if (upvalue != nullptr && upvalue->value() == local)
    return upvalue;

  // walked past this local on the stack, so there must not be an upvalue
  // for it already. make a new one and link it in the right place to keep
  // the list sorted
  UpvalueObject* created_upvalue = UpvalueObject::make_upvalue(vm, local, upvalue);
  if (prev_upvalue == nullptr)
    open_upvlaues_ = created_upvalue;
  else
    prev_upvalue->set_next(created_upvalue);
  return created_upvalue;
}

void FiberObject::close_upvalue(void) {
  UpvalueObject* upvalue = open_upvlaues_;

  // move the value into the upvalue itself and point the value to it
  upvalue->set_closed(upvalue->value_asref());
  upvalue->set_value(upvalue->closed_asptr());

  // remove it from the open upvalue list
  open_upvlaues_ = upvalue->next();
}

void FiberObject::close_upvalues(int slot) {
  Value* first = nullptr;
  if (slot < stack_.size())
    first = &stack_[slot];

  while (open_upvlaues_ != nullptr && open_upvlaues_->value() >= first)
    close_upvalue();
}

void FiberObject::riter_frames(
    std::function<void (const CallFrame&, FunctionObject*)>&& visit) {
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto& frame = *it;
    FunctionObject* fn;
    if (frame.fn->type() == ObjType::FUNCTION)
      fn = Xt::down<FunctionObject>(frame.fn);
    else
      fn = Xt::down<ClosureObject>(frame.fn)->fn();

    visit(frame, fn);
  }
}

void FiberObject::reset_fiber(BaseObject* fn) {
  stack_.clear();
  frames_.clear();

  stack_.reserve(kDefaultCap);
  open_upvlaues_ = nullptr;
  caller_ = nullptr;
  caller_is_trying_ = false;

  const u8_t* ip;
  if (fn->type() == ObjType::FUNCTION)
    ip = Xt::down<FunctionObject>(fn)->codes();
  else
    ip = Xt::down<ClosureObject>(fn)->fn()->codes();
  frames_.push_back(CallFrame(ip, fn, 0));
}

str_t FiberObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fiber " << this << "]";
  return ss.str();
}

void FiberObject::gc_mark(WrenVM& vm) {
  // stack functions
  for (auto& f : frames_)
    vm.mark_object(f.fn);

  // stack variables
  for (auto& v : stack_)
    vm.mark_value(v);

  // open upvalues
  for (auto* upvalue = open_upvlaues_;
      upvalue != nullptr; upvalue = upvalue->next())
    vm.mark_object(upvalue);

  vm.mark_object(caller_);
  vm.mark_object(error_);
}

FiberObject* FiberObject::make_fiber(WrenVM& vm, BaseObject* fn) {
  auto* o = new FiberObject(vm.fiber_cls(), fn);
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(void) noexcept
  : BaseObject(ObjType::CLASS, nullptr) {
}

ClassObject::ClassObject(ClassObject* meta_class,
    ClassObject* supercls, int num_fields, StringObject* name) noexcept
  : BaseObject(ObjType::CLASS, meta_class)
  , num_fields_(num_fields)
  , name_(name) {
  if (supercls != nullptr)
    bind_superclass(supercls);
}

void ClassObject::bind_superclass(ClassObject* superclass) {
  ASSERT(superclass != nullptr, "must have superclass");

  superclass_ = superclass;
  // include the superclass in the total number of fields
  num_fields_ += superclass->num_fields_;
  // inherit methods from its superclass
  int super_methods_count = superclass_->methods_count();
  for (int i = 0; i < super_methods_count; ++i)
    bind_method(i, superclass_->methods_[i]);
}

void ClassObject::bind_method(FunctionObject* fn) {
  int ip = 0;
  for (;;) {
    switch (Code c = Xt::as_type<Code>(fn->get_code(ip++))) {
    case Code::LOAD_FIELD:
    case Code::STORE_FIELD:
    case Code::LOAD_FIELD_THIS:
    case Code::STORE_FIELD_THIS:
      {
        auto num_fields = fn->get_code(ip) + superclass_->num_fields();
        fn->set_code(ip++, num_fields);
      } break;
    case Code::CLOSURE:
      {
        int constant = (fn->get_code(ip) << 8) | fn->get_code(ip + 1);
        bind_method(fn->get_constant(constant).as_function());
        ip += FunctionObject::get_argc(fn->codes(), fn->constants(), ip - 1);
      } break;

    case Code::END: return;
    default:
      // other instructions are unaffected, so just skip over them
      ip += FunctionObject::get_argc(fn->codes(), fn->constants(), ip - 1);
      break;
    }
  }
}

void ClassObject::bind_method(int i, int method_type, const Value& fn) {
  FunctionObject* method_fn = fn.is_function()
    ? fn.as_function() : fn.as_closure()->fn();

  // methods are always bound against the class, and not the metaclass, even
  // for static methods, so that constructors (which are static) get bound
  // like instance methods
  bind_method(method_fn);

  if (Xt::as_type<Code>(method_type) == Code::METHOD_STATIC )
    cls()->bind_method(i, fn.as_object());
  else
    bind_method(i, fn.as_object());
}

void ClassObject::bind_method(int i, const Method& method) {
  // make sure the buffer is big enough to reach the symbol's index

  while (i >= methods_count())
    methods_.push_back(Method());
  methods_[i] = method;
}

str_t ClassObject::stringify(void) const {
  std::stringstream ss;
  ss << "[class `" << this << "`]";
  return ss.str();
}

void ClassObject::gc_mark(WrenVM& vm) {
  vm.mark_object(cls());
  vm.mark_object(superclass_);
  vm.mark_object(name_);
  for (auto& m : methods_) {
    if (m.type == MethodType::BLOCK)
      vm.mark_object(m.fn());
  }
}

u32_t ClassObject::hash(void) const {
  return name_->hash();
}

ClassObject* ClassObject::make_single_class(WrenVM& vm, StringObject* name) {
  auto* o = new ClassObject(nullptr, nullptr, 0, name);
  vm.append_object(o);
  return o;
}

ClassObject* ClassObject::make_class(
    WrenVM& vm, ClassObject* superclass, int num_fields, StringObject* name) {
  // metaclasses always inherit Class and do not parallel the non-metaclass
  // hierarclly

  PinnedGuard pinned_name(vm, name);
  StringObject* metaclass_name = StringObject::format(vm, "@ metaclass", name);

  PinnedGuard pinned_metaclass_name(vm, metaclass_name);
  ClassObject* meta_class = new ClassObject(
      vm.class_cls(), vm.class_cls(), 0, metaclass_name);
  vm.append_object(meta_class);

  PinnedGuard pinned_metaclass(vm, meta_class);
  auto* o = new ClassObject(meta_class, superclass, num_fields, name);
  vm.append_object(o);
  return o;
}

InstanceObject::InstanceObject(ClassObject* cls) noexcept
  : BaseObject(ObjType::INSTANCE, cls)
  , fields_(cls->num_fields()) {
  for (int i = 0; i < cls->num_fields(); ++i)
    fields_[i] = nullptr;
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "[instance `" << this << "` of " << cls()->name_cstr() << "]";
  return ss.str();
}

void InstanceObject::gc_mark(WrenVM& vm) {
  vm.mark_object(cls());
  for (auto& v : fields_)
    vm.mark_value(v);
}

InstanceObject* InstanceObject::make_instance(WrenVM& vm, ClassObject* cls) {
  auto* o = new InstanceObject(cls);
  vm.append_object(o);
  return o;
}

}
