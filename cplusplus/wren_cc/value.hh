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

#include <functional>
#include <optional>
#include <variant>
#include <vector>
#include "common.hh"
#include "utils.hh"
#include "wren.hh"

namespace wrencc {

enum class ValueType : u8_t {
  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  OBJECT,
  UNDEFINED,
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

class WrenVM;
class BaseObject;

class NilObject;
class BooleanObject;
class NumericObject;
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

class BaseObject : private UnCopyable {
  ObjType type_;
  bool is_dark_{};
  ClassObject* cls_{}; // the object's class
public:
  BaseObject(ObjType type, ClassObject* cls = nullptr) noexcept
    : type_(type), cls_(cls) {
  }
  virtual ~BaseObject(void) {}

  inline ObjType type(void) const { return type_; }
  inline bool is_darken(void) const { return is_dark_; }
  inline void set_darken(bool darken = true) { is_dark_ = darken; }
  inline ClassObject* cls(void) const { return cls_; }
  inline void set_cls(ClassObject* cls) { cls_ = cls; }

  virtual bool is_equal(BaseObject* r) const { return false; }
  virtual str_t stringify(void) const { return "<object>"; }
  virtual void gc_blacken(WrenVM& vm) {}
  virtual void finalize(WrenVM& vm) {}

  virtual u32_t hash(void) const {
    ASSERT(false, "only immutable objects can be hashed");
    return 0;
  }
};

enum Tag {
  MASK = 7,

  NaN = 0,
  NIL,
  FALSE,
  TRUE,
  UNDEFINED,
  UNUSED2,
  UNUSED3,
  UNUSED4,
};

class TagValue final : public Copyable {
  static constexpr u64_t kSignBit = 1llu << 63;
  static constexpr u64_t kQNaN = 0x7ffc000000000000llu;

  union {
    double num_;
    u64_t bits_;
  };

  template <typename T> inline void set_decimal(T x) {
    num_ = Xt::as_type<double>(x);
  }

  inline bool check(ObjType type) const {
    return is_object() && as_object()->type() == type;
  }
public:
  TagValue(void) noexcept { bits_ = (kQNaN | Tag::UNDEFINED); }
  TagValue(nil_t) noexcept { bits_ = (kQNaN | Tag::NIL); }
  TagValue(bool b) noexcept { bits_ = b ? (kQNaN | Tag::TRUE) : (kQNaN | Tag::FALSE); }
  TagValue(i8_t n) noexcept { set_decimal(n); }
  TagValue(u8_t n) noexcept { set_decimal(n); }
  TagValue(i16_t n) noexcept { set_decimal(n); }
  TagValue(u16_t n) noexcept { set_decimal(n); }
  TagValue(i32_t n) noexcept { set_decimal(n); }
  TagValue(u32_t n) noexcept { set_decimal(n); }
  TagValue(i64_t n) noexcept { set_decimal(n); }
  TagValue(u64_t n) noexcept { set_decimal(n); }
  TagValue(float n) noexcept { set_decimal(n); }
  TagValue(double d) noexcept { num_ = d; }
  TagValue(BaseObject* o) noexcept { bits_ = (kSignBit | kQNaN | (u64_t)(o)); }

  inline bool operator==(const TagValue& r) const noexcept { return is_equal(r); }
  inline bool operator!=(const TagValue& r) const noexcept { return !(*this == r); }

  inline int tag(void) const { return Xt::as_type<int>(bits_ & Tag::MASK); }
  inline ObjType objtype(void) const { return as_object()->type(); }

