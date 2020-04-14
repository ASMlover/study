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
#pragma once

#include <optional>
#include <tuple>
#include <vector>
#include "common.hh"
#include "utility.hh"

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

  inline str_t get_stringify(strv_t name) const {
    ss_t ss;
    ss << "[" << name << "`" << this << "`]";
    return ss.str();
  }

  StringObject* as_string() noexcept;
  const char* as_cstring() noexcept;
  ListObject* as_list() noexcept;
  RangeObject* as_range() noexcept;
  MapObject* as_map() noexcept;
  ModuleObject* as_module() noexcept;
  FunctionObject* as_function() noexcept;
  ForeignObject* as_foreign() noexcept;
  UpvalueObject* as_upvalue() noexcept;
  ClosureObject* as_closure() noexcept;
  FiberObject* as_fiber() noexcept;
  ClassObject* as_class() noexcept;
  InstanceObject* as_instance() noexcept;

  virtual bool is_equal(BaseObject* r) const { return false; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void gc_blacken(WrenVM& vm) {}
  virtual void initialize(WrenVM& vm) {}
  virtual void finalize(WrenVM& vm) {}

  virtual u32_t hasher() const {
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

  inline bool is(ObjType type) const noexcept {
    return is_object() && objtype() == type;
  }

  inline bool is_true() const noexcept { return type_ == ValueType::TRUE; }
  inline bool is_false() const noexcept { return type_ == ValueType::FALSE; }

  template <typename T>
  inline void set_decimal(T x) { as_.num = Xt::to_decimal(x); }
public:
  ObjValue() noexcept {}
  ObjValue(nil_t) noexcept : type_(ValueType::NIL) {}
  ObjValue(bool b) noexcept : type_(b ? ValueType::TRUE : ValueType::FALSE) {}
  ObjValue(i8_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u8_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i16_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u16_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i32_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u32_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i64_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u64_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(float n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(double d) noexcept : type_(ValueType::NUMERIC) { as_.num = d; }
  ObjValue(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.obj = o; }

  inline ValueType type() const noexcept { return type_; }
  inline ObjType objtype() const noexcept { return as_.obj->type(); }

  inline bool is_undefined() const noexcept { return type_ == ValueType::UNDEFINED; }
  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return is_true() || is_false(); }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return is(ObjType::STRING); }
  inline bool is_list() const noexcept { return is(ObjType::LIST); }
  inline bool is_range() const noexcept { return is(ObjType::RANGE); }
  inline bool is_map() const noexcept { return is(ObjType::MAP); }
  inline bool is_module() const noexcept { return is(ObjType::MODULE); }
  inline bool is_function() const noexcept { return is(ObjType::FUNCTION); }
  inline bool is_foreign() const noexcept { return is(ObjType::FOREIGN); }
  inline bool is_upvalue() const noexcept { return is(ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(ObjType::CLOSURE); }
  inline bool is_fiber() const noexcept { return is(ObjType::FIBER); }
  inline bool is_class() const noexcept { return is(ObjType::CLASS); }
  inline bool is_instance() const noexcept { return is(ObjType::INSTANCE); }

  inline bool is_falsely() const noexcept { return is_nil() || is_false(); }
  inline bool operator==(ObjValue r) const noexcept { return is_equal(r); }
  inline bool operator!=(ObjValue r) const noexcept { return !is_equal(r); }

  inline bool as_boolean() const noexcept { return is_true(); }
  template <typename Int = int>
  inline Int as_integer() const noexcept { return Xt::as_type<Int>(as_.num); }
  inline double as_numeric() const noexcept { return as_.num; }
  inline BaseObject* as_object() const noexcept { return as_.obj; }

  StringObject* as_string() const noexcept { return as_.obj->as_string(); }
  const char* as_cstring() const noexcept { return as_.obj->as_cstring(); }
  ListObject* as_list() const noexcept { return as_.obj->as_list(); }
  RangeObject* as_range() const noexcept { return as_.obj->as_range(); }
  MapObject* as_map() const noexcept { return as_.obj->as_map(); }
  ModuleObject* as_module() const noexcept { return as_.obj->as_module(); }
  FunctionObject* as_function() const noexcept { return as_.obj->as_function(); }
  ForeignObject* as_foreign() const noexcept { return as_.obj->as_foreign(); }
  UpvalueObject* as_upvalue() const noexcept { return as_.obj->as_upvalue(); }
  ClosureObject* as_closure() const noexcept { return as_.obj->as_closure(); }
  FiberObject* as_fiber() const noexcept { return as_.obj->as_fiber(); }
  ClassObject* as_class() const noexcept { return as_.obj->as_class(); }
  InstanceObject* as_instance() const noexcept { return as_.obj->as_instance(); }

  bool is_same(ObjValue r) const noexcept;
  bool is_equal(ObjValue r) const noexcept;
  u32_t hasher() const noexcept;
  str_t stringify() const noexcept;
};

using Value = ObjValue;

inline std::ostream& operator<<(std::ostream& out, Value val) noexcept {
  return out << val.stringify();
}

// a heap-allocated string object
class StringObject final : public BaseObject {
  // number of bytes in the string, not including the null terminator
  sz_t size_{};

  // the hash value of the string's contents
  u32_t hash_{};

  // string's bytes followed by a null terminator
  char* data_{};

  StringObject(ClassObject* cls, char c) noexcept;
  StringObject(ClassObject* cls,
      const char* s, sz_t n, bool replace_owner = false) noexcept;
  virtual ~StringObject() noexcept;

  void hash_string() noexcept;
public:
  inline sz_t size() const noexcept { return size_; }
  inline sz_t length() const noexcept { return size_; }
  inline bool empty() const noexcept { return size_ == 0; }
  inline const char* cstr() const noexcept { return data_; }
  inline char* data() noexcept { return data_; }
  inline const char* data() const noexcept { return data_; }
  inline char& operator[](sz_t i) { return data_[i]; }
  inline const char& operator[](sz_t i) const { return data_[i]; }
  inline char& at(sz_t i) { return data_[i]; }
  inline const char& at(sz_t i) const { return data_[i]; }

  inline bool compare(StringObject* s) const {
    return this == s || (hash_ == s->hash_ && size_ == s->size_
        && std::memcmp(data_, s->data_, size_) == 0);
  }

  inline bool compare(const str_t& s) const {
    return size_ == s.size() && std::memcmp(data_, s.data(), size_) == 0;
  }

  inline bool compare(const char* s, sz_t n) const {
    return size_ == n && std::memcmp(data_, s, size_) == 0;
  }

  int find(StringObject* sub, sz_t off = 0) const;

  virtual bool is_equal(BaseObject* r) const override;
  virtual str_t stringify() const override;
  virtual u32_t hasher() const override;

  static StringObject* create(WrenVM& vm, char c);
  static StringObject* create(WrenVM& vm, const char* s, sz_t n);
  static StringObject* create(WrenVM& vm, const str_t& s);
  static StringObject* concat(WrenVM& vm, StringObject* s1, StringObject* s2);
  static StringObject* concat(WrenVM& vm, const char* s1, const char* s2);
  static StringObject* concat(WrenVM& vm, const str_t& s1, const str_t& s2);
  static StringObject* from_byte(WrenVM& vm, u8_t value);
  static StringObject* from_numeric(WrenVM& vm, double value);
  static StringObject* from_range(
      WrenVM& vm, StringObject* s, sz_t off, sz_t n, sz_t step);
  static StringObject* format(WrenVM& vm, const char* format, ...);
};

class ListObject final : public BaseObject {
  std::vector<Value> elements_; // the elements in the list

  ListObject(ClassObject* cls, sz_t n = 0, Value v = nullptr) noexcept;
public:
  inline sz_t size() const noexcept { return elements_.size(); }
  inline sz_t length() const noexcept { return elements_.size(); }
  inline bool empty() const noexcept { return elements_.empty(); }
  inline Value* data() noexcept { return elements_.data(); }
  inline const Value* data() const noexcept { return elements_.data(); }
  inline Value& operator[](sz_t i) noexcept { return elements_[i]; }
  inline Value operator[](sz_t i) const noexcept { return elements_[i]; }
  inline Value& at(sz_t i) noexcept { return elements_.at(i); }
  inline Value at(sz_t i) const noexcept { return elements_.at(i); }
  inline void clear() noexcept { elements_.clear(); }
  inline void append(Value v) { elements_.push_back(v); }

  inline void insert(int i, Value v) {
    elements_.insert(elements_.begin() + i, v);
  }

  inline Value remove(int i) {
    Value r = elements_[i];
    elements_.erase(elements_.begin() + i);
    return r;
  }

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ListObject* create(WrenVM& vm, sz_t n = 0, Value v = nullptr);
};

class RangeObject final : public BaseObject {
  double from_{}; // the beginning of the range
  double to_{}; // the end of the range
  bool is_inclusive_{}; // true if [to_] is included in the range

  RangeObject(ClassObject* cls,
      double from, double to, bool is_inclusive) noexcept
    : BaseObject(ObjType::RANGE, cls)
    , from_(from)
    , to_(to)
    , is_inclusive_(is_inclusive) {
  }
public:
  inline double from() const noexcept { return from_; }
  inline double to() const noexcept { return to_; }
  inline bool is_inclusive() const noexcept { return is_inclusive_; }

  virtual bool is_equal(BaseObject* r) const override;
  virtual str_t stringify() const override;
  virtual u32_t hasher() const override;

  static RangeObject* create(
      WrenVM& vm, double from, double to, bool is_inclusive);
};

// a hash table mapping keys to values
class MapObject final : public BaseObject {
  using MapEntry = std::pair<Value, Value>;

  // the initial (and minimum) capacity of a non-empty map object
  static constexpr sz_t kMinCapacity = 16;

  // the rate at which a collection's capacity grows when the size exceeds
  // the current capacity, the new capacity will be determined by *multiplying*
  // the old capacity by this, growing geomertrically is necessary to ensure
  // that adding to a collection has O(1) amortized complexity
  static constexpr sz_t kGrowFactor = 2;

  // the maximum percentage of map entries that can be filled before the map
  // is grown, a lower load takes more memory but reduces collisions which
  // makes lookup faster
  static constexpr sz_t kLoadPercent = 75;

  sz_t capacity_{}; // the number of entries allocated
  sz_t size_{}; // the number of entries in the map
  std::vector<MapEntry> entries_; // contiguous array of [capacity_] entries

  std::tuple<bool, int> find_entry(Value key) const;
  bool insert_entry(Value k, Value v);
  void resize(sz_t new_capacity);

  MapObject(ClassObject* cls) noexcept
    : BaseObject(ObjType::MAP, cls) {
  }
public:
  inline sz_t size() const noexcept { return size_; }
  inline sz_t length() const noexcept { return size_; }
  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t capacity() const noexcept { return capacity_; }

  void clear();
  bool contains(Value key) const;
  std::optional<Value> get(Value key) const;
  void set(Value key, Value val);
  Value remove(Value key);

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static MapObject* create(WrenVM& vm);
};

// a loaded module and top-level variables it defines
//
// while this is an object and is managed by thc GC, it never appears
// as a first-class object
class ModuleObject final : public BaseObject {
  // the name of the module
  StringObject* name_{};

  // the currently defiend top-level variables
  std::vector<Value> variables_;

  // symbol table for the names of all module variables, indexes here
  // directly correspond to entries in [variables_]
  // SymbolTable variable_names_;
  std::vector<StringObject*> variable_names_;

  ModuleObject(StringObject* name) noexcept
    : BaseObject(ObjType::MODULE), name_(name) {
  }
public:
  inline sz_t size() const noexcept { return variables_.size(); }
  inline sz_t length() const noexcept { return variables_.size(); }
  inline bool empty() const noexcept { return variables_.empty(); }
  inline Value get_variable(sz_t i) const { return variables_[i]; }
  inline void set_variable(sz_t i, Value v) { variables_[i] = v; }
  inline StringObject* name() const { return name_; }

  inline StringObject* get_variable_name(sz_t i) const {
    return variable_names_[i];
  }

  inline const char* name_cstr() const {
    return name_ != nullptr ? name_->cstr() : "";
  }

  std::optional<sz_t> find_variable(const str_t& name) const;
  sz_t declare_variable(WrenVM& vm, const str_t& name, int line);
  std::tuple<int, int> define_variable(
      WrenVM& vm, const str_t& name, Value value);
  void iter_variables(
      std::function<void (sz_t, StringObject*, Value)>&& fn, int off = 0);

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ModuleObject* create(WrenVM& vm, StringObject* name);
};

// stores debugging information for a function used for things like stack
// traces
class FunctionDebug final : private UnCopyable {
  // the name of the function, heap allocated and owned by the FunctionDebug
  str_t name_{};

  // an array of line numbers, thers is one element in this array for each
  // bytecode in the function's bytecode array. the value of that element is
  // the line in the source code that generated that instruction
  std::vector<int> source_lines_;
public:
  FunctionDebug() noexcept {}

  FunctionDebug(const str_t& name, int* source_lines, int lines_count) noexcept
    : name_(name)
    , source_lines_(source_lines, source_lines + lines_count) {
  }

  inline const str_t& name() const noexcept { return name_; }
  inline void set_name(const str_t& name) noexcept { name_ = name; }
  inline int get_line(int i) const noexcept { return source_lines_[i]; }
  inline void append_line(int line) noexcept { source_lines_.push_back(line); }

  inline void set_lines(std::initializer_list<int> source_lines) noexcept {
    source_lines_ = source_lines;
  }
};

// a function object, it wraps and owns the bytecode and other debug information
// for a callable chunk of code
//
// function objects aare not passed around and invoked directly, instead, they
// are always referenced by an [ClosureObject] which is the real first-class
// representation of a function, this isn't strictly necessary if the function
// has no upvalues, but lets the rest of the VM assume all called objects will
// be closures
class FunctionObject final : public BaseObject {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;

  // the module where this function was defined
  ModuleObject* module_{};

  // the maximum number of stack slots this function may use
  int max_slots_{};

  // the number of upvalues this function closes over
  int num_upvalues_{};

  // the number of parameters this function expects, used to ensure that .call
  // handles a mismatch between number of parameters and arguments, this will
  // only be set for functions, and not FunctionObject that represent or scripts
  int arity_{};
  FunctionDebug debug_;

  FunctionObject(ClassObject* cls, ModuleObject* module, int max_slots) noexcept;
  FunctionObject(ClassObject* cls,
      u8_t* codes, int codes_count,
      Value* constants, int constants_count,
      ModuleObject* module, int max_slots, int num_upvalues, int arity,
      const str_t& debug_name, int* source_lines, int lines_count) noexcept;
public:
  inline ModuleObject* module() const noexcept { return module_; }
  inline int max_slots() const noexcept { return max_slots_; }
  inline void set_max_slots(int max_slots) noexcept { max_slots_ = max_slots; }
  inline int num_upvalues() const noexcept { return num_upvalues_; }
  inline void set_num_upvalues(int num_upvalues) noexcept { num_upvalues_ = num_upvalues; }
  inline int inc_num_upvalues() noexcept { return num_upvalues_++; }
  inline int arity() const noexcept { return arity_; }
  inline void set_arity(int arity) noexcept { arity_ = arity; }
  inline FunctionDebug& debug() noexcept { return debug_; }
  inline const FunctionDebug& debug() const noexcept { return debug_; }
  inline void bind_name(const str_t& name) noexcept { debug_.set_name(name); }

  inline const u8_t* codes() const { return codes_.data(); }
  inline void set_codes(const std::vector<u8_t>& codes) { codes_ = codes; }
  inline sz_t codes_count() const noexcept { return codes_.size(); }
  inline u8_t get_code(sz_t i) const noexcept { return codes_[i]; }
  inline const Value* constants() const { return constants_.data(); }
  inline void set_constants(const std::vector<Value>& constants) { constants_ = constants; }
  inline sz_t constants_count() const noexcept { return constants_.size(); }
  inline Value get_constant(sz_t i) const noexcept { return constants_[i]; }
  inline void set_constant(sz_t i, Value v) noexcept { constants_[i] = v; }

  template <typename T> inline void append_code(T c) noexcept {
    codes_.push_back(Xt::as_type<u8_t>(c));
  }

  template <typename T> inline void set_code(sz_t i, T c) noexcept {
    codes_[i] = Xt::as_type<u8_t>(c);
  }

  inline void append_constant(Value v) noexcept {
    constants_.push_back(v);
  }

  int indexof_constant(Value v) const;

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;
  virtual u32_t hasher() const override;

  static int get_argc(const u8_t* bytecode, const Value* constants, int ip);
  static FunctionObject* create(
      WrenVM& vm, ModuleObject* module, int max_slots);
  static FunctionObject* create(WrenVM& vm,
      u8_t* codes, int codes_count,
      Value* constants, int constants_count,
      ModuleObject* module, int max_slots, int num_upvalues, int arity,
      const str_t& name, int* source_lines, int lines_count);
};

class ForeignObject final : public BaseObject {
  std::vector<u8_t> data_;

  ForeignObject(ClassObject* cls, sz_t count) noexcept
    : BaseObject(ObjType::FOREIGN, cls)
    , data_(count) {
  }
public:
  inline u8_t* data() noexcept { return data_.data(); }
  inline const u8_t* data() const noexcept { return data_.data(); }

  virtual str_t stringify() const override;
  virtual void finalize(WrenVM& vm) override;

  static ForeignObject* create(WrenVM& vm, ClassObject* cls, sz_t count);
};

// the dynamically allocated data structure for a variable that has been used
// by a closure, whenever a function accesses a variable declared in an
// enclosing function, it will get to it through this
//
// an upvalue can be either "closed" or "open", an open upvalue points directly
// to a [Value] that is still stored on the fiber's stack because the local
// variable is still in scope in the function where it's declared
//
// when that local variable goes out of scope, the upvalue pointing to it will
// be closed, when that happens, the value gets copied off the stack into the
// upvalue itself, that way, it can have a longer lifetime than the stack
// variable
class UpvalueObject final : public BaseObject {
  // pointer to the variable this upvalue is referencing
  Value* value_{};

  // if the upvalue is closed then the closed-over value will be hoisted out
  // of the stack into here, [value_] will then be changed to point to this
  Value closed_{};

  // open upvalues are stored in a linked list by the fiber, this points to
  // the next upvalue in that list
  UpvalueObject* next_{};

  UpvalueObject(Value* value, UpvalueObject* next = nullptr) noexcept;
public:
  inline Value* value() const noexcept { return value_; }
  inline Value* value_asptr() const noexcept { return value_; }
  inline Value& value_asref() noexcept { return *value_; }
  inline void set_value(Value* value) noexcept { value_ = value; }
  inline void set_value(Value value) noexcept { *value_ = value; }
  inline Value closed() const noexcept { return closed_; }
  inline Value* closed_asptr() const noexcept { return (Value*)&closed_; }
  inline Value& closed_asref() noexcept { return closed_; }
  inline void set_closed(Value closed) noexcept { closed_ = closed; }
  inline void set_closed(Value* closed) noexcept { closed_ = *closed; }
  inline UpvalueObject* next() const noexcept { return next_; }
  inline void set_next(UpvalueObject* next) noexcept { next_ = next; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static UpvalueObject* create(
      WrenVM& vm, Value* value, UpvalueObject* next = nullptr);
};

// an instance of a first-class function and the environment it has closed
// over, unlike [FunctionObject], this has captured the upvalues that the
// function accesses
class ClosureObject final : public BaseObject {
  // the function that this closure is an instance of
  FunctionObject* fn_{};

  // the upvalues this function has closed over
  UpvalueObject** upvalues_{};

  ClosureObject(ClassObject* cls, FunctionObject* fn) noexcept;
  virtual ~ClosureObject() noexcept;
public:
  inline FunctionObject* fn() const noexcept { return fn_; }
  inline bool has_upvalues() const noexcept { return upvalues_ != nullptr; }
  inline UpvalueObject** upvalues() const noexcept { return upvalues_; }
  inline UpvalueObject* get_upvalue(int i) const noexcept { return upvalues_[i]; }

  inline void set_upvalue(int i, UpvalueObject* upvalue) noexcept {
    upvalues_[i] = upvalue;
  }

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ClosureObject* create(WrenVM& vm, FunctionObject* fn);
};

}
