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
#include <variant>
#include <vector>
#include "common.hh"
#include "utility.hh"
#include "wren.hh"

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
#if defined(_WRENCC_LINUX)
  ObjValue(long long n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(unsigned long long n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
#endif
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

  static StringObject* concat(WrenVM& vm, const char* s1, sz_t n1, const char* s2, sz_t n2);
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
  static StringObject* concat(WrenVM& vm, strv_t s1, strv_t s2);
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

struct CallFrame {
  // pointer to the current instruction in the function's bytecode
  const u8_t* ip{};

  // the closure being executed
  ClosureObject* closure{};

  // index to the first stack slot used by this call frame, this will contain
  // the receiver, followed by the function's parameters, then local variables
  // and temporaries
  int stack_start{};

  CallFrame() noexcept {}
  CallFrame(const u8_t* _ip, ClosureObject* _closure, int _stack_start) noexcept
    : ip(_ip), closure(_closure), stack_start(_stack_start) {
  }
};

// tracks how this fiber has been invoked, aside from ways that can be
// detected from the state of other fields in the fiber
enum class FiberState {
  // the fiber is being run from another fiber using a call to `try()`
  FIBER_TRY,

  // the fiber was directly invoked by `interpret()`, this means it's the
  // initial fiber used by a call to `wren_call()` or `wren_interpret()`
  FIBER_ROOT,

  // the fiber is invoked some other way, if [caller] is `nil` then the fiber
  // was invoked using `call()`, if [num_frames] is zero, then the fiber has
  // finished running and is done, if [num_frames] is one and that frame's
  // `ip` points to the first byte to code, the fiber has not been started yet
  FIBER_OTEHR,
};

class FiberObject final : public BaseObject {
  static constexpr sz_t kStackCapacity = 1<<10;
  static constexpr sz_t kFrameCapacity = 4;

  // the number of allocated slots in the stack array
  sz_t stack_capacity_{};

  // the stack of value slots, this is used for holding local variables and
  // temporaries while the fiber is executing, it heap-allocated and grown
  // as needed
  std::vector<Value> stack_;

  // the stack of call frames, this is a dynamic array that grows as needed
  // but never shrinks
  std::vector<CallFrame> frames_;

  // pointer to the first node in the linked list of open upvalues that are
  // pointing to values still on the stack. the headd of the list will be
  // the upvalue closest to the top of the stack, and then the list works
  // downwards
  UpvalueObject* open_upvalues_{};

  // the fiber that ran this one, if this fiber is yielded, control will
  // resume to this one, maybe `nil`
  FiberObject* caller_{};
  FiberState state_{FiberState::FIBER_OTEHR};

  // if the fiber failed because of a runtime error, this will contain the
  // error object, otherwise it will be nil
  Value error_{nullptr};

  FiberObject(ClassObject* cls, ClosureObject* closure) noexcept;
  virtual ~FiberObject() {}
public:
  inline Value* values_at(sz_t i = 0) noexcept { return stack_.data() + i; }
  inline const Value* values_at(sz_t i = 0) const noexcept { return stack_.data() + i; }
  inline Value* values_at_beg() noexcept { return values_at(); }
  inline const Value* values_at_beg() const noexcept { return values_at(); }
  inline Value* values_at_top() noexcept { return values_at(stack_.size()); }
  inline const Value* values_at_top() const noexcept { return values_at(stack_.size()); }
  inline void resize_stack(sz_t n) { stack_.resize(n); }
  inline sz_t stack_size() const noexcept { return stack_.size(); }
  inline sz_t frame_size() const noexcept { return frames_.size(); }
  inline void pop_frame() { frames_.pop_back(); }
  inline bool is_frame_empty() const noexcept { return frames_.empty(); }
  inline Value& get_value(sz_t i) noexcept { return stack_[i]; }
  inline const Value& get_value(sz_t i) const noexcept { return stack_[i]; }
  inline void set_value(sz_t i, Value v) noexcept { stack_[i] = v; }
  inline const CallFrame& get_frame(sz_t i) const noexcept { return frames_[i]; }
  inline FiberObject* caller() const noexcept { return caller_; }
  inline void set_fiber(FiberObject* caller) noexcept { caller_ = caller; }
  inline FiberState state() const noexcept { return state_; }
  inline void set_state(FiberState state) noexcept { state_ = state; }
  inline const Value& error() const noexcept { return error_; }
  inline const char* error_asstr() const noexcept { return error_.as_cstring(); }
  inline bool has_error() const noexcept { return !error_.is_nil(); }

  inline Value& peek_value(sz_t distance = 0) noexcept {
    return stack_[stack_.size() - 1 - distance];
  }

  inline const Value& peek_value(sz_t distance = 0) const noexcept {
    return stack_[stack_.size() - 1 - distance];
  }

  inline CallFrame& peek_frame(sz_t distance = 0) noexcept {
    return frames_[frames_.size() - 1 - distance];
  }

  inline const CallFrame& peek_frame(sz_t distance = 0) const noexcept {
    return frames_[frames_.size() - 1 - distance];
  }

  inline void push(Value v) noexcept {
    stack_.push_back(v);
  }

  inline Value pop() noexcept {
    Value r = stack_.back();
    stack_.pop_back();
    return r;
  }

  inline void set_value_safely(sz_t i, Value v) noexcept {
    if (i < stack_.size())
      stack_[i] = v;
    else
      stack_.push_back(v);
  }

  inline void reset_fiber() noexcept {
    stack_.clear();
    frames_.clear();
  }

  void ensure_stack(WrenVM& vm, sz_t needed);
  void call_function(WrenVM& vm, ClosureObject* closure, int argc = 0);
  UpvalueObject* capture_upvalue(WrenVM& vm, int slot);
  void close_upvalue();
  void close_upvalues(int slot);

  void riter_frames(
      std::function<void (const CallFrame&, FunctionObject*)>&& visitor);

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static FiberObject* create(WrenVM& vm, ClosureObject* closure);
};

// the type of a primitive function
//
// primitives are similiar to foreign functions, but have more direct access
// to VM internals, it is passed the arguments in [args], if it returns a
// value, it places it in `args[0]` and returns `true`, if it causes a runtime
// error or midifies the running fiber, it returns `false`
using PrimitiveFn = std::function<bool (WrenVM& vm, Value* args)>;

enum class MethodType {
  // a primitive method implemented in C++ in the vm, unlike foreign
  // methods, this can directly manipulate the fiber's stack
  PRIMITIVE,

  // a externally-defined C++ method
  FOREIGN,

  // a normal user-defined method
  BLOCK,

  // no method for the given symbol
  NONE,
};

struct Method {
  MethodType type{MethodType::NONE};

  // the method function itself, the [type] determines which field of the
  // variant is used
  std::variant<PrimitiveFn, WrenForeignFn, ClosureObject*> m{};

  Method() noexcept {}
  Method(MethodType t) noexcept : type(t) {}
  Method(const PrimitiveFn& fn) noexcept : type(MethodType::PRIMITIVE), m(fn) {}
  Method(const WrenForeignFn& fn) noexcept : type(MethodType::FOREIGN), m(fn) {}
  Method(ClosureObject* closure) noexcept : type(MethodType::BLOCK), m(closure) {}
  Method(MethodType t, const PrimitiveFn& fn) noexcept : type(t), m(fn) {}
  Method(MethodType t, const WrenForeignFn& fn) noexcept : type(t), m(fn) {}
  Method(MethodType t, ClosureObject* closure) noexcept : type(t), m(closure) {}

  inline MethodType get_type() const noexcept { return type; }
  inline void set_type(MethodType t) noexcept { type = t; }
  inline const PrimitiveFn& primitive() const noexcept { return std::get<PrimitiveFn>(m); }
  inline const WrenForeignFn& foreign() const noexcept { return std::get<WrenForeignFn>(m); }
  inline ClosureObject* closure() const noexcept { return std::get<ClosureObject*>(m); }

  inline Method& operator=(const PrimitiveFn& primitive) noexcept {
    type = MethodType::PRIMITIVE;
    m = primitive;
    return *this;
  }

  inline Method& operator=(const WrenForeignFn& foreign) noexcept {
    type = MethodType::FOREIGN;
    m = foreign;
    return *this;
  }

  inline Method& operator=(ClosureObject* closure) noexcept {
    type = MethodType::BLOCK;
    m = closure;
    return *this;
  }

  inline void assign(const PrimitiveFn& primitive) noexcept {
    type = MethodType::PRIMITIVE;
    m = primitive;
  }

  inline void assign(const WrenForeignFn& foreign) noexcept {
    type = MethodType::FOREIGN;
    m = foreign;
  }

  inline void assign(ClosureObject* closure) noexcept {
    type = MethodType::BLOCK;
    m = closure;
  }
};

class ClassObject final : public BaseObject {
  ClassObject* superclass_{};

  // the number of fields needed for an instance of this class, including
  // all of its superclass fields
  int num_fields_{};

  // the name of class
  StringObject* name_{};

  // the table of methods that are defined in or inherited by this class,
  // methods are called by symbol, and the symbol directly maps to an index
  // in this table, this makes method calls fast at the expense of empty
  // calls in the list for methods the class doesn't support
  //
  // you can think of it as a hash table that never has collisiions but has
  // a really low load factor, since methods are pretty small (just a type
  // and a pointer), this should be a worthwhile treade-off
  std::vector<Method> methods_;

  ClassObject() noexcept;
  ClassObject(ClassObject* metacls,
      ClassObject* supercls = nullptr, int num_fields = 0, StringObject* name = nullptr) noexcept;
public:
  inline ClassObject* superclass() const noexcept { return superclass_; }
  inline int num_fields() const noexcept { return num_fields_; }
  inline StringObject* name() const noexcept { return name_; }
  inline const char* name_cstr() const noexcept { return name_->cstr(); }
  inline sz_t methods_count() const noexcept { return methods_.size(); }
  inline Method& get_method(sz_t i) noexcept { return methods_[i]; }
  inline const Method& get_method(sz_t i) const noexcept { return methods_[i]; }
  inline void set_method(sz_t i, const PrimitiveFn& primitive) noexcept { methods_[i] = primitive; }
  inline void set_method(sz_t i, const WrenForeignFn& foreign) noexcept { methods_[i] = foreign; }
  inline void set_method(sz_t i, ClosureObject* closure) noexcept { methods_[i] = closure; }

  void bind_superclass(ClassObject* superclass);
  void bind_method(FunctionObject* fn);
  void bind_method(sz_t i, const Method& method);

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;
  virtual u32_t hasher() const override;

  // creates a new `raw` class, it has no metaclass or superclass whatsover
  // this is only used for bootstrapping the initial Object and Class classes,
  // which are a little special
  static ClassObject* create_raw(WrenVM& vm, StringObject* name = nullptr);

  // creates a new class object as well as its associated metaclass
  static ClassObject* create(WrenVM& vm,
      ClassObject* superclass = nullptr, int num_fields = 0, StringObject* name = nullptr);
};

class InstanceObject final : public BaseObject {
  std::vector<Value> fields_;

  InstanceObject(ClassObject* cls) noexcept;
public:
  inline Value& get_field(sz_t i) noexcept { return fields_[i]; }
  inline const Value& get_field(sz_t i) const noexcept { return fields_[i]; }
  inline void set_field(sz_t i, Value v) noexcept { fields_[i] = v; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static InstanceObject* create(WrenVM& vm, ClassObject* cls);
};

}