  inline bool is_nil(void) const { return bits_ == (kQNaN | Tag::NIL); }
  inline bool is_boolean(void) const { return (bits_ == (kQNaN | Tag::TRUE)) || (bits_ == (kQNaN | Tag::FALSE)); }
  inline bool is_numeric(void) const { return (bits_ & kQNaN) != kQNaN; }
  inline bool is_object(void) const { return (bits_ & (kSignBit | kQNaN)) == (kSignBit | kQNaN); }
  inline bool is_undefined(void) const { return bits_ == (kQNaN | Tag::UNDEFINED); }
  inline bool is_string(void) const { return check(ObjType::STRING); }
  inline bool is_list(void) const { return check(ObjType::LIST); }
  inline bool is_range(void) const { return check(ObjType::RANGE); }
  inline bool is_map(void) const { return check(ObjType::MAP); }
  inline bool is_module(void) const { return check(ObjType::MODULE); }
  inline bool is_function(void) const { return check(ObjType::FUNCTION); }
  inline bool is_foreign(void) const { return check(ObjType::FOREIGN); }
  inline bool is_upvalue(void) const { return check(ObjType::UPVALUE); }
  inline bool is_closure(void) const { return check(ObjType::CLOSURE); }
  inline bool is_fiber(void) const { return check(ObjType::FIBER); }
  inline bool is_class(void) const { return check(ObjType::CLASS); }
  inline bool is_instance(void) const { return check(ObjType::INSTANCE); }

  inline bool is_falsely(void) const { return is_nil() || bits_ == (kQNaN | Tag::FALSE); }

  inline bool as_boolean(void) const { return bits_ == (kQNaN | Tag::TRUE); }
  inline double as_numeric(void) const { return num_; }
  inline BaseObject* as_object(void) const { return (BaseObject*)(bits_ & ~(kSignBit | kQNaN)); }

  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
  ListObject* as_list(void) const;
  RangeObject* as_range(void) const;
  MapObject* as_map(void) const;
  ModuleObject* as_module(void) const;
  FunctionObject* as_function(void) const;
  ForeignObject* as_foreign(void) const;
  UpvalueObject* as_upvalue(void) const;
  ClosureObject* as_closure(void) const;
  FiberObject* as_fiber(void) const;
  ClassObject* as_class(void) const;
  InstanceObject* as_instance(void) const;

  inline bool is_same(const TagValue& r) const noexcept { return bits_ == r.bits_; }
  bool is_equal(const TagValue& r) const noexcept;

  u32_t hash(void) const;
  str_t stringify(void) const;
};

class ObjValue final : public Copyable {
  ValueType type_{};

  union {
    double num_{};
    BaseObject* obj_;
  };

  inline bool check(ObjType type) const { return is_object() && obj_->type() == type; }

