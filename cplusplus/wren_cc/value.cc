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
#include <cmath>
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

inline u32_t hash_bits(u64_t hash) {
  // from V8's computeLongHash() whicn in turn cites:
  // Thomas Wang, Integer Hash Functions
  // http://www.concentric.net/~Ttwang/tech/inthash.htm
  hash = ~hash + (hash << 18);
  hash = hash ^ (hash >> 31);
  hash = hash * 21;
  hash = hash ^ (hash >> 11);
  hash = hash + (hash << 6);
  hash = hash ^ (hash >> 22);
  return Xt::as_type<u32_t>(hash & 0x3fffffff);
}

inline u32_t hash_numeric(double num) {
  // generates a hash code for [num]

  DoubleBits bits;
  bits.num = num;
  return hash_bits(bits.b64);
}

str_t BaseObject::type_asstr() const {
  switch (type_) {
  case ObjType::STRING: return "<STRING>";
  case ObjType::LIST: return "<LIST>";
  case ObjType::RANGE: return "<RANGE>";
  case ObjType::MAP: return "<MAP>";
  case ObjType::MODULE: return "<MODULE>";
  case ObjType::FUNCTION: return "<FUNCTION>";
  case ObjType::FOREIGN: return "<FOREIGN>";
  case ObjType::UPVALUE: return "<UPVALUE>";
  case ObjType::CLOSURE: return "<CLOSURE>";
  case ObjType::FIBER: return "<FIBER>";
  case ObjType::CLASS: return "<CLASS>";
  case ObjType::INSTANCE: return "<INSTANCE>";
  }
  return "<UNKNOWN>";
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

ForeignObject* TagValue::as_foreign(void) const {
  return Xt::down<ForeignObject>(as_object());
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
  return hash_bits(bits_);
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

ForeignObject* ObjValue::as_foreign(void) const {
  return Xt::down<ForeignObject>(obj_);
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

int StringObject::find(StringObject* sub, int start_index) const {
  if (sub->size_ == 0)
    return start_index;
  if (sub->size_ + start_index > size_)
    return -1;
  if (start_index >= size_)
    return -1;

  char* first_occur = std::strstr(value_ + start_index, sub->value_);
  return first_occur != nullptr ? Xt::as_type<int>(first_occur - value_) : -1;
}

bool StringObject::is_equal(BaseObject* r) const {
  auto* o = Xt::down<StringObject>(r);
  return compare(o);
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

StringObject* StringObject::make_string_from_range(
    WrenVM& vm, StringObject* s, int off, int n, int step) {
  str_t text;
  for (int i = 0; i < n; ++i) {
    int index = off + i * step;
    text.push_back(s->value_[index]);
  }

  return make_string(vm, text);
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

  if (std::isnan(value))
    return make_string(vm, "nan");
  if (std::isinf(value))
    return make_string(vm, value > 0.0 ? "infinity" : "-infinity");

  return make_string(vm, Xt::to_string(value));
}

StringObject* StringObject::from_byte(WrenVM& vm, u8_t value) {
  return make_string(vm, Xt::as_type<char>(value));
}

StringObject* StringObject::format(WrenVM& vm, const char* format, ...) {
  // creates a new formatted string from [format] and any additional arguments
  // used in the format string
  //
  // this is a very restricted flavor of formatting, intended only for internal
  // use by the VM, two formatting characters are supported, each of which
  // reads the next argument as a certain type:
  //
  // $ - a pure C string
  // # - a C++ string
  // @ - a wren string object

  va_list ap;

  str_t text;
  va_start(ap, format);
  for (const char* c = format; *c != '\0'; ++c) {
    switch (*c) {
    case '$': text += va_arg(ap, const char*); break;
    case '#': text += va_arg(ap, str_t); break;
    case '@': text += va_arg(ap, Value).as_cstring(); break;
    default: text.push_back(*c); break; // any other charactor is interpreted literally
    }
  }
  va_end(ap);

  return make_string(vm, text);
}

ListObject::ListObject(
    ClassObject* cls, int num_elements, const Value& v) noexcept
  : BaseObject(ObjType::LIST, cls) {
  if (num_elements > 0)
    elements_.resize(num_elements, v);
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

void ListObject::gc_blacken(WrenVM& vm) {
  for (auto& e : elements_)
    vm.gray_value(e);
}

ListObject* ListObject::make_list(
    WrenVM& vm, int num_elements, const Value& v) {
  auto* o = new ListObject(vm.list_cls(), num_elements, v);
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
      entry.first = k;
      entry.second = v;
      return true;
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

void MapObject::gc_blacken(WrenVM& vm) {
  for (auto& entry : entries_) {
    if (!entry.first.is_undefined()) {
      vm.gray_value(entry.first);
      vm.gray_value(entry.second);
    }
  }
}

MapObject* MapObject::make_map(WrenVM& vm) {
  auto* o = new MapObject(vm.map_cls());
  vm.append_object(o);
  return o;
}

DebugObject::DebugObject(
    const str_t& name, int* source_lines, int lines_count) noexcept
  : name_(name)
  , source_lines_(source_lines, source_lines + lines_count) {
}

int ModuleObject::find_variable(const str_t& name) const {
  for (sz_t i = 0; i < variable_names_.size(); ++i) {
    if (variable_names_[i]->compare(name))
      return Xt::as_type<int>(i);
  }
  return -1;
}

int ModuleObject::declare_variable(WrenVM& vm, const str_t& name, int line) {
  // adds a new implicitly declared top-level variable named [name] to [module]
  // based on a use site occurring on [line]
  //
  // does not check to see if a variable with that name is already decalred or
  // defined, returns the symbol for the new variable or -2 if there are too
  // many variables defined

  if (variables_.size() == MAX_MODULE_VARS)
    return -2;

  // implicitly defined variables get a "value" that is the line where the
  // variable is first used, we'll use that later to report an error on the
  // right line
  variables_.push_back(line);
  variable_names_.push_back(StringObject::make_string(vm, name));
  return Xt::as_type<int>(variable_names_.size()) - 1;
}

std::tuple<int, int> ModuleObject::define_variable(
    WrenVM& vm, const str_t& name, const Value& value) {
  // adds a new top-level variable named [name] to [module], and optionally
  // populates line with the line of the implicit first use (line can be 0)
  //
  // returns the symbol for the new variable, -1 if a variable with the given
  // name is already defined or -2 if there are too many variables defined
  // returns -3 if this is a top-level lowercase variable (localname) that
  // was used before being defined

  int line{0};
  if (variables_.size() == MAX_MODULE_VARS)
    return std::make_tuple(-2, line);

  // see if the variable is already explicitly or implicitly declared
  int symbol = find_variable(name);
  if (symbol == -1) {
    // brand new variable
    variable_names_.push_back(StringObject::make_string(vm, name));
    symbol = Xt::as_type<int>(variable_names_.size()) - 1;
    variables_.push_back(value);
  }
  else if (variables_[symbol].is_numeric()) {
    // an implicitly declared variable's value will always be a number, now
    // we have a real definition
    line = variables_[symbol].as_integer<int>();
    variables_[symbol] = value;

    // if this was a localname we want to error if it was referenced this
    // definition
    if (std::islower(name[0]))
      symbol = -3;
  }
  else {
    symbol = -1;
  }

  return std::make_tuple(symbol, line);
}

void ModuleObject::iter_variables(
    std::function<void (int, const Value&, StringObject*)>&& fn, int offset) {
  for (int i = offset; i < variables_.size(); ++i)
    fn(i, variables_[i], variable_names_[i]);
}

str_t ModuleObject::stringify(void) const {
  std::stringstream ss;
  ss << "[module `" << this << "`]";
  return ss.str();
}

void ModuleObject::gc_blacken(WrenVM& vm) {
  for (auto& v : variables_)
    vm.gray_value(v);
  for (auto& v : variable_names_)
    vm.gray_object(v);
  vm.gray_object(name_);
}

ModuleObject* ModuleObject::make_module(WrenVM& vm, StringObject* name) {
  // modules are never used as first-class objects, so do not need a class
  auto* o = new ModuleObject(name);
  vm.append_object(o);
  return o;
}

FunctionObject::FunctionObject(
    ClassObject* cls, ModuleObject* module, int max_slots) noexcept
  : BaseObject(ObjType::FUNCTION, cls)
  , module_(module)
  , max_slots_(max_slots) {
}

FunctionObject::FunctionObject(
    ClassObject* cls, ModuleObject* module,
    int max_slots, int num_upvalues, int arity,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& debug_name, int* source_lines, int lines_count) noexcept
  : BaseObject(ObjType::FUNCTION, cls)
  , module_(module)
  , max_slots_(max_slots)
  , num_upvalues_(num_upvalues)
  , codes_(codes, codes + codes_count)
  , constants_(constants, constants + constants_count)
  , arity_(arity)
  , debug_(debug_name, source_lines, lines_count) {
}

int FunctionObject::indexof_constant(const Value& v) const {
  for (sz_t i = 0; i < constants_.size(); ++i) {
    if (constants_[i] == v)
      return Xt::as_type<int>(i);
  }
  return -1;
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

void FunctionObject::gc_blacken(WrenVM& vm) {
  for (auto& c : constants_)
    vm.gray_value(c);
}

u32_t FunctionObject::hash(void) const {
  // allow bare (non-closure) functions so that we can use a map to find
  // existing constants in a function's constant table, this is only used
  // internally, since user code never sees a non-closure function, they
  // cannot use them as map keys

  return hash_numeric(arity_) ^ hash_numeric(codes_count());
}

int FunctionObject::get_argc(
    const u8_t* bytecode, const Value* constants, int ip) {
  // returns the number of arguments to the instruction at [ip] in bytecode

  switch (Code instruction = Xt::as_type<Code>(bytecode[ip])) {
  case Code::NIL:
  case Code::FALSE:
  case Code::TRUE:
  case Code::POP:
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
  case Code::CONSTRUCT:
  case Code::FOREIGN_CONSTRUCT:
  case Code::FOREIGN_CLASS:
  case Code::END_MODULE:
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
  case Code::JUMP:
  case Code::LOOP:
  case Code::JUMP_IF:
  case Code::AND:
  case Code::OR:
  case Code::METHOD_INSTANCE:
  case Code::METHOD_STATIC:
  case Code::IMPORT_MODULE:
    return 2;

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
    WrenVM& vm, ModuleObject* module, int max_slots) {
  auto* o = new FunctionObject(vm.fn_cls(), module, max_slots);
  vm.append_object(o);
  return o;
}

FunctionObject* FunctionObject::make_function(
    WrenVM& vm, ModuleObject* module,
    int max_slots, int num_upvalues, int arity,
    u8_t* codes, int codes_count,
    const Value* constants, int constants_count,
    const str_t& debug_name, int* source_lines, int lines_count) {
  auto* o = new FunctionObject(
      vm.fn_cls(), module, max_slots, num_upvalues, arity,
      codes, codes_count, constants, constants_count,
      debug_name, source_lines, lines_count);
  vm.append_object(o);
  return o;
}

str_t ForeignObject::stringify(void) const {
  std::stringstream ss;
  ss << "[foreign `" << this << "`]";
  return ss.str();
}

void ForeignObject::finalize(WrenVM& vm) {
  vm.finalize_foreign(this);
}

ForeignObject* ForeignObject::make_foreign(
    WrenVM& vm, ClassObject* cls, sz_t size) {
  auto* o = new ForeignObject(cls, size);
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

void UpvalueObject::gc_blacken(WrenVM& vm) {
  vm.gray_value(closed_);
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

void ClosureObject::gc_blacken(WrenVM& vm) {
  vm.gray_object(fn_);
  for (int i = 0; i < fn_->num_upvalues(); ++i)
    vm.gray_object(upvalues_[i]);
}

ClosureObject* ClosureObject::make_closure(WrenVM& vm, FunctionObject* fn) {
  auto* o = new ClosureObject(vm.fn_cls(), fn);
  vm.append_object(o);
  return o;
}

FiberObject::FiberObject(ClassObject* cls, ClosureObject* closure) noexcept
  : BaseObject(ObjType::FIBER, cls) {
  // add one slot for the unused implicit receiver slot that the compiler
  // assumes all functions have
  stack_capacity_ = closure == nullptr
    ? 1 : Xt::power_of_2ceil(closure->fn()->max_slots() + 1);
  stack_.reserve(stack_capacity_);
  frames_.reserve(kFrameCapacity);

  if (closure != nullptr) {
    // initialize the first call frame
    const u8_t* ip = closure->fn()->codes();
    frames_.push_back(CallFrame(ip, closure, 0));

    // the first slot always holds the closure
    stack_.push_back(closure);
  }
}

void FiberObject::ensure_stack(WrenVM& vm, int needed) {
  if (stack_capacity_ < needed) {
    stack_capacity_ = Xt::power_of_2ceil(needed);
    const Value* old_stack = stack_.data();
    stack_.reserve(stack_capacity_);

    // if the reallocation moves the stack, then we need to recalculate
    // every pointer that points into the old stack to into the same relative
    // distance in the new stack, we have to be a little careful about how
    // these are calculated because pointer subtraction is only well-defined
    // within a single array, hence the slightly redundant-looking arithmetic
    // below
    const Value* new_stack = stack_.data();
    if (new_stack != old_stack) {
      if (vm.get_api_stack() != nullptr)
        vm.set_api_stack_asptr(new_stack + (vm.get_api_stack() - old_stack));

      // open upvalues
      for (UpvalueObject* uv = open_upvlaues_; uv != nullptr; uv = uv->next())
        uv->set_value(new_stack + (uv->value() - old_stack));
    }
  }
}

void FiberObject::call_function(WrenVM& vm, ClosureObject* closure, int argc) {
  // grow the call frames if needed (use vector auto)

  // grow the stack capacity if needed
  int stack_count = Xt::as_type<int>(stack_.size());
  int needed = stack_count + closure->fn()->max_slots();
  ensure_stack(vm, needed);

  const u8_t* ip = closure->fn()->codes();
  frames_.push_back(CallFrame(ip, closure, stack_size() - argc));
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
    FunctionObject* fn = frame.closure->fn();

    visit(frame, fn);
  }
}

str_t FiberObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fiber " << this << "]";
  return ss.str();
}

void FiberObject::gc_blacken(WrenVM& vm) {
  // stack functions
  for (auto& f : frames_)
    vm.gray_object(f.closure);

  // stack variables
  for (auto& v : stack_)
    vm.gray_value(v);

  // open upvalues
  for (auto* upvalue = open_upvlaues_;
      upvalue != nullptr; upvalue = upvalue->next())
    vm.gray_object(upvalue);

  vm.gray_object(caller_);
  vm.gray_value(error_);
}

FiberObject* FiberObject::make_fiber(WrenVM& vm, ClosureObject* closure) {
  auto* o = new FiberObject(vm.fiber_cls(), closure);
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
  if (num_fields_ != -1) {
    num_fields_ += superclass->num_fields_;
  }
  else {
    ASSERT(superclass->num_fields_ == 0,
        "a foreign class cannot inherit from a class with fields");
  }

  // inherit methods from its superclass
  int super_methods_count = superclass_->methods_count();
  for (int i = 0; i < super_methods_count; ++i)
    bind_method(i, superclass_->methods_[i]);
}

void ClassObject::bind_method(FunctionObject* fn) {
  int ip = 0;
  for (;;) {
    switch (Code c = Xt::as_type<Code>(fn->get_code(ip))) {
    case Code::LOAD_FIELD:
    case Code::STORE_FIELD:
    case Code::LOAD_FIELD_THIS:
    case Code::STORE_FIELD_THIS:
      {
        auto num_fields = fn->get_code(ip) + superclass_->num_fields();
        fn->set_code(ip + 1, num_fields);
      } break;
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
      {
        // fill in the constant slot with a reference to the superclass
        int constant = (fn->get_code(ip + 3) << 8) | fn->get_code(ip + 4);
        fn->set_constant(constant, superclass_);
      } break;
    case Code::CLOSURE:
      {
        int constant = (fn->get_code(ip + 1) << 8) | fn->get_code(ip + 2);
        bind_method(fn->get_constant(constant).as_function());
      } break;
    case Code::END:
      return;
    default:
      // other instructions are unaffected, so just skip over them
      break;
    }
    ip += 1 + FunctionObject::get_argc(fn->codes(), fn->constants(), ip);
  }
}

void ClassObject::bind_method(int i, const Method& method) {
  // make sure the buffer is big enough to reach the symbol's index

  if (i >= methods_count())
    methods_.insert(methods_.end(), i - methods_count() + 1, Method());
  methods_[i] = method;
}

str_t ClassObject::stringify(void) const {
  std::stringstream ss;
  ss << "[class `" << name_cstr() << "` at `" << this << "`]";
  return ss.str();
}

void ClassObject::gc_blacken(WrenVM& vm) {
  vm.gray_object(cls());
  vm.gray_object(superclass_);
  vm.gray_object(name_);
  for (auto& m : methods_) {
    if (m.type == MethodType::BLOCK)
      vm.gray_object(m.closure());
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

void InstanceObject::gc_blacken(WrenVM& vm) {
  vm.gray_object(cls());
  for (auto& v : fields_)
    vm.gray_value(v);
}

InstanceObject* InstanceObject::make_instance(WrenVM& vm, ClassObject* cls) {
  auto* o = new InstanceObject(cls);
  vm.append_object(o);
  return o;
}

}
