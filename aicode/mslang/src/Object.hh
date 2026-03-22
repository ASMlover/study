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

#include <array>
#include <fstream>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Common.hh"
#include "Chunk.hh"
#include "Table.hh"

namespace ms {

enum class GcGeneration : u8_t {
  YOUNG = 0,
  OLD   = 1,
};

enum class ObjectType : int {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_CLOSURE,
  OBJ_UPVALUE,
  OBJ_CLASS,
  OBJ_INSTANCE,
  OBJ_BOUND_METHOD,
  OBJ_LIST,
  OBJ_MAP,
  OBJ_MODULE,
  OBJ_STRING_BUILDER,
  OBJ_TUPLE,
  OBJ_FILE,
  OBJ_WEAK_REF,
  OBJ_COROUTINE,
};

class Object {
  ObjectType type_;
  bool is_marked_{false};
  bool finalized_{false};
  GcGeneration generation_{GcGeneration::YOUNG};
  u8_t age_{0};
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
  bool is_finalized() const noexcept { return finalized_; }
  void set_finalized(bool v) noexcept { finalized_ = v; }
  Object* next() const noexcept { return next_; }
  void set_next(Object* next) noexcept { next_ = next; }

  GcGeneration generation() const noexcept { return generation_; }
  void set_generation(GcGeneration gen) noexcept { generation_ = gen; }
  bool is_young() const noexcept { return generation_ == GcGeneration::YOUNG; }
  bool is_old() const noexcept { return generation_ == GcGeneration::OLD; }
  u8_t age() const noexcept { return age_; }
  void increment_age() noexcept { age_++; }
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

// --- Shape (Hidden Class) ---
class Shape {
  u32_t id_;
  std::unordered_map<ObjString*, u32_t> slots_;
  std::unordered_map<ObjString*, Shape*> transitions_;
  u32_t slot_count_{0};

public:
  explicit Shape(u32_t id) noexcept : id_(id) {}
  ~Shape() noexcept;

  u32_t id() const noexcept { return id_; }
  u32_t slot_count() const noexcept { return slot_count_; }
  i32_t find_slot(ObjString* name) const noexcept;
  Shape* add_transition(ObjString* name, u32_t& next_id) noexcept;
  void mark_keys() noexcept;
};

// --- Inline Cache ---
class ObjClass;

enum class ICKind : u8_t {
  IC_NONE,    // Empty cache
  IC_FIELD,   // Property is a field (skip getter/method lookups)
  IC_METHOD,  // Cached method closure
  IC_GETTER,  // Cached getter closure
  IC_SETTER,  // Cached setter closure
};

inline constexpr int kIC_PIC_SIZE = 4;

struct ICEntry {
  ObjClass* klass{nullptr};
  ICKind kind{ICKind::IC_NONE};
  Value cached{};
  u32_t shape_id{0};
  u32_t slot_index{0};
};

struct InlineCache {
  std::array<ICEntry, kIC_PIC_SIZE> entries{};
  u8_t count{0};           // number of valid entries (0..kIC_PIC_SIZE)
  bool megamorphic{false}; // true → skip IC entirely, use slow path

  // Find entry matching klass; returns pointer or nullptr
  inline ICEntry* find_entry(ObjClass* klass) noexcept {
    for (u8_t i = 0; i < count; ++i)
      if (entries[i].klass == klass) return &entries[i];
    return nullptr;
  }

  // Append entry; returns false if full → caller sets megamorphic
  inline bool append_entry(const ICEntry& e) noexcept {
    if (count == kIC_PIC_SIZE) return false;
    entries[count++] = e;
    return true;
  }
};

// --- ObjFunction ---
class ObjFunction final : public Object {
  int arity_{0};          // max arity (= param count including those with defaults)
  int min_arity_{0};      // params without defaults
  int default_base_{-1};  // K(default_base..) = default values (-1 = none)
  bool has_rest_param_{false};  // last param is variadic ...name
  bool is_generator_{false};    // fun* — calling creates ObjCoroutine
  int upvalue_count_{0};
  int max_stack_size_{0};
  Chunk chunk_;
  ObjString* name_{nullptr};
  str_t script_path_;
  std::vector<InlineCache> ic_;

public:
  ObjFunction() noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  int arity() const noexcept { return arity_; }
  void set_arity(int arity) noexcept { arity_ = arity; }
  void increment_arity() noexcept { arity_++; }
  int min_arity() const noexcept { return min_arity_; }
  void set_min_arity(int v) noexcept { min_arity_ = v; }
  int default_base() const noexcept { return default_base_; }
  void set_default_base(int v) noexcept { default_base_ = v; }
  bool has_rest_param() const noexcept { return has_rest_param_; }
  void set_has_rest_param(bool v) noexcept { has_rest_param_ = v; }
  bool is_generator() const noexcept { return is_generator_; }
  void set_is_generator(bool v) noexcept { is_generator_ = v; }
  int upvalue_count() const noexcept { return upvalue_count_; }
  int increment_upvalue_count() noexcept { return upvalue_count_++; }
  Chunk& chunk() noexcept { return chunk_; }
  const Chunk& chunk() const noexcept { return chunk_; }
  ObjString* name() const noexcept { return name_; }
  void set_name(ObjString* name) noexcept { name_ = name; }
  const str_t& script_path() const noexcept { return script_path_; }
  void set_script_path(strv_t path) noexcept { script_path_ = str_t(path); }