  template <typename T> inline double to_decimal(T x) {
    return Xt::as_type<double>(x);
  }
public:
  ObjValue(void) noexcept : type_(ValueType::UNDEFINED) {}
  ObjValue(nil_t) noexcept : type_(ValueType::NIL) {}
  ObjValue(bool b) noexcept : type_(b ? ValueType::TRUE : ValueType::FALSE) {}
  ObjValue(i8_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(u8_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(i16_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(u16_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(i32_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(u32_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(i64_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(u64_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(float n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  ObjValue(double d) noexcept : type_(ValueType::NUMERIC), num_(d) {}
  ObjValue(BaseObject* o) noexcept : type_(ValueType::OBJECT), obj_(o) {}

  inline bool operator==(const ObjValue& r) const noexcept { return is_equal(r); }
  inline bool operator!=(const ObjValue& r) const noexcept { return !(*this == r); }

  inline ValueType type(void) const { return type_; }
  inline ObjType objtype(void) const { return obj_->type(); }
  inline bool is_valid(void) const { return type_ != ValueType::OBJECT || obj_ != nullptr; }

  inline bool is_nil(void) const { return type_ == ValueType::NIL; }
  inline bool is_boolean(void) const { return type_ == ValueType::TRUE || type_ == ValueType::FALSE; }
  inline bool is_numeric(void) const { return type_ == ValueType::NUMERIC; }
  inline bool is_object(void) const { return type_ == ValueType::OBJECT; }
  inline bool is_undefined(void) const { return type_ == ValueType::UNDEFINED; }
  inline bool is_string(void) const { return check(ObjType::STRING); }
  inline bool is_list(void) const { return check(ObjType::LIST); }
  inline bool is_range(void) const { return check(ObjType::RANGE); }
  inline bool is_map(void) const { return check(ObjType::MAP); }
  inline bool is_module(void) const { return check(ObjType::MODULE); }
  inline bool is_function(void) const { return check(ObjType::FUNCTION); }
  inline bool is_foreign(void) const { return check(ObjType::FOREIGN); }
  inline bool is_upvalue(void) const { return check(ObjType::UPVALUE); }
  inline bool is_closure(void) const { return check(ObjType::CLOSURE); }
  inline bool is_fiber(void) const { return check(ObjType::FIBER); }
  inline bool is_class(void) const { return check(ObjType::CLASS); }
  inline bool is_instance(void) const { return check(ObjType::INSTANCE); }

  inline bool is_falsely(void) const { return is_nil() || type_ == ValueType::FALSE; }

  inline BaseObject* as_object(void) const { return obj_; }
  inline bool as_boolean(void) const { return type_ == ValueType::TRUE; }
  inline double as_numeric(void) const { return num_; }

  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
  ListObject* as_list(void) const;
  RangeObject* as_range(void) const;
  MapObject* as_map(void) const;
  ModuleObject* as_module(void) const;
  FunctionObject* as_function(void) const;
  ForeignObject* as_foreign(void) const;
  UpvalueObject* as_upvalue(void) const;
  ClosureObject* as_closure(void) const;
  FiberObject* as_fiber(void) const;
  ClassObject* as_class(void) const;
  InstanceObject* as_instance(void) const;

  bool is_same(const ObjValue& r) const noexcept;
  bool is_equal(const ObjValue& r) const noexcept;
  u32_t hash(void) const;
  str_t stringify(void) const;
};

#if NAN_TAGGING
using Value = TagValue;
#else
using Value = ObjValue;
#endif

std::ostream& operator<<(std::ostream& out, const Value& val);

class StringObject final : public BaseObject {
  int size_{};
  u32_t hash_{};
  char* value_{};

  StringObject(ClassObject* cls, char c) noexcept;
  StringObject(ClassObject* cls,
      const char* s, int n, bool replace_owner = false) noexcept;
  virtual ~StringObject(void);

  void hash_string(void);
public:
  inline int size(void) const { return size_; }
  inline const char* cstr(void) const { return value_; }
  inline char operator[](int i) const { return value_[i]; }
  inline char& operator[](int i) { return value_[i]; }

  int find(StringObject* sub) const;

  virtual bool is_equal(BaseObject* r) const override;
  virtual str_t stringify(void) const override;
  virtual u32_t hash(void) const override;

  static StringObject* make_string(WrenVM& vm, char c);
  static StringObject* make_string(WrenVM& vm, const char* s, int n);
  static StringObject* make_string(WrenVM& vm, const str_t& s);
  static StringObject* make_string(WrenVM& vm, StringObject* s1, StringObject* s2);
  static StringObject* make_string_from_range(
      WrenVM& vm, StringObject* s, int off, int n, int step);
  static StringObject* concat_string(WrenVM& vm, const char* s1, const char* s2);
  static StringObject* concat_string(WrenVM& vm, const str_t& s1, const str_t& s2);
  static StringObject* from_numeric(WrenVM& vm, double value);
  static StringObject* format(WrenVM& vm, const char* format, ...);
};

class ListObject final : public BaseObject {
  std::vector<Value> elements_;

  ListObject(ClassObject* cls, int num_elements = 0) noexcept;
public:
  inline int count(void) const { return Xt::as_type<int>(elements_.size()); }
  inline const Value* elements(void) const { return elements_.data(); }
  inline const Value& get_element(int i) const { return elements_[i]; }
  inline void set_element(int i, const Value& e) { elements_[i] = e; }
  inline void clear(void) { elements_.clear(); }
  inline void add_element(const Value& e) { elements_.push_back(e); }
  inline void insert(int i, const Value& e) { elements_.insert(elements_.begin() + i, e); }
  inline Value remove(int i) {
    Value removed = elements_[i];
    elements_.erase(elements_.begin() + i);
    return removed;
  }

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ListObject* make_list(WrenVM& vm, int num_elements = 0);
};

class RangeObject final : public BaseObject {
  double from_{}; // the beginning of the range
  double to_{};   // the end of the range, may be greater or less than [from_]
  bool is_inclusive_{}; // true if [to] is included in the range

  RangeObject(ClassObject* cls, double from, double to, bool is_inclusive) noexcept
    : BaseObject(ObjType::RANGE, cls)
    , from_(from)
    , to_(to)
    , is_inclusive_(is_inclusive) {
  }
public:
  inline double from(void) const { return from_; }
  inline double to(void) const { return to_; }
  inline bool is_inclusive(void) const { return is_inclusive_; }

  virtual bool is_equal(BaseObject* r) const override;
  virtual str_t stringify(void) const override;
  virtual u32_t hash(void) const override;

  static RangeObject* make_range(WrenVM& vm,
      double from, double to, bool is_instance);
};

class MapObject final : public BaseObject {
  // a hash table mapping keys to values

  using MapEntry = std::pair<Value, Value>;

  // the maximum percentage of map entries that can be filled before the map
  // is grown, a lower load takes more memory but reduces collisiions which
  // makes lookup faster
  static constexpr int kLoadPercent = 75;
  static constexpr int kMinCapacity = 16;
  static constexpr int kGrowFactor = 2;

  int count_{};
  int capacity_{};
  std::vector<MapEntry> entries_;

  bool add_entry(std::vector<MapEntry>& entries,
      int capacity, const Value& k, const Value& v);
  void grow(void);
  void resize(int new_capacity);
  int find(const Value& key) const;
  MapObject(ClassObject* cls) noexcept : BaseObject(ObjType::MAP, cls) {}
public:
  inline int count(void) const { return count_; }
  inline int capacity(void) const { return capacity_; }
  inline const MapEntry& operator[](int i) const { return entries_[i]; }

  void clear(void);
  bool contains(const Value& key) const;
  std::optional<Value> get(const Value& key) const;
  void set(const Value& key, const Value& val);
  Value remove(const Value& key);

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static MapObject* make_map(WrenVM& vm);
};

// stores debugging information for a function used for things like stack
// traces
class DebugObject final : private UnCopyable {
  // the name of the function, heap allocated and owned by the FunctionObject
  str_t name_{};

  // an array of line numbers, there is one element in this array for each
  // bytecode in the function's bytecode array, the value of that element
  // is the line in the source code that generated that instruction
  std::vector<int> source_lines_;
public:
  DebugObject(void) noexcept {}
  DebugObject(const str_t& name, int* source_lines, int lines_count) noexcept;

  inline const str_t& name(void) const { return name_; }
  inline void set_name(const str_t& name) { name_ = name; }
  inline int get_line(int i) const { return source_lines_[i]; }
  inline void append_line(int i) { source_lines_.push_back(i); }
  inline void set_lines(std::initializer_list<int> lines) { source_lines_ = lines; }
};

class ModuleObject final : public BaseObject {
  // a loaded module and the top-level variables it defines
  //
  // while this is an BaseObject and is managed by the GC, it never appers
  // as a first-class object in Wren

  // the currently defined top-level variables
  std::vector<Value> variables_;

  // symbol table for the names of all module variables, indexes here directly
  // correspond to entries in [variables_]
  SymbolTable variable_names_;

  StringObject* name_{}; // name of the module

  ModuleObject(StringObject* name) noexcept
    : BaseObject(ObjType::MODULE), name_(name) {
  }
public:
  inline int count(void) const { return Xt::as_type<int>(variables_.size()); }
  inline int vars_count(void) const { return variable_names_.count(); }
  inline const Value& get_variable(int i) const { return variables_[i]; }
  inline void set_variable(int i, const Value& val) { variables_[i] = val; }
  inline int find_variable(const str_t& name) const { return variable_names_.get(name); }
  inline str_t get_variable_name(int i) const { return variable_names_.get_name(i); }
  inline StringObject* name(void) const { return name_; }
  inline const char* name_cstr(void) const { return name_ != nullptr ? name_->cstr() : ""; }

  int declare_variable(const str_t& name, int line);
  int define_variable(const str_t& name, const Value& value);
  void iter_variables(
      std::function<void (int, const Value&, const str_t&)>&& fn, int offset = 0);

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ModuleObject* make_module(WrenVM& vm, StringObject* name);
};

class FunctionObject final : public BaseObject {
  // the module where this function was defined
  ModuleObject* module_{};

  // the maximum number of stack slots this function may use
  int max_slots_{};

  int num_upvalues_{};
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;

  // the number of parameters this functon expects, used to ensure that `.call`
  // handles a mismatch between number of parameters and arguments, this will
  // only be set for fns, and not FunctionObjects that represent methods or
  // scripts
  int arity_{};
  DebugObject debug_;

  FunctionObject(ClassObject* cls, ModuleObject* module, int max_slots) noexcept;
  FunctionObject(
      ClassObject* cls, ModuleObject* module,
      int max_slots, int num_upvalues, int arity,
      u8_t* codes, int codes_count,
      const Value* constants, int constants_count,
      const str_t& debug_name, int* source_lines, int lines_count) noexcept;
public:
  inline int num_upvalues(void) const { return num_upvalues_; }
  inline void set_num_upvalues(int num_upvalues) { num_upvalues_ = num_upvalues; }
  inline int inc_num_upvalues(void) { return num_upvalues_++; }
  inline int max_slots(void) const { return max_slots_; }
  inline void set_max_slots(int max_slots) { max_slots_ = max_slots; }
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline void set_codes(const std::vector<u8_t>& codes) { codes_ = codes; }
  inline const Value* constants(void) const { return constants_.data(); }
  inline void set_constants(const std::vector<Value>& constants) { constants_ = constants; }
  inline int codes_count(void) const { return Xt::as_type<int>(codes_.size()); }
  inline int constants_count(void) const { return Xt::as_type<int>(constants_.size()); }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline const Value& get_constant(int i) const { return constants_[i]; }
  inline void set_constant(int i, const Value& v) { constants_[i] = v; }
  inline int arity(void) const { return arity_; }
  inline void set_arity(int arity) { arity_ = arity; }
  inline ModuleObject* module(void) const { return module_; }
  inline DebugObject& debug(void) { return debug_; }
  inline const DebugObject& debug(void) const { return debug_; }
  inline void bind_name(const str_t& name) { debug_.set_name(name); }

  template <typename T> inline void append_code(T c) {
    codes_.push_back(Xt::as_type<u8_t>(c));
  }

  template <typename T> inline void set_code(int i, T c) {
    codes_[i] = Xt::as_type<u8_t>(c);
  }

  inline void append_constant(const Value& v) {
    constants_.push_back(v);
  }

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static int get_argc(const u8_t* bytecode, const Value* constants, int ip);
  static FunctionObject* make_function(
      WrenVM& vm, ModuleObject* module, int max_slots);
  static FunctionObject* make_function(
      WrenVM& vm, ModuleObject* module,
      int max_slots, int num_upvalues, int arity,
      u8_t* codes, int codes_count,
      const Value* constants, int constants_count,
      const str_t& debug_name, int* source_lines, int lines_count);
};

class ForeignObject final : public BaseObject {
  std::vector<u8_t> data_;

  ForeignObject(ClassObject* cls, sz_t size) noexcept
    : BaseObject(ObjType::FOREIGN, cls)
    , data_(size) {
  }
public:
  inline u8_t* data(void) { return data_.data(); }
  inline const u8_t* data(void) const { return data_.data(); }

  virtual str_t stringify(void) const override;
  virtual void finalize(WrenVM& vm) override;

  static ForeignObject* make_foreign(WrenVM& vm, ClassObject* cls, sz_t size);
};

// the dynamically allocated data structure for a variable that has been
// used by a closure. whenever a function accesses a variable declared in
// an enclosing function, it will get to it through this.
//
// an upvalue can be either `closed` or `open`, an open upvalue points
// directly to a [value] that is still stored on the fiber's stack because
// the local variable is still in scope in the function where it's declared.
//
// when that local variable goes out of scope, the upvalue pointing to it
// will be closed. when that happens, the value gets copied off the stack
// into the upvalue itself, that way, it can have a longer lifetime than
// the stack variable.
class UpvalueObject final : public BaseObject {
  Value* value_{}; // pointer to the variable this upvalue is referencing

  // if the upvalue is closed then the closed-over value will be hoisted out
  // of the stack into here. [value] will then be changed to point to this.
  Value closed_{};

  // open upvalues are stored in a linked list by the fiber. this points to
  // the next upvalue in that list.
  UpvalueObject* next_{};

  UpvalueObject(Value* value, UpvalueObject* next = nullptr) noexcept;
  virtual ~UpvalueObject(void) {}
public:
  inline Value* value(void) const { return value_; }
  inline Value* value_asptr(void) const { return value_; }
  inline const Value& value_asref(void) const { return *value_; }
  inline void set_value(Value* value) { value_ = value; }
  inline void set_value(const Value& value) { *value_ = value; }
  inline const Value& closed(void) const { return closed_; }
  inline Value* closed_asptr(void) const { return const_cast<Value*>(&closed_); }
  inline const Value& closed_asref(void) const { return closed_; }
  inline void set_closed(const Value& closed) { closed_ = closed; }
  inline void set_closed(Value* closed) { closed_ = *closed; }
  inline UpvalueObject* next(void) const { return next_; }
  inline void set_next(UpvalueObject* next) { next_ = next; }

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static UpvalueObject* make_upvalue(
      WrenVM& vm, Value* value, UpvalueObject* next = nullptr);
};

class ClosureObject final : public BaseObject {
  FunctionObject* fn_{}; // function that this closure is an instance of
  UpvalueObject** upvalues_{}; // the upvalues this function has closed over

  ClosureObject(ClassObject* cls, FunctionObject* fn) noexcept;
  virtual ~ClosureObject(void);
public:
  inline FunctionObject* fn(void) const { return fn_; }
  inline bool has_upvalues(void) const { return upvalues_ != nullptr; }
  inline UpvalueObject** upvalues(void) const { return upvalues_; }
  inline UpvalueObject* get_upvalue(int i) const { return upvalues_[i]; }
  inline void set_upvalue(int i, UpvalueObject* upvalue) { upvalues_[i] = upvalue; }

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static ClosureObject* make_closure(WrenVM& vm, FunctionObject* fn);
};

struct CallFrame {
  // pointer to the current (really next-to-be-executed) instruction in the
  // function's bytecode
  const u8_t* ip{};

  // the closure being exected
  ClosureObject* closure{};

  // index of the first stack slot used by this call frame, this will contain
  // the receiver, followed by the function's parameters, then local variables
  // and temporaries
  int stack_start{};

  CallFrame(void) noexcept {}
  CallFrame(const u8_t* _ip, ClosureObject* _closure, int _stack_start) noexcept
    : ip(_ip), closure(_closure), stack_start(_stack_start) {
  }
};

class FiberObject final : public BaseObject {
  static constexpr sz_t kStackCapacity = 1<<10;
  static constexpr sz_t kFrameCapacity = 4;

  int stack_capacity_{}; // the number of allocated slots in the stack array
  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  // pointer to the first node in the linked list of open upvalues that are
  // pointing to values still on the stack. the head of the list will be the
  // upvalue closest to the top of the stack, and then the list works downwards
  UpvalueObject* open_upvlaues_{};

  // the fiber that ran this one, if this fiber is yielded, control will
  // resume to this one, it's may be `nullptr`
  FiberObject* caller_{};

  // if the fiber failed because of a runtime error, this will contain the
  // error object, otherwise it will be nil
  Value error_{nullptr};

  // this will be true id the caller that called this fiber did so using `try`
  // in that case, if this fiber fails with an error, the error will be given
  // to the caller
  bool caller_is_trying_{};

  FiberObject(ClassObject* cls, ClosureObject* closure) noexcept;
  virtual ~FiberObject(void) {}
public:
  inline void reset(void) {
    stack_.clear();
    frames_.clear();
  }

  inline Value* values_at(int i) { return stack_.data() + i; }
  inline const Value* values_at(int i) const { return stack_.data() + i; }
  inline Value* values_at_beg(void) { return stack_.data(); }
  inline const Value* values_at_beg(void) const { return stack_.data(); }
  inline Value* values_at_top(void) { return stack_.data() + stack_.size(); }
  inline const Value* values_at_top(void) const { return stack_.data() + stack_.size(); }
  inline void resize_stack(int n) { stack_.resize(n); }
  inline int stack_size(void) const { return Xt::as_type<int>(stack_.size()); }
  inline int frame_size(void) const { return Xt::as_type<int>(frames_.size()); }
  inline void pop_frame(void) { frames_.pop_back(); }
  inline bool empty_frame(void) const { return frames_.empty(); }
  inline Value& get_value(int i) { return stack_[i]; }
  inline const Value& get_value(int i) const { return stack_[i]; }
  inline void set_value(int i, const Value& v) { stack_[i] = v; }
  inline const CallFrame& get_frame(int i) const { return frames_[0]; }
  inline FiberObject* caller(void) const { return caller_; }
  inline void set_caller(FiberObject* caller) { caller_ = caller; }
  inline const Value& error(void) const { return error_; }
  inline const char* error_asstr(void) const { return error_.as_cstring(); }
  inline void set_error(const Value& error) { error_ = error; }
  inline bool caller_is_trying(void) const { return caller_is_trying_; }
  inline void set_caller_is_trying(bool b) { caller_is_trying_ = b; }

  inline const Value& peek_value(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
  }

  inline CallFrame& peek_frame(int distance = 0) {
    return frames_[frames_.size() - 1 - distance];
  }

  inline const CallFrame& peek_frame(int distance = 0) const {
    return frames_[frames_.size() - 1 - distance];
  }

  inline void push(const Value& v) {
    stack_.push_back(v);
  }

  inline Value pop(void) {
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }

  inline void set_value_safely(int i, const Value& v) {
    if (i < stack_size())
      stack_[i] = v;
    else
      stack_.push_back(v);
  }

  void ensure_stack(WrenVM& vm, int needed);
  void call_function(WrenVM& vm, ClosureObject* closure, int argc = 0);
  UpvalueObject* capture_upvalue(WrenVM& vm, int slot);
  void close_upvalue(void);
  void close_upvalues(int slot);

  void riter_frames(
      std::function<void (const CallFrame&, FunctionObject*)>&& visit);
  void reset_fiber(ClosureObject* closure);

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  // creates a new fiber object will invoke [closure], which can be a function
  // or closure
  static FiberObject* make_fiber(WrenVM& vm, ClosureObject* closure);
};

// the type of a primitive function
//
// primitives are similiar to foreign functions, but have more direct access
// to VM internals, it's passed the arguments in [args], if it returns a value
// it places it in `args[0]` and returns `true`, if it causes a runtime error
// or modifies the running fiber, it returns `false`
using PrimitiveFn = std::function<bool (WrenVM&, Value*)>;

enum class MethodType {
  PRIMITIVE,// a primitive method implemented in C that immediatelt returns a Value
  FOREIGN,  // a externally-defined C++ native method
  BLOCK,    // a normal user-defined method
  FNCALL,   // the special "call(...)" methods on function

  NONE,     // no method for the given symbol
};

struct Method {
  MethodType type{MethodType::NONE};
  std::variant<PrimitiveFn, WrenForeignFn, ClosureObject*> m_{};

  inline MethodType get_type(void) const { return type; }
  inline void set_type(MethodType method_type) { type = method_type; }
  inline const PrimitiveFn& primitive(void) const { return std::get<PrimitiveFn>(m_); }
  inline void set_primitive(const PrimitiveFn& fn) { m_ = fn; }
  inline const WrenForeignFn& foreign(void) const { return std::get<WrenForeignFn>(m_); }
  inline void set_foreign(const WrenForeignFn& fn) { m_ = fn; }
  inline ClosureObject* closure(void) const { return std::get<ClosureObject*>(m_); }
  inline void set_closure(ClosureObject* closure) { m_ = closure; }

  Method(void) noexcept {}
  Method(MethodType t) noexcept : type(t) {}
  Method(const PrimitiveFn& fn) noexcept : type(MethodType::PRIMITIVE), m_(fn) {}
  Method(const WrenForeignFn& fn) noexcept : type(MethodType::FOREIGN), m_(fn) {}
  Method(ClosureObject* closure) noexcept : type(MethodType::BLOCK), m_(closure) {}
  Method(MethodType t, const PrimitiveFn& fn) noexcept : type(t), m_(fn) {}
  Method(MethodType t, const WrenForeignFn& fn) noexcept : type(t), m_(fn) {}
  Method(MethodType t, ClosureObject* closure) noexcept : type(t), m_(closure) {}

  inline void assign(const PrimitiveFn& fn) noexcept {
    type = MethodType::PRIMITIVE;
    m_ = fn;
  }
  inline void assign(const WrenForeignFn& fn) noexcept {
    type = MethodType::FOREIGN;
    m_ = fn;
  }
  inline void assign(ClosureObject* closure) noexcept {
    type = MethodType::BLOCK;
    m_ = closure;
  }
};

class ClassObject final : public BaseObject {
  ClassObject* superclass_{};
  int num_fields_{};
  StringObject* name_{}; // the name of the class

  std::vector<Method> methods_{};

  ClassObject(void) noexcept;
  ClassObject(
      ClassObject* meta_class, ClassObject* supercls = nullptr,
      int num_fields = 0, StringObject* name = nullptr) noexcept;
public:
  inline ClassObject* superclass(void) const { return superclass_; }
  inline int num_fields(void) const { return num_fields_; }
  inline StringObject* name(void) const { return name_; }
  inline const char* name_cstr(void) const { return name_->cstr(); }
  inline int methods_count(void) const { return Xt::as_type<int>(methods_.size()); }
  inline Method& get_method(int i) { return methods_[i]; }
  inline const Method& get_method(int i) const { return methods_[i]; }
  inline void set_method(int i, const PrimitiveFn& fn) {
    methods_[i].type = MethodType::PRIMITIVE;
    methods_[i].set_primitive(fn);
  }
  inline void set_method(int i, const WrenForeignFn& fn) {
    methods_[i].type = MethodType::FOREIGN;
    methods_[i].set_foreign(fn);
  }
  inline void set_method(int i, ClosureObject* closure) {
    methods_[i].type = MethodType::BLOCK;
    methods_[i].set_closure(closure);
  }
  inline void set_method(int i, MethodType fn_type, ClosureObject* closure) {
    methods_[i].type = fn_type;
    methods_[i].set_closure(closure);
  }

  void bind_superclass(ClassObject* superclass);
  void bind_method(FunctionObject* fn);
  void bind_method(int i, const Method& method);

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;
  virtual u32_t hash(void) const override;

  static ClassObject* make_single_class(WrenVM& vm, StringObject* name = nullptr);
  static ClassObject* make_class(
      WrenVM& vm, ClassObject* superclass = nullptr,
      int num_fields = 0, StringObject* name = nullptr);
};

class InstanceObject final : public BaseObject {
  std::vector<Value> fields_;

  InstanceObject(ClassObject* cls) noexcept;
public:
  inline const Value& get_field(int i) const { return fields_[i]; }
  inline void set_field(int i, const Value& v) { fields_[i] = v; }

  virtual str_t stringify(void) const override;
  virtual void gc_blacken(WrenVM& vm) override;

  static InstanceObject* make_instance(WrenVM& vm, ClassObject* cls);
};

}
