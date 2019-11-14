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

#include <list>
#include "utils.hh"
#include "value.hh"
#include "compiler.hh"

namespace wrencc {

enum class Code : u8_t {
#undef CODEF
#define CODEF(c, _) c,
#include "codes_def.hh"
};

inline Code operator+(Code a, int b) {
  return Xt::as_type<Code>(Xt::as_type<int>(a) + b);
}

inline int operator-(Code a, Code b) {
  return Xt::as_type<int>(a) - Xt::as_type<int>(b);
}

struct Pinned {
  BaseObject* obj{};
  Pinned* prev{};
};

struct WrenHandle {
  // a handle to a value, basically just linked list of extra GC roots.
  //
  // note that even non-heap-allocated values can be stored here

  Value value{};

  WrenHandle* prev{};
  WrenHandle* next{};

  WrenHandle(const Value& v,
      WrenHandle* p = nullptr, WrenHandle* n = nullptr) noexcept
    : value(v), prev(p), next(p) {
  }

  void clear(void) {
    value = nullptr;
    prev = next = nullptr;
  }
};

class WrenVM final : private UnCopyable {
  // the maximum number of temporary objects that can be made visible to the
  // GC at one time
  static constexpr sz_t kMaxTempRoots = 5;
  static constexpr int kGrayCapacity = 4;

  using LoadModuleFn = std::function<str_t (WrenVM&, const str_t&)>;
  using BindForeignMethodFn = std::function<WrenForeignFn (
      WrenVM&, const str_t&, const str_t&, bool, const str_t&)>;
  using BindForeignClassFn = std::function<WrenForeignClass (
      WrenVM&, const str_t&, const str_t&)>;

  ClassObject* fn_class_{};
  ClassObject* bool_class_{};
  ClassObject* class_class_{};
  ClassObject* fiber_class_{};
  ClassObject* nil_class_{};
  ClassObject* num_class_{};
  ClassObject* obj_class_{};
  ClassObject* str_class_{};
  ClassObject* list_class_{};
  ClassObject* range_class_{};
  ClassObject* map_class_{};

  // the fiber that is currently running
  FiberObject* fiber_{};

  // the loaded modules, each key is an StringObject (expect for the main
  // module, whose key is nil) for the module's name and the value is the
  // ModuleObject for the module
  MapObject* modules_{};

  // the most recently imported module, more specifically, the module whose
  // code has most recently finished executing
  //
  // not treated like a GC root since the module is already in [modules_]
  ModuleObject* last_module_{};

  // how many bytes of object data have been allocated
  sz_t total_allocated_{};
  // the number of total allocated objects that will trigger the next GC
  sz_t next_gc_{1<<10}; // 1024 // class的methods超过1000时，回收会crash，method_many_methods.wren

  std::list<BaseObject*> objects_; // all currently allocated objects

  // the gray set for the garbage collector, this is the stack of unprocessed
  // objects while a garbage collection pass is in process
  std::vector<BaseObject*> gray_objects_;
  int gray_capacity_{kGrayCapacity};

  // the list of temporary roots, this is for temporarily or new objects
  // that are not otherwise reachable but should not be collected.
  //
  // they are organized as a stack of pointers stored in this array, this
  // implies that temporary roots need to have stack semantics: only the
  // most recently pushed object can be released
  std::vector<BaseObject*> temp_roots_;

  // pointer to the bottom of the range of stack slots available for use from
  // the C++ API, during a foreign method, this will be in the stack of the
  // fiber that is executing a method
  //
  // if not in a foreign method, this is initially nullptr, if the user
  // requests slots by calling wrenEnsureSlots(), a stack is created and this
  // is initialized
  Value* api_stack_{};

  // the function used to locate foreign functions
  BindForeignMethodFn bind_foreign_meth_{};

  // the function used to locate foreign classes
  BindForeignClassFn bind_foreign_cls_{};

  // the function used to laod modules
  LoadModuleFn load_module_fn_{};

  // the function used to report errors
  WrenErrorFn error_fn_{};

  // list of active value handles or nullptr if there are no handles
  WrenHandle* handles_{};