  int max_stack_size() const noexcept { return max_stack_size_; }
  void set_max_stack_size(int size) noexcept { max_stack_size_ = size; }

  // Inline cache support
  sz_t add_ic() noexcept { ic_.emplace_back(); return ic_.size() - 1; }
  InlineCache& ic_at(sz_t index) noexcept { return ic_[index]; }
  sz_t ic_count() const noexcept { return ic_.size(); }
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
  Table methods_;
  Table static_methods_;
  Table getters_;
  Table setters_;
  Table abstract_methods_;
  Shape* root_shape_{nullptr};
  u32_t next_shape_id_{1};

public:
  explicit ObjClass(ObjString* name) noexcept;
  ~ObjClass() noexcept override;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjString* name() const noexcept { return name_; }
  Table& methods() noexcept { return methods_; }
  const Table& methods() const noexcept { return methods_; }
  Table& static_methods() noexcept { return static_methods_; }
  const Table& static_methods() const noexcept { return static_methods_; }
  Table& getters() noexcept { return getters_; }
  const Table& getters() const noexcept { return getters_; }
  Table& setters() noexcept { return setters_; }
  const Table& setters() const noexcept { return setters_; }
  Table& abstract_methods() noexcept { return abstract_methods_; }
  const Table& abstract_methods() const noexcept { return abstract_methods_; }
  Shape* root_shape() const noexcept { return root_shape_; }
  u32_t& next_shape_id_ref() noexcept { return next_shape_id_; }
};

// --- ObjInstance ---
class ObjInstance final : public Object {
  ObjClass* klass_{nullptr};
  Shape* shape_{nullptr};
  std::vector<Value> fields_;

public:
  explicit ObjInstance(ObjClass* klass) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjClass* klass() const noexcept { return klass_; }
  Shape* shape() const noexcept { return shape_; }

  // Fast path: direct slot access
  Value get_field(u32_t slot) const noexcept { return fields_[slot]; }
  void set_field(u32_t slot, Value val) noexcept { fields_[slot] = val; }

  // Slow path: name-based lookup
  bool get_field(ObjString* name, Value* value) const noexcept;
  void set_field(ObjString* name, Value value) noexcept;
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

// --- ObjList ---
class ObjList final : public Object {
  std::vector<Value> elements_;

public:
  ObjList() noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  std::vector<Value>& elements() noexcept { return elements_; }
  const std::vector<Value>& elements() const noexcept { return elements_; }
  sz_t len() const noexcept { return elements_.size(); }
};

// --- ObjMap ---
struct ValueHash {
  sz_t operator()(const Value& v) const noexcept;
};

struct ValueEqual {
  bool operator()(const Value& a, const Value& b) const noexcept {
    return a.is_equal(b);
  }
};

using ValueMap = std::unordered_map<Value, Value, ValueHash, ValueEqual>;

class ObjMap final : public Object {
  ValueMap entries_;

public:
  ObjMap() noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ValueMap& entries() noexcept { return entries_; }
  const ValueMap& entries() const noexcept { return entries_; }
  sz_t len() const noexcept { return entries_.size(); }
};

// --- ObjModule ---
class ObjModule final : public Object {
  ObjString* name_{nullptr};
  Table exports_;

public:
  explicit ObjModule(ObjString* name) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjString* name() const noexcept { return name_; }
  Table& exports() noexcept { return exports_; }
  const Table& exports() const noexcept { return exports_; }
};

// --- ObjStringBuilder ---
class ObjStringBuilder final : public Object {
  str_t buffer_;

public:
  ObjStringBuilder() noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;

  str_t& buffer() noexcept { return buffer_; }
  const str_t& buffer() const noexcept { return buffer_; }
  sz_t len() const noexcept { return buffer_.size(); }
  void append(strv_t s) noexcept { buffer_.append(s); }
  void clear() noexcept { buffer_.clear(); }
};

// --- ObjTuple ---
class ObjTuple final : public Object {
  std::vector<Value> elements_;
  u32_t hash_{0};

public:
  explicit ObjTuple(std::vector<Value> elements) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  const std::vector<Value>& elements() const noexcept { return elements_; }
  sz_t len() const noexcept { return elements_.size(); }
  u32_t hash() const noexcept { return hash_; }

private:
  u32_t compute_hash() const noexcept;
};

// --- ObjFile ---
class ObjFile final : public Object {
  str_t path_;
  str_t mode_;
  std::unique_ptr<std::fstream> stream_;
  bool is_open_{false};

public:
  ObjFile(str_t path, str_t mode) noexcept;
  ~ObjFile() noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;

