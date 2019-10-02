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
#define CODEF(c) c,
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

enum class InterpretRet {
  SUCCESS,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

struct WrenMethod {
  // the fiber that invokes the method, it's stack is pre-populated with the
  // receiver for the method, and it contains a single callframe whose function
  // is the bytecode stub to invoke the method
  FiberObject* fiber{};
  WrenMethod* prev{};
  WrenMethod* next{};

  WrenMethod(FiberObject* fb,
      WrenMethod* p = nullptr, WrenMethod* n = nullptr) noexcept
    : fiber(fb), prev(p), next(n) {
  }

  void clear(void) {
    fiber = nullptr;
    prev = next = nullptr;
  }
};

struct WrenValue {
  // a handle to a value, basically just linked list of extra GC roots.
  //
  // note that even non-heap-allocated values can be stored here

  Value value{};

  WrenValue* prev{};
  WrenValue* next{};

  WrenValue(const Value& v,
      WrenValue* p = nullptr, WrenValue* n = nullptr) noexcept
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

  using LoadModuleFn = std::function<str_t (WrenVM&, const str_t&)>;
  using BindForeignFn = std::function<WrenForeignFn (
      WrenVM&, const str_t&, const str_t&, bool, const str_t&)>;

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

  // the ID that will be assigned to the next fiber that is allocated, Fibers
  // are given unique-ish (not completely unique since this can overflow) IDs
  // so that they can be used as map keys
  u16_t next_fiberid_{1};

  // the loaded modules, each key is an StringObject (expect for the main
  // module, whose key is nil) for the module's name and the value is the
  // ModuleObject for the module
  MapObject* modules_{};

  // how many bytes of object data have been allocated
  sz_t total_allocated_{};
  // the number of total allocated objects that will trigger the next GC
  sz_t next_gc_{1<<10}; // 1024 // class的methods超过1000时，回收会crash，method_many_methods.wren

  std::list<BaseObject*> objects_; // all currently allocated objects

  // the list of temporary roots, this is for temporarily or new objects
  // that are not otherwise reachable but should not be collected.
  //
  // they are organized as a stack of pointers stored in this array, this
  // implies that temporary roots need to have stack semantics: only the
  // most recently pushed object can be released
  std::vector<BaseObject*> temp_roots_;

  // during a foreign function call, this will point to the first argument
  // of the call on the fiber's stack
  Value* foreign_call_slot_{};
  // during a foreign function call, this will contain the number of arguments
  // to the function
  int foreign_call_argc_{};

  // the function used to locate foreign functions
  BindForeignFn bind_foreign_fn_{};

  // the function used to laod modules
  LoadModuleFn load_module_fn_{};

  // list of active method handles or nullptr if there are no handles
  WrenMethod* method_handles_{};

  // list of active value handles or nullptr if there are no handles
  WrenValue* value_handles_{};

  // compiler and debugger data:
  //
  // the compiler that is currently compiling code, this is used so that
  // heap allocated objects used by the compiler can be found if a GC is
  // kicked off in the middle of a compile
  Compiler* compiler_{};

  // there is a single global symbol table for all method names on all classes
  // method calls are dispatched directly by index in this table
  SymbolTable method_names_;

  void print_stacktrace(FiberObject* fiber);
  FiberObject* runtime_error(FiberObject* fiber, const Value& error);
  Value method_not_found(ClassObject* cls, int symbol);
  Value validate_superclass(const Value& name, const Value& supercls_val) ;
  void validate_foreign_argument(int index) const;

  ModuleObject* get_module(const Value& name) const;
  FiberObject* load_module(const Value& name, const str_t& source_bytes);
  Value import_module(const Value& name);
  std::tuple<bool, Value> import_variable(
      const Value& module_name, const Value& variable_name);
  InterpretRet load_into_core(const str_t& source_bytes);
  FunctionObject* make_call_stub(ModuleObject* module, const str_t& signature);
  WrenForeignFn find_foreign_method(const str_t& module_name,
      const str_t& class_name, bool is_static, const str_t& signature);

  Value bind_method(int i, Code method_type,
      ModuleObject* module, ClassObject* cls, const Value& method_val);

  void call_foreign(FiberObject* fiber, const WrenForeignFn& foreign, int argc);

  InterpretRet interpret(FiberObject* fiber);

  void collect(void);
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
  inline void set_compiler(Compiler* compiler) { compiler_ = compiler; }
  inline void set_load_fn(const LoadModuleFn& fn) { load_module_fn_ = fn; }
  inline void set_load_fn(LoadModuleFn&& fn) { load_module_fn_ = std::move(fn); }
  inline void set_foreign_fn(const BindForeignFn& fn) { bind_foreign_fn_ = fn; }
  inline void set_foreign_fn(BindForeignFn&& fn) { bind_foreign_fn_ = std::move(fn); }
  inline void set_primitive(
      ClassObject* cls, const str_t& name, const PrimitiveFn& fn) {
    set_native(cls, name, fn);
  }
  inline u16_t gen_fiberid(void) { return next_fiberid_++; }

  ModuleObject* get_core_module(void) const;
  void set_metaclasses(void);
  int declare_variable(ModuleObject* module, const str_t& name);
  int define_variable(ModuleObject* module, const str_t& name, const Value& value);
  void set_native(ClassObject* cls, const str_t& name, const PrimitiveFn& fn);
  const Value& find_variable(ModuleObject* module, const str_t& name) const;

  void push_root(BaseObject* obj);
  void pop_root(void);
  Value import_module(const str_t& name);

  void append_object(BaseObject* obj);
  void mark_object(BaseObject* obj);
  void mark_value(const Value& val);

  ClassObject* get_class(const Value& val) const;
  InterpretRet interpret(const str_t& source_path, const str_t& source_bytes);
  void call_function(FiberObject* fiber, BaseObject* fn, int argc);

  WrenMethod* acquire_method(
      const str_t& module, const str_t& variable, const str_t& signature);
  void release_method(WrenMethod* method);
  void wren_call(WrenMethod* method, const char* arg_types, ...);
  void release_value(WrenValue* value);

  bool get_argument_bool(int index) const;
  double get_argument_double(int index) const;
  const char* get_argument_string(int index) const;
  WrenValue* get_argument_value(int index);
  void return_bool(bool value);
  void return_double(double value);
  void return_string(const str_t& text);
  void return_value(WrenValue* value);
};

class PinnedGuard final : private UnCopyable {
  WrenVM& vm_;
public:
  PinnedGuard(WrenVM& vm, BaseObject* obj) noexcept
    : vm_(vm) {
    vm_.push_root(obj);
  }

  ~PinnedGuard(void) noexcept {
    vm_.pop_root();
  }
};

}