  // compiler and debugger data:
  //
  // the compiler that is currently compiling code, this is used so that
  // heap allocated objects used by the compiler can be found if a GC is
  // kicked off in the middle of a compile
  Compiler* compiler_{};

  // there is a single global symbol table for all method names on all classes
  // method calls are dispatched directly by index in this table
  SymbolTable method_names_;

  // user defined state data associated with the VM
  void* user_data_{};

  void print_stacktrace(void);
  void runtime_error(void);
  void method_not_found(ClassObject* cls, int symbol);
  bool check_arity(const Value& value, int argc);
  Value validate_superclass(
      const Value& name, const Value& supercls_val, int num_fields);
  void validate_api_slot(int slot) const;
  void bind_foreign_class(ClassObject* cls, ModuleObject* module);
  void create_class(int num_fields, ModuleObject* module);
  void create_foreign(FiberObject* fiber, Value* stack);

  ModuleObject* get_module(const Value& name) const;
  Value get_module_variable_impl(ModuleObject* module, const Value& variable_name);
  ClosureObject* compile_in_module(const Value& name,
      const str_t& source_bytes, bool is_expression, bool print_errors);
  WrenForeignFn find_foreign_method(const str_t& module_name,
      const str_t& class_name, bool is_static, const str_t& signature);

  void bind_method(int i, Code method_type,
      ModuleObject* module, ClassObject* cls, const Value& method_val);

  void call_foreign(FiberObject* fiber, const WrenForeignFn& foreign, int argc);

  InterpretRet interpret(FiberObject* fiber);

  void free_object(BaseObject* obj);
public:
  WrenVM(void) noexcept;
  ~WrenVM(void);

  inline void set_fn_cls(ClassObject* cls) { fn_class_ = cls; }
  inline void set_bool_cls(ClassObject* cls) { bool_class_ = cls; }
  inline void set_class_cls(ClassObject* cls) { class_class_ = cls; }
  inline void set_fiber_cls(ClassObject* cls) { fiber_class_ = cls; }
  inline void set_nil_cls(ClassObject* cls) { nil_class_ = cls; }
  inline void set_num_cls(ClassObject* cls) { num_class_ = cls; }
  inline void set_obj_cls(ClassObject* cls) { obj_class_ = cls; }
  inline void set_str_cls(ClassObject* cls) { str_class_ = cls; }
  inline void set_list_cls(ClassObject* cls) { list_class_ = cls; }
  inline void set_range_cls(ClassObject* cls) { range_class_ = cls; }
  inline void set_map_cls(ClassObject* cls) { map_class_ = cls; }

  inline ClassObject* fn_cls(void) const { return fn_class_; }
  inline ClassObject* bool_cls(void) const { return bool_class_; }
  inline ClassObject* class_cls(void) const { return class_class_; }
  inline ClassObject* fiber_cls(void) const { return fiber_class_; }
  inline ClassObject* nil_cls(void) const { return nil_class_; }
  inline ClassObject* num_cls(void) const { return num_class_; }
  inline ClassObject* obj_cls(void) const { return obj_class_; }
  inline ClassObject* str_cls(void) const { return str_class_; }
  inline ClassObject* list_cls(void) const { return list_class_; }
  inline ClassObject* range_cls(void) const { return range_class_; }
  inline ClassObject* map_cls(void) const { return map_class_; }