  const str_t& path() const noexcept { return path_; }
  const str_t& mode() const noexcept { return mode_; }
  bool is_open() const noexcept { return is_open_; }
  std::fstream& stream() noexcept { return *stream_; }

  bool open() noexcept;
  void close() noexcept;
  str_t read_all() noexcept;
  str_t read_line() noexcept;
  void write(strv_t data) noexcept;
  bool eof() const noexcept;
};

class ObjWeakRef final : public Object {
  Object* target_;

public:
  explicit ObjWeakRef(Object* target) noexcept;
  str_t stringify() const noexcept override;
  sz_t size() const noexcept override;
  // No trace_references: GC does not trace through weak refs

  Object* target() const noexcept { return target_; }
  void clear() noexcept { target_ = nullptr; }
  bool is_alive() const noexcept { return target_ != nullptr; }
};

// --- ObjCoroutine ---
// A coroutine wraps a generator closure. The VM manages save/restore of frame state.

enum class CoroutineState : u8_t { CREATED, RUNNING, SUSPENDED, DEAD };

// Properly-copyable snapshot of a CallFrame for coroutine save/restore.
// Avoids std::memcpy on non-trivially-copyable std::vector/Value members.
struct SavedCallFrame {
  ObjClosure* closure{nullptr};
  Instruction* ip{nullptr};
  ptrdiff_t slots_offset{0};  // slots relative to coroutine stack base
  std::vector<ObjClosure*> deferred{};
  Value pending_return{};
  bool returning{false};
};

class ObjCoroutine final : public Object {
  ObjClosure* closure_;
  CoroutineState state_{CoroutineState::CREATED};

  // Saved VM state (managed by VM)
  std::vector<SavedCallFrame> saved_frames_;  // properly-copied CallFrame snapshots
  std::vector<Value> saved_stack_;
  int saved_stack_top_offset_{0};  // number of saved stack values

  Value yielded_value_{};
  Value sent_value_{};
  std::vector<Value> init_args_;  // args captured when generator function is called

public:
  explicit ObjCoroutine(ObjClosure* closure) noexcept
      : Object(ObjectType::OBJ_COROUTINE), closure_(closure) {}

  str_t stringify() const noexcept override {
    return std::format("<coroutine {}>", closure_ ? closure_->function()->stringify() : "?");
  }
  void trace_references() noexcept override;
  sz_t size() const noexcept override { return sizeof(ObjCoroutine); }

  ObjClosure* closure() const noexcept { return closure_; }
  CoroutineState state() const noexcept { return state_; }
  void set_state(CoroutineState s) noexcept { state_ = s; }
  Value& yielded_value() noexcept { return yielded_value_; }
  const Value& yielded_value() const noexcept { return yielded_value_; }
  Value& sent_value() noexcept { return sent_value_; }

  std::vector<Value>& init_args() noexcept { return init_args_; }

  // Accessors for VM save/restore
  std::vector<SavedCallFrame>& saved_frames() noexcept { return saved_frames_; }
  std::vector<Value>& saved_stack() noexcept { return saved_stack_; }
  int& saved_stack_top_offset() noexcept { return saved_stack_top_offset_; }
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

inline ObjList* as_list(const Value& v) noexcept {
  return as_obj<ObjList>(v.as_object());
}

inline ObjMap* as_map(const Value& v) noexcept {
  return as_obj<ObjMap>(v.as_object());
}

inline ObjModule* as_module(const Value& v) noexcept {
  return as_obj<ObjModule>(v.as_object());
}

inline ObjStringBuilder* as_string_builder(const Value& v) noexcept {
  return as_obj<ObjStringBuilder>(v.as_object());
}

inline ObjTuple* as_tuple(const Value& v) noexcept {
  return as_obj<ObjTuple>(v.as_object());
}

inline ObjFile* as_file(const Value& v) noexcept {
  return as_obj<ObjFile>(v.as_object());
}

inline ObjWeakRef* as_weak_ref(const Value& v) noexcept {
  return as_obj<ObjWeakRef>(v.as_object());
}

inline ObjCoroutine* as_coroutine(const Value& v) noexcept {
  return as_obj<ObjCoroutine>(v.as_object());
}

inline strv_t as_cppstring(const Value& v) noexcept {
  return as_string(v)->value();
}

} // namespace ms