  inline SymbolTable& mnames(void) { return method_names_; }
  inline MapObject* modules(void) const { return modules_; }
  inline FiberObject* fiber(void) { return fiber_; }
  inline const FiberObject* fiber(void) const { return fiber_; }
  inline void set_fiber(FiberObject* fiber) { fiber_ = fiber; }
  inline void set_compiler(Compiler* compiler) { compiler_ = compiler; }
  inline LoadModuleFn get_load_fn(void) const { return load_module_fn_; }
  inline Value* get_api_stack(void) const { return api_stack_; }
  inline Value& get_api_stack(int i) { return api_stack_[i]; }
  inline const Value& get_api_stack(int i) const { return api_stack_[i]; }
  inline void set_api_stack(int i, const Value& v) { api_stack_[i] = v; }
  inline void set_load_fn(const LoadModuleFn& fn) { load_module_fn_ = fn; }
  inline void set_load_fn(LoadModuleFn&& fn) { load_module_fn_ = std::move(fn); }
  inline void set_error_fn(const WrenErrorFn& fn) { error_fn_ = fn; }
  inline void set_error_fn(WrenErrorFn&& fn) { error_fn_ = std::move(fn); }
  inline BindForeignMethodFn get_foreign_meth(void) const { return bind_foreign_meth_; }
  inline void set_foreign_meth(const BindForeignMethodFn& fn) { bind_foreign_meth_ = fn; }
  inline void set_foreign_meth(BindForeignMethodFn&& fn) { bind_foreign_meth_ = std::move(fn); }
  inline BindForeignClassFn get_foreign_cls(void) const { return bind_foreign_cls_; }
  inline void set_foreign_cls(const BindForeignClassFn& fn) { bind_foreign_cls_ = fn; }
  inline void set_foreign_cls(BindForeignClassFn&& fn) { bind_foreign_cls_ = std::move(fn); }
  inline void set_primitive(
      ClassObject* cls, const str_t& name, const PrimitiveFn& fn) {
    set_native(cls, name, fn);
  }
  inline void* get_user_data(void) const { return user_data_; }
  inline void set_user_data(void* data) { user_data_ = data; }

  InterpretRet interpret_in_module(
      const str_t& module, const str_t& source_bytes);
  void set_metaclasses(void);
  int declare_variable(ModuleObject* module, const str_t& name, int line);
  int define_variable(ModuleObject* module, const str_t& name, const Value& value);
  void set_native(ClassObject* cls, const str_t& name, const PrimitiveFn& fn);
  const Value& find_variable(ModuleObject* module, const str_t& name) const;
  void set_api_stack_asref(const Value& value);
  void set_api_stack_asptr(const Value* value);
  Value import_module(const Value& name);
  Value get_module_variable(const Value& module_name, const Value& variable_name);

  void push_root(BaseObject* obj);
  void pop_root(void);

  void collect(void);
  void append_object(BaseObject* obj);
  void gray_object(BaseObject* obj);
  void gray_value(const Value& val);
  void blacken_objects(void);

  ClassObject* get_class(const Value& val) const;
  InterpretRet interpret(const str_t& source_bytes);
  ClosureObject* compile_source(const str_t& module,
      const str_t& source_bytes, bool is_expression, bool print_errors = false);

  WrenHandle* make_call_handle(const str_t& signature);
  InterpretRet wren_call(WrenHandle* method);
  WrenHandle* make_handle(const Value& value);
  void release_handle(WrenHandle* value);
  void finalize_foreign(ForeignObject* foreign);

  int get_slot_count(void) const;
  WrenType get_slot_type(int slot) const;
  void ensure_slots(int num_slots);
  bool get_slot_bool(int slot) const;
  double get_slot_double(int slot) const;
  const char* get_slot_string(int slot) const;
  WrenHandle* get_slot_hanle(int slot);
  void* get_slot_foreign(int slot) const;

  void set_slot(int slot, const Value& value);
  void* set_slot_new_foreign(int slot, int class_slot, sz_t size);
  int get_list_count(int slot);
  void get_list_element(int list_slot, int index, int element_slot);
  void insert_into_list(int list_slot, int index, int element_slot);
  void get_variable(const str_t& module, const str_t& name, int slot);
  void abort_fiber(int slot);
};

class PinnedGuard final : private UnCopyable {
  WrenVM& vm_;
  bool guard_{};
public:
  PinnedGuard(WrenVM& vm) noexcept : vm_(vm) {}

  PinnedGuard(WrenVM& vm, BaseObject* obj) noexcept
    : vm_(vm) {
    vm_.push_root(obj);
    guard_ = true;
  }

  ~PinnedGuard(void) noexcept {
    if (guard_)
      vm_.pop_root();
  }

  inline void set_guard_object(BaseObject* obj) noexcept {
    vm_.push_root(obj);
    guard_ = true;
  }
};

}
